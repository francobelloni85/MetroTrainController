/*----------------------------------------------------------------------------
 * Name:    BlinkySingleLED.c
 * Purpose: LED Flasher
 *----------------------------------------------------------------------------*/

#include <stm32f10x.h>                       /* STM32F103 definitions         */
#include <RTL.h>                      /* RTX kernel functions & defines      */

#define GIGI_USE_ODR 999

volatile int T1=0;
volatile int T2=0;
volatile int IDLE=0;

__task void Task1(void);
__task void Task2(void);

void some_delay(unsigned long int n) {
	unsigned long int i;
	for (i-0; i<n; i++) {i=i;}
}

// Declares a semaphore
OS_SEM sem;

// Defines Task1
__task void Task1(void) {
//  os_sem_init( sem, 0 );     // qui non funziona!!!
  while( 1 ) {
		T2=0; T1=1; IDLE=0;
//    os_dly_wait( 2 );
		some_delay(1000);
    os_sem_send( sem ); // Frees the semaphore
  }
}

// Defines Task2
__task void Task2(void) {
  while( 1 ) {
    // Waits for the semaphore to be freed by Task1
    os_sem_wait( sem, 0xFFFF ); 
		T2=1; T1=0; IDLE=0;
    some_delay(1000);
  }
}

// Defines TaskInit
__task void TaskInit(void) {
  os_sem_init( sem, 0 );     // Initializes the semaphore
  os_tsk_create( Task1, 1 );
  os_tsk_create( Task2, 1 ); // TRY WITH DIFFERENT PRIORITY !! 
  os_tsk_delete_self();      // kills self
}

// Main
int main (void) { 
  // Starts TaskInit which, in turn,
  // creates the two tasks Task1 and Task2
  os_sys_init( TaskInit );
}
