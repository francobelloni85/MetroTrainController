#include "stm32f10x_it.h"

void TIM2_IRQHandler(void) {
	unsigned int i = 1<<8;
	
  /* Clear TIM2 update interrupt */
  TIM_ClearFlag(TIM2, TIM_FLAG_CC2);

	// Toggle LED
  if(GPIOB->ODR & i)  GPIOB->ODR &= ~i;
  else GPIOB->ODR |= i;
}

