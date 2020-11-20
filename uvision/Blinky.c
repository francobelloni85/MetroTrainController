/*----------------------------------------------------------------------------
 * Name:    BlinkySingleLED.c
 * Purpose: LED Flasher
 * 
 * Quando sono in stop
 * per riprendere "la corsa" deve togliarsi il segnale di stop e il guidatore deve mettere la leva su 0
 * 
 *----------------------------------------------------------------------------*/

#include <stm32f10x.h> /* STM32F103 definitions         */
#include <RTL.h>       /* RTX kernel functions & defines      */

#define GIGI_USE_ODR 999
#define BIN_TO_BYTE(b7, b6, b5, b4, b3, b2, b1, b0) ((b7 << 7) + (b6 << 6) + (b5 << 5) + (b4 << 4) + (b3 << 3) + (b2 << 2) + (b1 << 1) + b0)

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

// Declares a semaphore
OS_SEM sem;

unsigned int in_pin;

char pin_input_array[];

// INPUTS ------------------------------------------------

void WritePinInput(int index)
{
  if (index > 8 || index < 0)
  {
    return;
  }

  // 	char pin_output_array[] = "00000000";
  //
  // 	switch (index)
  //   {
  //   case 0:
  //     pin_output_array = BIN_TO_BYTE(1, 0, 0, 0, 0, 0, 0, 0);
  //     break;
  //   case 1:
  //     pin_output_array = BIN_TO_BYTE(0, 1, 0, 0, 0, 0, 0, 0);
  //     break;
  //   case 2:
  //     pin_output_array = BIN_TO_BYTE(0, 0, 1, 0, 0, 0, 0, 0);
  //     break;
  //   case 3:
  //     pin_output_array = BIN_TO_BYTE(0, 0, 0, 1, 0, 0, 0, 0);
  //     break;
  //   case 4:
  //     pin_output_array = BIN_TO_BYTE(0, 0, 0, 0, 1, 0, 0, 0);
  //     break;
  //   case 5:
  //     pin_output_array = BIN_TO_BYTE(0, 0, 0, 0, 0, 1, 0, 0);
  //     break;
  //   case 6:
  //     pin_output_array = BIN_TO_BYTE(0, 0, 0, 0, 0, 0, 1, 0);
  //     break;
  //   case 7:
  //     pin_output_array = BIN_TO_BYTE(0, 0, 0, 0, 0, 0, 0, 1);
  //     break;
  //   }

  //char empty[] = "00000000";
  //empty[index] = '1';
  //GPIO_B = empty;
}

void ReadInput()
{

  if (GPIOC->IDR & 0)
  {
    CurrentState = EMERGENCY;
    CurrentLeverPosition = strong_braking;
    _is_emergency_ON = 1;
  }

  if (GPIOC->IDR & 1)
  {
    CurrentState = STOP;
    CurrentLeverPosition = medium_braking;
    _is_stop_ON = 1;
  }

  if (GPIOC->IDR & 2)
  {
    CurrentState = NORMAL;
    CurrentLeverPosition = strong_braking;
  }

  if (GPIOC->IDR & 3)
  {
    CurrentState = NORMAL;
    CurrentLeverPosition = medium_braking;
  }

  //   if (value[4] == '1')
  //   {
  //     CurrentState = NORMAL;
  //     CurrentLeverPosition = minimum_braking;
  //   }

  //   if (value[5] == '1')
  //   {
  //     CurrentState = NORMAL;
  //     CurrentLeverPosition = no_acceleration;
  //   }

  //   if (value[6] == '1')
  //   {
  //     CurrentState = NORMAL;
  //     CurrentLeverPosition = minimum_acceleration;
  //   }

  //   if (value[7] == '1')
  //   {
  //     CurrentState = NORMAL;
  //     CurrentLeverPosition = medium_acceleration;
  //   }

  //if (value[8] == '1')
  //{
  //    CurrentState = state.NORMAL;
  //    CurrentLeverPosition = lever_position.maximum_acceleration;
  //}
}

// OUTPUTS ------------------------------------------------

void WritePinOutput(int index)
{
  if (index > 12 || index < 0)
  {
    return;
  }

  // Reset the last value
  GPIOC->ODR = 0x00000000;

  // Set the new value
  in_pin = 1 << index;
  GPIOC->ODR |= in_pin;
}

void WriteOutput(enum lever_position lever_Position)
{

  // 	char pin_output_array[] = "00000000";
  //
  //   switch (lever_Position)
  //   {
  //   case minimum_acceleration:
  //     pin_output_array = BIN_TO_BYTE(0, 0, 0, 1, 0, 0, 0, 0);
  //     break;

  //   case medium_acceleration:
  //     pin_output_array = BIN_TO_BYTE(0, 0, 0, 0, 1, 0, 0, 0);
  //     break;

  //   case maximum_acceleration:
  //     pin_output_array = BIN_TO_BYTE(0, 0, 0, 0, 0, 1, 0, 0);
  //     break;

  //   case no_acceleration:
  //     pin_output_array = BIN_TO_BYTE(0, 0, 0, 0, 0, 0, 1, 0);
  //     break;

  //   case minimum_braking:
  //     pin_output_array = BIN_TO_BYTE(0, 0, 0, 0, 0, 0, 0, 1);
  //     break;

  //   case medium_braking:
  //     pin_output_array = BIN_TO_BYTE(0, 0, 0, 0, 0, 0, 0, 0);
  //     break;

  //   case strong_braking:
  //     pin_output_array = BIN_TO_BYTE(0, 0, 0, 0, 0, 0, 0, 0);
  //     break;
  //   }

  //   GPIOB->ODR = pin_output_array;
}

// UTILITY ------------------------------------------------

void some_delay(unsigned long int n)
{
  unsigned long int i;
  for (i - 0; i < n; i++)
  {
    i = i;
  }
}

// TASKS ------------------------------------------------

// Defines TaskEventSimulator
__task void TaskEventSimulator(void)
{

  while (1)
  {
    task_train_controller = 0;
    task_event_simulator = 1;
    IDLE = 0;

    os_dly_wait(2);

    WritePinOutput(2);
    some_delay(100);

    WritePinOutput(3);
    some_delay(100);

    WritePinOutput(4);
    some_delay(100);

    WritePinOutput(5);
    some_delay(100);

    WritePinOutput(6);
    some_delay(100);

    WritePinOutput(7);
    some_delay(100);

    WritePinOutput(1);
    some_delay(100);

    WritePinOutput(8);
    some_delay(100);

    WritePinOutput(0);
    some_delay(100);

    WritePinOutput(8);
    some_delay(100);

    WritePinOutput(1);
    some_delay(100);

    os_sem_send( sem ); // Frees the semaphore

  }
}

// Defines TaskTrainController
__task void TaskTrainController(void)
{
  while (1)
  {
    task_train_controller = 1;
    task_event_simulator = 0;
    IDLE = 0;

    ReadInput();

    if (_is_emergency_ON)
    {
      CurrentState = EMERGENCY;
      CurrentLeverPosition = strong_braking;
      continue;
    }

    if (_is_stop_ON)
    {
      CurrentState = STOP;
      CurrentLeverPosition = medium_braking;
      continue;
    }

    switch (CurrentState)
    {

    case EMERGENCY:
      _is_emergency_ON = 1;
      WriteOutput(strong_braking);
      break;

    case STOP:
      _is_stop_ON = 1;
      WriteOutput(medium_braking);
      break;

    case NORMAL:
      WriteOutput(CurrentLeverPosition);
      break;
    }

    os_sem_wait( sem, 0xFFFF ); 

  }
}

// Defines TaskInit
__task void TaskInit(void)
{
  // Initializes the semaphore
  os_sem_init(sem, 0);

  os_tsk_create(TaskEventSimulator, 1);

  os_tsk_create(TaskTrainController, 5);

  // kills self
  os_tsk_delete_self();
}

// Main
int main(void)
{
  /* Enable GPIOB clock            */
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

  GPIOC->CRH = 0x80000003;

  // creates the two tasks
  // TaskEventSimulator	and TaskTrainController
  os_sys_init(TaskInit);
}
