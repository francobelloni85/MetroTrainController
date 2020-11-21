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

// GPIO B Pins
int PIN_MAXIMUM_ACCELERATION = 8;
int PIN_MEDIUM_ACCELERATION = 7;
int PIN_MINIMUM_ACCELERATION = 6;
int PIN_NO_ACCELERATION_NO_BRAKING = 5;
int PIN_MINIMUM_BRAKING = 4;
int PIN_MEDIUM_BRAKING = 3;
int PIN_STRONG_BRAKING = 2;
int PIN_STOP_SIGNAL = 1;
int PIN_ALLARM_SIGNAL = 0;

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

// TASKS ----------------------------------------------------------------------

volatile int task_event_simulator = 0;
volatile int task_train_controller = 0;
volatile int IDLE = 0;

__task void TaskEventSimulator(void);
__task void TaskTrainController(void);

// Declares a semaphore
OS_SEM sem;

// END TASKS

// SISTEM STATUS VARIABLES ---------------------------------------------------

/// <summary>
/// The value read from the input
/// </summary>
enum state CurrentState = NORMAL;

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

// GPIOx ADDRESS ------------------------------------------------------------

unsigned int read_pin;

int *crh_B; //CRN register for input output settings
int *odr_B; //ODR register for output data

unsigned int in_pin_b;
unsigned int in_pin_c;

// Used to set the pin in the GPIO_B

volatile int in_pin = 0x00001111;

// END GPIOx ADDRESS

// DEBUG VARIABLES  ---------------------------------------------------------

// used to simulate the input pin
int GPIOB_debug_pin = -1;

// used to simulate emergency pin is on
int GPIOB_debug_emergency_pin = 0;

// used to simulate stop pin is on
int GPIOB_debug_stop_pin = 0;

// END DEBUG VARIABLES

// GPIO_B -------------------------------------------------------------------

void ReadInput()
{
  // Pin 0
  read_pin = 0x00000001;
  if (GPIOB->IDR & read_pin || GPIOB_debug_emergency_pin == 1)
  {
    CurrentState = EMERGENCY;
    CurrentLeverPosition = strong_braking;
    _is_emergency_ON = 1;
  }

  // Pin 1
  read_pin = 0x00000002;
  if (GPIOB->IDR & read_pin || GPIOB_debug_emergency_pin == 1)
  {
    CurrentState = STOP;
    CurrentLeverPosition = medium_braking;
    _is_stop_ON = 1;
  }

  // Pin 2
  read_pin = 0x00000004;
  if (GPIOB->IDR & read_pin || GPIOB_debug_pin == PIN_STRONG_BRAKING)
  {
    CurrentState = NORMAL;
    CurrentLeverPosition = strong_braking;
  }

  // Pin 3
  read_pin = 0x00000008;
  if (GPIOB->IDR & read_pin || GPIOB_debug_pin == PIN_MEDIUM_BRAKING)
  {
    CurrentState = NORMAL;
    CurrentLeverPosition = medium_braking;
  }

  // Pin 4
  read_pin = 0x00000010;
  if (GPIOB->IDR & read_pin || GPIOB_debug_pin == PIN_MINIMUM_BRAKING)
  {
    CurrentState = NORMAL;
    CurrentLeverPosition = minimum_braking;
  }

  // Pin 5
  read_pin = 0x00000020;
  if (GPIOB->IDR & read_pin || GPIOB_debug_pin == PIN_NO_ACCELERATION_NO_BRAKING)
  {
    CurrentState = NORMAL;
    CurrentLeverPosition = no_acceleration;
  }

  // Pin 6
  read_pin = 0x00000040;
  if (GPIOB->IDR & read_pin || GPIOB_debug_pin == PIN_MINIMUM_ACCELERATION)
  {
    CurrentState = NORMAL;
    CurrentLeverPosition = minimum_acceleration;
  }

  // Pin 7
  read_pin = 0x00000080;
  if (GPIOB->IDR & read_pin || GPIOB_debug_pin == PIN_MEDIUM_ACCELERATION)
  {
    CurrentState = NORMAL;
    CurrentLeverPosition = medium_acceleration;
  }

  // Pin 8
  read_pin = 0x00000100;
  if (GPIOB->IDR & read_pin || GPIOB_debug_pin == PIN_MAXIMUM_ACCELERATION)
  {
    CurrentState = NORMAL;
    CurrentLeverPosition = maximum_acceleration;
  }

  // DEBUG TEST LETTURA
  read_pin = 0x00001111;
  if (GPIOB->IDR & read_pin)
  {
    read_pin = 12;
  }
}

void WritePin_GPIOB(int index)
{

  if (index > 12 || index < 0)
  {
    return;
  }

  // 1) Reset all the pins
  *odr_B = 0x00000000;

  // 2) Write the pin for the debug pourpose
  GPIOB_debug_pin = index;

  // 3) Write the signal in the GPIOB

  // ALLARM SIGNAL
  if (index == 0x00000000 || index == 0)
  {
    *odr_B = 0x00000001;
    GPIOB_debug_emergency_pin = 1;
  }

  // STOP SIGNAL
  if (index == 0x00000001 || index == 1)
  {
    *odr_B = 0x00000002;
    GPIOB_debug_stop_pin = 1;
  }

  // STRONG_BRAKING
  if (index == 0x00000002 || index == 2)
  {
    *odr_B = 0x00000004;
  }

  // MEDIUM_BRAKING
  if (index == 0x00000003)
  {
    *odr_B = 0x00000008;
  }

  // MINIMUM_BRAKING
  if (index == 0x00000004)
  {
    *odr_B = 0x00000010;
  }

  // NO_ACCELERATION_NO_BRAKING
  if (index == 0x00000005)
  {
    *odr_B = 0x00000020;
  }

  // MINIMUM_ACCELERATION
  if (index == 0x00000006)
  {
    *odr_B = 0x00000040;
  }

  // MEDIUM_ACCELERATION
  if (index == 0x00000007)
  {
    *odr_B = 0x00000080;
  }

  // MAXIMUM_ACCELERATION
  if (index == 0x00000008)
  {
    *odr_B = 0x00000100;
  }

  // Reset the last value
  //GPIOB->ODR = 0x00000000;

  // Set the new value
  //in_pin_b = 1 << index;
  //GPIOB->ODR |= in_pin_b;
}

// GPIO_C -----------------------------------------------------------------

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

void WriteLeveOutput(enum lever_position lever_Position)
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

// UTILITY --------------------------------------------------------

void some_delay(unsigned long int n)
{
  unsigned long int i;
  for (i - 0; i < n; i++)
  {
    i = i;
  }
}

// TASKS ----------------------------------------------------------

int pin_to_set = 3;

// Defines TaskEventSimulator
__task void TaskEventSimulator(void)
{

  while (1)
  {
    task_train_controller = 0;
    task_event_simulator = 1;
    IDLE = 0;

    //os_dly_wait(2);

    pin_to_set = 2;
    WritePin_GPIOB(pin_to_set);
    some_delay(1000);

    os_sem_send(sem); // Frees the semaphore

    pin_to_set = 3;
    WritePin_GPIOB(pin_to_set);
    some_delay(1000);

    os_sem_send(sem); // Frees the semaphore

    WritePin_GPIOB(4);
    some_delay(1000);

    os_sem_send(sem); // Frees the semaphore

    WritePin_GPIOB(5);
    some_delay(1000);

    os_sem_send(sem); // Frees the semaphore

    WritePin_GPIOB(6);
    some_delay(1000);

    os_sem_send(sem); // Frees the semaphore

    WritePin_GPIOB(7);
    some_delay(1000);

    os_sem_send(sem); // Frees the semaphore

    WritePin_GPIOB(1);
    some_delay(1000);

    os_sem_send(sem); // Frees the semaphore

    WritePin_GPIOB(8);
    some_delay(1000);

    os_sem_send(sem); // Frees the semaphore

    WritePin_GPIOB(0);
    some_delay(1000);

    os_sem_send(sem); // Frees the semaphore

    WritePin_GPIOB(8);
    some_delay(1000);

    os_sem_send(sem); // Frees the semaphore

    WritePin_GPIOB(1);
    some_delay(1000);

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
      WriteLeveOutput(CurrentLeverPosition);
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
  crh_B = (int *)(0x40010C04); //define the address of crh register
  odr_B = (int *)(0x40010C0C); //define the address of odr register

  /* Enable GPIOB clock            */
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

  GPIOA->CRH = 0x20010C04;
  GPIOB->CRH = 0x40010C04;
  GPIOC->CRH = 0x60010C04;

  // DEBUG > SET BIT DIRECTLY
  *odr_B = 0x00000000;
  *odr_B = 0x00001110;

  // DEBUG > TEST READ VALUE AND IT'S WORKING
  if (GPIOB->IDR & in_pin)
  {
    in_pin = in_pin;
  }

  // creates the two tasks
  // TaskEventSimulator	and TaskTrainController
  os_sys_init(TaskInit);
}
