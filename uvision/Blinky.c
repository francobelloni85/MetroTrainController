/*----------------------------------------------------------------------------
 * Name:    Blink.c
 * Purpose: TrainController
 * Model: 	STM32F
 * 
 *----------------------------------------------------------------------------	*/

#include <stdio.h>     /* prototype declarations for I/O functions  	    */
#include <stm32f10x.h> /* STM32F103 definitions                             */
#include <RTL.h>       /* RTX kernel functions & defines      				*/

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
  STRONG_BRAKING = 1,
  MEDIUM_BRAKING,
  MINIMUM_BRAKING,
  NO_ACCELERATION,
  MINIMUM_ACCELERATION,
  MEDIUM_ACCELERATION,
  MAXIMUM_ACCELERATION,
  EMERGENCY_BRAKE,
};

// TASKS ----------------------------------------------------------------------

volatile int task_event_simulator = 0;
volatile int task_train_controller = 0;
volatile int IDLE = 0;

__task void TaskEventSimulator(void);
__task void TaskTrainController(void);
__task void TaskMessages(void);

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
int is_emergency_ON = 0;

/// <summary>
/// Is set to true if the emergency stop has been pressed
/// </summary>
int is_stop_ON = 0;

// it is set to true if the time limit in position 3 of the speed has been exceeded
int is_max_speed_limt = 0;

int count_max_speed_limt_tick = 0;

int sleep_timer_tick = 3;

// END SISTEM STATUS VARIABLES

// GPIOx ADDRESS ------------------------------------------------------------

unsigned int read_pin;
unsigned int write_pin;

int in_pin_c = 0;

// Used to set the pin in the GPIO_B

volatile int in_pin = 0x00001111;

// END GPIOx ADDRESS

// SERIAL -------------------------------------------------------------------

extern void SER_Init(void); /* see Serial.c */

int is_message_usar_display = 0;

int loop_i = 0;

const int message_len = 5;

char message[message_len];

// TASK TRAIN

int count_task_train_controller = 0;

// GPIO_B -------------------------------------------------------------------

// This class read the inputs
// and set:
// the allarm pin _is_emergency_ON or is_stop_ON
// the CurrentLeverPosition
void ReadInput()
{

  // Count the pins
  int count_signal = 0;

  // Reset the allarm
  is_emergency_ON = 0;
  is_stop_ON = 0;
	read_pin = 0 ;
	
  // Pin 0 (EMERGENCY)	
  if (IsPinON_GPIOB(PIN_ALLARM_SIGNAL) == 1)
  {
    is_emergency_ON = 1;
  }

  // Pin 1 (STOP) 
	if (IsPinON_GPIOB(PIN_STOP_SIGNAL)== 1)
  {
    is_stop_ON = 1;
  }
		
  // Pin 2
  if (IsPinON_GPIOB(PIN_STRONG_BRAKING)== 1)
	{
    CurrentLeverPosition = STRONG_BRAKING;
    count_signal++;
  }

  // Pin 3
	if (IsPinON_GPIOB(PIN_MEDIUM_BRAKING)== 1)
  {
    CurrentLeverPosition = MEDIUM_BRAKING;
    count_signal++;
  }

  // Pin 4
	if (IsPinON_GPIOB(PIN_MINIMUM_BRAKING)== 1)
  {
    CurrentLeverPosition = MINIMUM_BRAKING;
    count_signal++;
  }

  // Pin 5
	if (IsPinON_GPIOB(PIN_NO_ACCELERATION_NO_BRAKING)== 1)
  {
    CurrentLeverPosition = NO_ACCELERATION;
    count_signal++;
  }

  // Pin 6
	if (IsPinON_GPIOB(PIN_MINIMUM_ACCELERATION)== 1)
  {
    CurrentLeverPosition = MINIMUM_ACCELERATION;
    count_signal++;
  }

  // Pin 7
  if (IsPinON_GPIOB(PIN_MEDIUM_ACCELERATION)== 1)
	{
    CurrentLeverPosition = MEDIUM_ACCELERATION;
    count_signal++;
  }

  // Pin 8
	if (IsPinON_GPIOB(PIN_MAXIMUM_ACCELERATION) == 1)
  {
    CurrentLeverPosition = MAXIMUM_ACCELERATION;
    count_signal++;
  }

  if (count_signal >= 2)
  {
    // Qualcosa non va
    // Metto la leva in posizione di "freno" ?
    CurrentLeverPosition = STRONG_BRAKING;
  }

}

void WritePin_GPIOB(int index)
{

  if (index > 12 || index < 0)
  {
    return;
  }

  // 1) Reset all the pins  
	GPIOB->BSRR = 0xFFFF0000;
	
	// 2) Set the new value
  in_pin_c = 1 << index;
  GPIOB->ODR |= in_pin_c;
	  
}

void WriteInterruptSignal_GPIOB(int index)
{	
  if (index > 2 || index < 0)
  {
    return;
  }

  // ALLARM SIGNAL
  if (index == 0x00000000 || index == 0)
  {   
		GPIOC->BSRR = (1U << 0); 
  }

  // STOP SIGNAL
  if (index == 0x00000001 || index == 1)
  {
		GPIOC->BSRR = (1U << 1); 
  }
}


int IsPinON_GPIOB(int index){

	int value = 0;
	
	//Reading PBxx
	value = (( GPIOB->IDR >> index ) & 0x1);
	
	return value;
	
}




// GPIO_C -----------------------------------------------------------------

// Warning
// This function must be through WriteLeveOutput,
// it must not be called directly
void WritePin_GPIOC(int index)
{
  if (index > 12 || index < 0)
  {
    return;
  }
	
	// 1) Reset all the pins  
	GPIOC->BSRR = 0xFFFF0000;
	
  // Reset all the pins (opt2)
  GPIOC->ODR = 0x00000000;
		
  // Set the new value
  in_pin_c = 1 << index;
  GPIOC->ODR |= in_pin_c;
		
}

// Warning
// This function must be through WriteLeveOutput,
// it must not be called directly
void WritePin_GPIOC_NO_RESET(int index)
{	  
	
  if (index > 12 || index < 0)
  {
    return;
  }
	
  // Set the new value
	in_pin_c = 1 << index;
  GPIOC->ODR |= in_pin_c;
	
	//GPIOC->BSRR = (1U << 5); 
}

void WriteLeveOutput(enum lever_position lever_Position)
{
  int pin = 0;

  switch (lever_Position)
  {
  case MINIMUM_ACCELERATION:
    pin = 2;
    break;

  case MEDIUM_ACCELERATION:
    pin = 1;
    break;

  case MAXIMUM_ACCELERATION:
    pin = 0;
    break;

  case NO_ACCELERATION:
    pin = 4;
    break;

  case MINIMUM_BRAKING:
    pin = 9;
    break;

  case MEDIUM_BRAKING:
    pin = 10;
    break;

  case STRONG_BRAKING:
    pin = 11;
    break;
		
	case EMERGENCY_BRAKE:
    pin = 12;
		WritePin_GPIOC_NO_RESET(pin);
    break;
  }

  WritePin_GPIOC(pin);
}

// UTILITY --------------------------------------------------------

// TASKS ----------------------------------------------------------

void WaitTaskEventSimulatorVariables(int system_tick)
{
  os_dly_wait(system_tick);
  task_train_controller = 0;
  task_event_simulator = 1;
  IDLE = 0;
}

void SentMessageHello()
{

  message[0] = 'H';
  message[1] = 'e';
  message[2] = 'l';
  message[3] = 'l';
  message[4] = 'o';

  for (loop_i = 0; loop_i < message_len; loop_i++)
  {
    SER_PutChar(message[loop_i]);
  }

  os_dly_wait(10);
}

// Defines TaskEventSimulator
__task void TaskEventSimulator(void)
{

  while (1)
  {

    task_train_controller = 0;
    task_event_simulator = 1;
    IDLE = 0;

    // TEST 1  - MULTIPLE INPUT SIGNALS --------------

    WaitTaskEventSimulatorVariables(sleep_timer_tick);

    // TEST 2  - READ AND WRITE SIGNAL --------------

    WritePin_GPIOB(PIN_NO_ACCELERATION_NO_BRAKING);

    WaitTaskEventSimulatorVariables(sleep_timer_tick);

    WritePin_GPIOB(PIN_MINIMUM_ACCELERATION);

    WaitTaskEventSimulatorVariables(sleep_timer_tick);

    WritePin_GPIOB(PIN_MEDIUM_ACCELERATION);

    WaitTaskEventSimulatorVariables(sleep_timer_tick);

    WritePin_GPIOB(PIN_MAXIMUM_ACCELERATION);

    WaitTaskEventSimulatorVariables(sleep_timer_tick);

    WritePin_GPIOB(PIN_MINIMUM_BRAKING);

    WaitTaskEventSimulatorVariables(sleep_timer_tick);

    WritePin_GPIOB(PIN_MEDIUM_BRAKING);

    WaitTaskEventSimulatorVariables(sleep_timer_tick);

    WritePin_GPIOB(PIN_STRONG_BRAKING);

    WaitTaskEventSimulatorVariables(sleep_timer_tick);

    // TEST 3  - STOP SIGNAL --------------

    // se arriva il segnale di stop il treno di deve fermare (gentilamente)
    // si può riprendere solo se il segnale di stop viene interrotto E
    // il macchinista rimette la leva in posizione di "folle"

    // 3A -> VADO A VELOCITA MEDIA  --------------

    WritePin_GPIOB(PIN_MEDIUM_ACCELERATION);

    WaitTaskEventSimulatorVariables(sleep_timer_tick);

    // 3B -> MANDO IL SEGNALE DI STOP
    WriteInterruptSignal_GPIOB(PIN_STOP_SIGNAL);

    WaitTaskEventSimulatorVariables(sleep_timer_tick);

    // 3C -> PROVO AD USCIRE DALLO STOP
    // SETTO LA VELOCITA A MEDIA MA TENGO IL SEGNALE DI STOP
		GPIOB->BSRR = (1U << PIN_STOP_SIGNAL); 
		GPIOB->BSRR = (1U << PIN_MAXIMUM_ACCELERATION); 

    WaitTaskEventSimulatorVariables(sleep_timer_tick);

    // 3D -> TOLGO IL SEGNALE DI STOP
    GPIOB->BSRR = 0xFFFF0000;
		GPIOB->BSRR = (1U << PIN_MAXIMUM_ACCELERATION); 

    WaitTaskEventSimulatorVariables(sleep_timer_tick);

    // 3E -> RI-PROVO AD USCIRE DALLO STOP
    WritePin_GPIOB(PIN_MEDIUM_ACCELERATION);

    WaitTaskEventSimulatorVariables(sleep_timer_tick);

    // 3F -> PORTO LA LEVA IN POSIZIONE CORRETTA
    WritePin_GPIOB(PIN_NO_ACCELERATION_NO_BRAKING);

    WaitTaskEventSimulatorVariables(sleep_timer_tick);

    // 3G -> RI-RI-PROVO AD USCIRE DALLO STOP
    WritePin_GPIOB(PIN_MEDIUM_ACCELERATION);

    WaitTaskEventSimulatorVariables(sleep_timer_tick);

    // TEST 4  - VELOCITA A MAXSPEED(3) PER TROPPO TEMPO --------------

    WritePin_GPIOB(PIN_MAXIMUM_ACCELERATION);

    WaitTaskEventSimulatorVariables(200);

    WritePin_GPIOB(PIN_MINIMUM_BRAKING);

    WaitTaskEventSimulatorVariables(sleep_timer_tick);

    continue;

    // TEST 5  - SEGNALE DI EMERGENZA --------------

    // 5A -> SEGNALE DI ALLARME
    WritePin_GPIOB(PIN_ALLARM_SIGNAL);

    WaitTaskEventSimulatorVariables(sleep_timer_tick);

    // 5B -> PROVO INUTILMENTE AD USCIRE DALL'ALLARME
    WritePin_GPIOB(PIN_MEDIUM_ACCELERATION);
  }
}

void ExitTaskTrainController()
{
  // Needed in the debug to go to sleep
  count_task_train_controller++;
  if (count_task_train_controller >= 20)
  {
    count_task_train_controller = 0;
    os_dly_wait(1);
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

    if (is_emergency_ON)
    {
      CurrentState = EMERGENCY;
      CurrentLeverPosition = STRONG_BRAKING;
			WriteLeveOutput(STRONG_BRAKING);
			WriteLeveOutput(EMERGENCY_BRAKE);
      ExitTaskTrainController();
      continue;
    }

    if (is_stop_ON || CurrentState == STOP)
    {
      // There is the STOP signal
      if (is_stop_ON)
      {
        CurrentState = STOP;
        WriteLeveOutput(MEDIUM_BRAKING);
        ExitTaskTrainController();
        continue;
      }
      else
      {
        // there is no more the stop sign
        // and the lever is in position 0
        // -> it is possible to exit from the STOP situation
        if (CurrentLeverPosition == NO_ACCELERATION)
        {
					WriteLeveOutput(NO_ACCELERATION);
          CurrentState = NORMAL;					
        }
      }

      // Exit here and do not continue if in the stop state
      ExitTaskTrainController();
      continue;
    }

    // Se sono in posizione di max speed aumento di un tick (10ms circa)
    // Se ho raggiunto i 24,000 tick (4 secondi circa) allora devo
    // forzare e diminuire la velocita.
    // Ogni altro segnale della leva resetta il contatore.
    if (CurrentLeverPosition == MAXIMUM_ACCELERATION)
    {
      count_max_speed_limt_tick++;
      if (count_max_speed_limt_tick > 180)
      {
        is_max_speed_limt = 1;
      }
    }
    else
    {
      count_max_speed_limt_tick = 0;
      is_max_speed_limt = 0;
    }

    // if the time limit in position 3 of the speed has been exceeded
    if (is_max_speed_limt == 1)
    {
      WriteLeveOutput(MEDIUM_ACCELERATION);
      ExitTaskTrainController();
      continue;
    }

    // I'm in the normal condition
    // Just "read and write"
    WriteLeveOutput(CurrentLeverPosition);
    ExitTaskTrainController();
  }
}

// Defines TaskMessages
__task void TaskMessages(void)
{
  while (1)
  {

    task_train_controller = 0;
    task_event_simulator = 0;
    IDLE = 1;

    if (is_message_usar_display == 0)
    {
      // Per mandare un messaggio bisogna scrivere nella tab USART del debugger
      printf("Ready to read from USART -- Type in the USART tab\n");
      is_message_usar_display = 1;
    }

    while (SER_CheckChar() == 1)
    {
      char char_read = SER_GetChar();
      SER_PutChar('>');
      SER_PutChar(char_read);
    }

    os_dly_wait(10);
  }
}

// Defines TaskInit
__task void TaskInit(void)
{
  os_tsk_create(TaskEventSimulator, 100);

  os_tsk_create(TaskTrainController, 50);

  os_tsk_create(TaskMessages, 2);

  // kills self
  os_tsk_delete_self();
}




// Main
int main(void)
{
	
	
		// TO DO:	
		// Outputs must be configured as push-pull.		
		// Read from GPIOB		
		// CONFIGUARE I REGISTRI 
	
		// BREVE INTRO -> 
		// https://gist.github.com/Vitorbnc/e35f1ff1485d660edf365241dacfa387
		// https://embetronicx.com/tutorials/microcontrollers/stm32/stm32-gpio-tutorial/
		
		// il registo ha 4 bit, che definisco la porta. Ogni registro gestice 8 porte
		// ci sono 2 registri CRH e CRL che per gestire le 16 porte
		// il registro si carica tutto in una volta con una parola da 32 bit -> 4 bit *8 porte = 32bit
		// in base alla tabella decido come settare una porta
		// I primi 2 bit decidono una cosa, i secondi 2 bit un altra
		// 0000.0011.0000.0011.0000.0011.0000.0011
		// Ad esempio
		// 0011 (binary) or 0x3 (HEX) - Corresponds to setting pin as output
		// tutti i bit sono quindi settati (come da tabella) su GP output push-pull
	
		// HELP FROM BIT TO HEX
		// https://www.rapidtables.com/convert/number/decimal-to-hex.html
		
		
		// GPIOB > defined all pin as Output  
		GPIOB->CRH    =  0x33333333;  
		GPIOB->CRL     = 0x33333333;    
		
		// GPIOC > defined all pin as Output  
		GPIOC->CRH    =  0x33333333;  
		GPIOC->CRL     = 0x33333333;  
			
    SER_Init();                     // initialize the serial interface
		
    // Enable GPIOB clock       
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
		
		// enable GPIOA clock
		RCC->APB2ENR |= ( 1UL << 2); 
		
		// JUST FOR DEBUG -------------
		
		//set all the bits of Port B
		GPIOB->BSRR = 0x0000FFFF;
		
		//Clear all the bits of Port B
		GPIOB->BSRR = 0xFFFF0000;
		
		// END JUST FOR DEBUG -------------
		 
		// SET 2 pin HIGHT for TEST 1 
		
		//Set the 5th bit of Port B
		GPIOB->BSRR = (1U << 5); 
		
		//Set the 8th bit of Port B
		GPIOB->BSRR = (1U << 8); 
		 
		// Useful stuff
		
		// Clear the 5th bit of Port B
		// GPIOB->BSRR = (1U << 21);
		
		// 		//Reading PB0 bit
		// 		if( ( GPIOB->IDR & 0x8) == 0 )   
		// 		{
		// 			//PORT B's 0th bit is 0
		// 		}
		// 		else
		// 		{
		// 			//PORT B's 0th bit is 1
		// 		}
		 
    // creates the 3 tasks
    // TaskEventSimulator, TaskTrainController and TaskMessages
    os_sys_init(TaskInit);

}
