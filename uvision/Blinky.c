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

#define GPIOA_IDR  (*(volatile const uint32_t *)(0x40010800UL + 0x08UL)) // use `const` since this register is read-only
#define GPIOA_ODR  (*(volatile uint32_t *)(0x40010800UL + 0x0CUL))
#define GPIOA_BSRR (*(volatile uint32_t *)(0x40010800UL + 0x10UL))
#define GPIOA_BRR  (*(volatile uint32_t *)(0x40010800UL + 0x14UL))

#define GPIOB_IDR  (*(volatile const uint32_t *)(0x40010800UL + 0x08UL)) // use `const` since this register is read-only
#define GPIOB_ODR  (*(volatile uint32_t *)(0x40010800UL + 0x0CUL))
#define GPIOB_BSRR (*(volatile uint32_t *)(0x40010800UL + 0x10UL))
#define GPIOB_BRR  (*(volatile uint32_t *)(0x40010800UL + 0x14UL))

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

unsigned int read_pin;

void ReadInput()
{
	// Choose a pin number from 0 to 15
	uint8_t pin_i = 2; // pin index

	// Read it
	int pin_state = (GPIOB_IDR >> (uint32_t)pin_i) & 0x1;
	
  read_pin = 1 << 0;
  if (GPIOB->IDR & read_pin)
  {
    CurrentState = EMERGENCY;
    CurrentLeverPosition = strong_braking;
    _is_emergency_ON = 1;
  }

  read_pin = 1 << 1;
  if (GPIOB->IDR & read_pin)
  {
    CurrentState = STOP;
    CurrentLeverPosition = medium_braking;
    _is_stop_ON = 1;
  }

  read_pin = 1 << 2;
  if (GPIOB->IDR & read_pin)
  {
    CurrentState = NORMAL;
    CurrentLeverPosition = strong_braking;
  }

  read_pin = 1 << 3;
  if (GPIOB->IDR & read_pin)
  {
    CurrentState = NORMAL;
    CurrentLeverPosition = medium_braking;
  }

  read_pin = 1 << 4;
  if (GPIOB->IDR & read_pin)
  {
    CurrentState = NORMAL;
    CurrentLeverPosition = minimum_braking;
  }

  read_pin = 1 << 5;
  if (GPIOB->IDR & read_pin)
  {
    CurrentState = NORMAL;
    CurrentLeverPosition = no_acceleration;
  }

  read_pin = 1 << 6;
  if (GPIOB->IDR & read_pin)
  {
    CurrentState = NORMAL;
    CurrentLeverPosition = minimum_acceleration;
  }

  read_pin = 1 << 7;
  if (GPIOB->IDR & read_pin)
  {
    CurrentState = NORMAL;
    CurrentLeverPosition = medium_acceleration;
  }

  read_pin = 1 << 8;
  if (GPIOB->IDR & read_pin)
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

unsigned int read_all_port;

// Defines TaskTrainController
__task void TaskTrainController(void)
{
  while (1)
  {
    task_train_controller = 1;
    task_event_simulator = 0;
    IDLE = 0;

    ReadInput();
		
		//read_all_port =GPIO_ReadInputData(GPIOB);

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

int *crh_B;     //CRN register for input output settings
int *odr_B;     //ODR register for output data

unsigned int out_pin = 0;
unsigned int in_pin = 1;
; 
// Main
int main(void)
{
	crh_B=(int*)(0x40010C04);   				//define the address of crh register 
	odr_B=(int*)(0x40010C0C);   //define the address of odr register	
	
  /* Enable GPIOB clock            */
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
	
	GPIOB->CRH    =  0x40010C04;               /* PB.8 defined as Output, PB.15 as Input  */
	
  GPIOA->CRH = 0x20010C04;
	GPIOB->CRH = 0x40010C04;
	GPIOC->CRH = 0x60010C04;
	
	*odr_B=0x00000000;
	*odr_B=0x00001111;
	if(GPIOB->IDR & in_pin) {
      GPIOB->ODR |= out_pin;                 // switch LED ON
	}
	// Working!
	in_pin = 0x00001111;
	if(GPIOB->IDR & in_pin) {
      GPIOB->ODR |= out_pin;                 // switch LED ON
	}
	
  // creates the two tasks
  // TaskEventSimulator	and TaskTrainController
  os_sys_init(TaskInit);
}
