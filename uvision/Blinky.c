/*----------------------------------------------------------------------------
 * Name:    BlinkySingleLED.c
 * Purpose: LED Flasher
 *----------------------------------------------------------------------------*/

#include <stm32f10x.h> /* STM32F103 definitions         */
#include <RTL.h>       /* RTX kernel functions & defines      */

#define GIGI_USE_ODR 999

enum state
{
  STARTUP = 0,
  NORMAL = 1,
  STOP = 2,
  EMERGENCY = 3
};

enum lever_position
{
  strong_braking = 1,
  medium_braking,
  minimum_braking,
  no_acceleration,
  minimum_acceleration,
  medium_acceleration,
  maximum_acceleration,
};

volatile int task_event_simulator = 0;
volatile int task_train_controller = 0;
volatile int IDLE = 0;

char GPIO_A[8];
char GPIO_B[8];
char GPIO_C[8];

__task void TaskEventSimulator(void);
__task void TaskTrainController(void);

/// <summary>
/// The value read from the input
/// </summary>
enum state CurrentState;

/// <summary>
/// The value read from the input
/// </summary>
enum lever_position CurrentLeverPosition;

/// <summary>
/// Is set to true if the emergency button has been pressed
/// </summary>
int _is_emergency_ON = 0;

/// <summary>
/// Is set to true if the emergency stop has been pressed
/// </summary>
int _is_stop_ON = 0;

void WritePinInput(int index)
{
  if (index > 8 || index < 0)
  {
    return;
  }
  //char empty[] = "00000000";
  //empty[index] = '1';
  //GPIO_B = empty;
}

void WriteInput(enum state my_state, enum lever_position my_lever_position)
{
  int int_state = (int)my_state;
  int int_lever_position = (int)my_lever_position;

  switch (int_state)
  {
  case 1:
    //state.NORMAL:
    WritePinInput(int_lever_position);
    break;
  case 2:
    // state.EMERGENCY:
    WritePinInput(0);
    break;
  case 3:
    // state.STOP:
    WritePinInput(1);
    break;
  }
}

void WritePinOutput(int index)
{
  if (index > 12 || index < 0)
  {
    return;
  }

  //char empty[] = "000000000000";
  //empty[index] = '1';
  //GPIO_C = empty;
}

void some_delay(unsigned long int n)
{
  unsigned long int i;
  for (i - 0; i < n; i++)
  {
    i = i;
  }
}

// Declares a semaphore
OS_SEM sem;

// Defines TaskEventSimulator
__task void TaskEventSimulator(void)
{

  while (1)
  {
    task_train_controller = 0;
    task_event_simulator = 1;
    IDLE = 0;
    os_dly_wait(2);
    some_delay(1000);
    os_sem_send(sem); // Frees the semaphore
  }
}

// Defines TaskTrainController

__task void TaskTrainController(void)
{
  while (1)
  {
    // Waits for the semaphore to be freed by TaskEventSimulator
    os_sem_wait(sem, 0xFFFF);
    task_train_controller = 1;
    task_event_simulator = 0;
    IDLE = 0;
    some_delay(1000);
  }
}

// Defines TaskInit
__task void TaskInit(void)
{
  os_sem_init(sem, 0); // Initializes the semaphore
  os_tsk_create(TaskEventSimulator, 1);
  os_tsk_create(TaskTrainController, 10);
  os_tsk_delete_self(); // kills self
}

// Main
int main(void)
{
  // Starts TaskInit which, in turn,
  // creates the two tasks TaskEventSimulator and TaskTrainController
  os_sys_init(TaskInit);
}
