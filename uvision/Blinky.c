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

unsigned int in_pin_b;
unsigned int in_pin_c;

char pin_input_array[];

// GPIO_B ------------------------------------------------

void ReadInput()
{

  if (GPIOB->IDR & 0)
  {
    CurrentState = EMERGENCY;
    CurrentLeverPosition = strong_braking;
    _is_emergency_ON = 1;
  }

  if (GPIOB->IDR & 1)
  {
    CurrentState = STOP;
    CurrentLeverPosition = medium_braking;
    _is_stop_ON = 1;
  }

  if (GPIOB->IDR & 2)
  {
    CurrentState = NORMAL;
    CurrentLeverPosition = strong_braking;
  }

  if (GPIOB->IDR & 3)
  {
    CurrentState = NORMAL;
    CurrentLeverPosition = medium_braking;
  }

  if (GPIOB->IDR & 4)
  {
    CurrentState = NORMAL;
    CurrentLeverPosition = minimum_braking;
  }

  if (GPIOB->IDR & 5)
  {
    CurrentState = NORMAL;
    CurrentLeverPosition = no_acceleration;
  }

  if (GPIOB->IDR & 6)
  {
    CurrentState = NORMAL;
    CurrentLeverPosition = minimum_acceleration;
  }

  if (GPIOB->IDR & 7)
  {
    CurrentState = NORMAL;
    CurrentLeverPosition = medium_acceleration;
  }

  if (GPIOB->IDR & 8)
  {
    CurrentState = NORMAL;
    CurrentLeverPosition = maximum_acceleration;
  }

}

void WritePin_GPIOB(int index)
{
  if (index > 12 || index < 0)
  {
    return;
  }

  // Reset the last value
  GPIOB->ODR = 0x00000000;

  // Set the new value
  in_pin_b = 1 << index;
  GPIOB->ODR |= in_pin_b;
}

// GPIO_C ------------------------------------------------

void WritePin_GPIOC(int index)
{
  if (index > 12 || index < 0)
  {
    return;
  }

  // Reset the last value
  GPIOC->ODR = 0x00000000;

  // Set the new value
  in_pin_c = 1 << index;
  GPIOC->ODR |= in_pin_c;
}

void WriteOutput(enum lever_position lever_Position)
{
    int pin = 0;

    switch (lever_Position)
    {
    case minimum_acceleration:
       pin = 2;
      break;

    case medium_acceleration:
       pin = 1;
      break;

    case maximum_acceleration:
      pin = 0;
      break;

    case no_acceleration:
       pin = 4;
      break;

    case minimum_braking:
       pin = 9;
      break;

    case medium_braking:
       pin = 10;
      break;

    case strong_braking:
       pin = 11;
      break;
    }

    WritePin_GPIOC(pin);

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

    WritePin_GPIOB(2);
    some_delay(100);

    WritePin_GPIOB(3);
    some_delay(100);

    WritePin_GPIOB(4);
    some_delay(100);

    WritePin_GPIOB(5);
    some_delay(100);

    WritePin_GPIOB(6);
    some_delay(100);

    WritePin_GPIOB(7);
    some_delay(100);

    WritePin_GPIOB(1);
    some_delay(100);

    WritePin_GPIOB(8);
    some_delay(100);

    WritePin_GPIOB(0);
    some_delay(100);

    WritePin_GPIOB(8);
    some_delay(100);

    WritePin_GPIOB(1);
    some_delay(100);

    os_sem_send(sem); // Frees the semaphore
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
      WritePin_GPIOC(12);
      break;

    case STOP:
      _is_stop_ON = 1;
      WritePin_GPIOC(medium_braking);
      break;

    case NORMAL:
      WriteOutput(CurrentLeverPosition);
      break;
    }

    os_sem_wait(sem, 0xFFFF);
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
