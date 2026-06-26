#include <Arduino.h>


#include <stdio.h>
#include <stdlib.h>
#include "ph7.h"
/* 
 * Display an error message and exit.
 */
static void Fatal(const char *zMsg)
{
  puts(zMsg);
  /* Shutdown the library */
  ph7_lib_shutdown();
  /* Exit immediately */
  exit(0);
}
#include <unistd.h>

/*
 * VM output consumer callback.
 * Each time the virtual machine generates some outputs,the following
 * function gets called by the underlying virtual machine to consume
 * the generated output.
 * All this function does is redirecting the VM output to STDOUT.
 * This function is registered later via a call to ph7_vm_config()
 * with a configuration verb set to: PH7_VM_CONFIG_OUTPUT.
 */
static int Output_Consumer(const void *pOutput,unsigned int nOutputLen,void *pUserData /* Unused */)
{
  ssize_t nWr;
  nWr = write(1,pOutput,nOutputLen);
  if( nWr < 0 ){
    /* Abort processing */
    return PH7_ABORT;
  }
  /* All done,VM output was redirected to STDOUT */
  return PH7_OK;
}
/*
 * Main program: Compile and execute the PHP file. 
 */
const char *prog = "\r\n"
"<?php\r\n"
" echo 'Welcome guest'.PHP_EOL;\r\n"
" echo 'Current system time is: '.date('Y-m-d H:i:s').PHP_EOL;\r\n"
" echo 'and you are running '.php_uname();\r\n"
"?>\r\n";


int engine_test() {

  
  ph7 *pEngine; /* PH7 engine */
  ph7_vm *pVm;  /* Compiled PHP program */
  int rc;
  int dump_vm = 0;    /* Dump VM instructions if TRUE */
  int err_report = 1; /* Report run-time errors if TRUE */
  int n;              /* Script arguments */


  /* Allocate a new PH7 engine instance */
  rc = ph7_init(&pEngine);
  if( rc != PH7_OK ){
    /*
     * If the supplied memory subsystem is so sick that we are unable
     * to allocate a tiny chunk of memory,there is no much we can do here.
     */
    Fatal("Error while allocating a new PH7 engine instance");
  }
  /* Set an error log consumer callback. This callback [Output_Consumer()] will
   * redirect all compile-time error messages to STDOUT.
   */
  ph7_config(pEngine,PH7_CONFIG_ERR_OUTPUT,
    Output_Consumer, /* Error log consumer */
    0 /* NULL: Callback Private data */
    );

  /* Now,it's time to compile our PHP file */
  rc = ph7_compile(pEngine, prog, strlen(prog), &pVm);

  if( rc != PH7_OK ){ /* Compile error */
    if( rc == PH7_IO_ERR ){
      Fatal("IO error while opening the target file");
    }else if( rc == PH7_VM_ERR ){
      Fatal("VM initialization error");
    }else{
      /* Compile-time error, your output (STDOUT) should display the error messages */
      Fatal("Compile error");
    }
  }
  /*
   * Now we have our script compiled,it's time to configure our VM.
   * We will install the VM output consumer callback defined above
   * so that we can consume the VM output and redirect it to STDOUT.
   */
  rc = ph7_vm_config(pVm,
    PH7_VM_CONFIG_OUTPUT,
    Output_Consumer,    /* Output Consumer callback */
    0                   /* Callback private data */
    );
  if( rc != PH7_OK ){
    Fatal("Error while installing the VM output consumer callback");
  }

  if( dump_vm ){
    /* Dump PH7 byte-code instructions */
    ph7_vm_dump_v2(pVm,
      Output_Consumer, /* Dump consumer callback */
      0
      );
  }
  /*
   * And finally, execute our program. Note that your output (STDOUT in our case)
   * should display the result.
   */
  ph7_vm_exec(pVm,0);
  /* All done, cleanup the mess left behind.
  */
  ph7_vm_release(pVm);
  ph7_release(pEngine);
  return 0;
}




void setup() {
  
  Serial.begin(115200);
}


void loop() {

  delay(1000);
  engine_test();
}
