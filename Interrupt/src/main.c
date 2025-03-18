#include "S32K144.h"
#include "interrupt_manager.h"

#define LED_GREEN 16 // PTD16
#define SW2 12		 // PTC12

void delay(volatile int cycles);
void InitLed(uint8_t ledPin);
void InitSW(uint8_t swPin);
void ToggleLed(uint8_t ledPin);
void InitInterrupt(uint8_t swPin);

void MyNVIC_EnableIRQ(uint8_t IRQn);
void MyNVIC_DisableIRQ(uint8_t IRQn);
void MyNVIC_SetPendingIRQ(uint8_t IRQn);
void MyNVIC_ClearPendingIRQ(uint8_t IRQn);
void MyNVIC_SetPriority(uint8_t IRQn, uint8_t priority);

volatile uint8_t g_ledPin = LED_GREEN;
volatile uint8_t g_swPin = SW2;

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

	// 2. Set MUX as GPIO, enable Passive Filter, disable  Pull-up, enable interrupt on Falling Edge
	PORTC->PCR[swPin] |= PORT_PCR_MUX(1) | PORT_PCR_PFE(1); // Set MUX as GPIO
	PORTC->PCR[swPin] &= ~PORT_PCR_PE(1);					// Enable Passive Filter
	PORTC->PCR[swPin] |= PORT_PCR_IRQC(0xA);				// Enable interrupt on Falling Edge

	// 3. Set data direction as input
	PTC->PDDR &= ~(1 << swPin);
}

void ToggleLed(uint8_t ledPin)
{
	PTD->PTOR |= (1 << ledPin);
}

void InitInterrupt(uint8_t swPin)
{
	// 1. Clear any pending IRQ
	INT_SYS_ClearPending(PORTC_IRQn);
	// MyNVIC_ClearPendingIRQ(PORTC_IRQn);

	// 2. Set priority
	INT_SYS_SetPriority(PORTC_IRQn, 201);
	// MyNVIC_SetPendingIRQ(PORTC_IRQn, 201);

	// 3. Enable NVIC Interrupt
	INT_SYS_EnableIRQ(PORTC_IRQn);
	// MyNVIC_EnableIRQ(PORTC_IRQn);
}

// Interrupt Service Routine (ISR)
void PORTC_IRQHandler(void)
{
	if (PORTC->ISFR & (1 << g_swPin))
	{
		ToggleLed(g_ledPin);
		PORTC->ISFR |= (1 << g_swPin);
	}
}

void MyNVIC_EnableIRQ(uint8_t IRQn)
{
	S32_NVIC->ISER[IRQn >> 5] |= (1 << (IRQn % 32));
}

void MyNVIC_DisableIRQ(uint8_t IRQn)
{
	S32_NVIC->ICER[IRQn >> 5] |= (1 << (IRQn % 32));
}

void MyNVIC_SetPendingIRQ(uint8_t IRQn)
{
	S32_NVIC->ISPR[IRQn >> 5] |= (1 << (IRQn % 32));
}

void MyNVIC_ClearPendingIRQ(uint8_t IRQn)
{
	S32_NVIC->ICPR[IRQn >> 5] |= (1 << (IRQn % 32));
}

void MyNVIC_SetPriority(uint8_t IRQn, uint8_t priority)
{
	S32_NVIC->IP[IRQn] = priority;
}
