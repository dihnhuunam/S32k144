#include "S32K144.h"
#include "interrupt_manager.h"

#define LED_GREEN 16 // PTD16
#define SW2 12		 // PTC12

void delay(volatile int cycles);
void InitLed(uint8_t ledPin);
void InitSW(uint8_t swPin);
void InitInterrupt(uint8_t swPin);

volatile uint8_t g_ledPin = LED_GREEN;

int main(void)
{
	InitLed(LED_GREEN);
	InitSW(SW2);
	InitInterrupt(SW2);

	while (1)
	{
	}
	return 0;
}

void delay(volatile int cycle)
{
	while (cycle--)
		;
}

void InitLed(uint8_t ledPin)
{
	// 1. Enable clock
	PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC(1);

	// 2. Select MUX as GPIO, enable Drive Strength
	PORTD->PCR[ledPin] = PORT_PCR_MUX(1) | PORT_PCR_DSE(1);

	// 3. Set data direction as output, default LED is OFF
	PTD->PDDR |= (1 << ledPin);
	PTD->PDOR |= (1 << ledPin);
}

void InitSW(uint8_t swPin)
{
	// 1. Enable clock for PORTC
	PCC->PCCn[PCC_PORTC_INDEX] |= PCC_PCCn_CGC(1);

	// 2. Set MUX as GPIO, enable Passive Filter, disable  Pull-up
	PORTC->PCR[swPin] |= PORT_PCR_MUX(1) | PORT_PCR_PFE(1);
	PORTC->PCR[swPin] &= ~PORT_PCR_PE(1);

	// 3. Set data direction as input
	PTC->PDDR &= ~(1 << swPin);
}

void InitInterrupt(uint8_t swPin)
{
	// Enable interrupt on Falling Edge
	PORTC->PCR[swPin] |= PORT_PCR_IRQC(0xA);

	// Enable NVIC Interrupt
	INT_SYS_EnableIRQ(PORTC_IRQn);
}

// Interrupt Service Routine (ISR)
void PORTC_IRQHandler(void)
{
	if (PORTC->ISFR & (1 << SW2))
	{
		PTD->PTOR |= (1 << g_ledPin); // Toggle LED
		PORTC->ISFR = (1 << SW2);	  // Clear ISF
	}
}
