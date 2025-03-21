#include "S32K144.h"
#include "interrupt_manager.h"
#include "stdint.h"

#define LED_GREEN 16 // PTD16

volatile uint8_t g_led = LED_GREEN;

void LED_init(uint8_t ledPin)
{
	// Enable clock for LPIT
	PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC_MASK;

	// Select MUX as GPIO and Enable DSE
	PORTD->PCR[ledPin] |= PORT_PCR_MUX(1);
	PORTD->PCR[ledPin] |= PORT_PCR_DSE_MASK;

	// Set data direction as output and turn LED on at first
	PTD->PDDR |= (1 << ledPin);
	PTD->PDOR &= ~(1 << ledPin);
}

void FIRCDIV_configure(void)
{
	if (!(SCG->FIRCCSR & SCG_FIRCCSR_FIRCEN_MASK))
	{
		SCG->FIRCCSR |= SCG_FIRCCSR_FIRCEN_MASK;
		while (!(SCG->FIRCCSR & SCG_FIRCCSR_FIRCVLD_MASK))
			;
	}
	SCG->FIRCDIV = SCG_FIRCDIV_FIRCDIV1(1) | SCG_FIRCDIV_FIRCDIV2(1);
}

void LPIT_init(void)
{
	// Settup clock for LPIT
	FIRCDIV_configure();
	PCC->PCCn[PCC_LPIT_INDEX] &= ~PCC_PCCn_CGC_MASK; // Disable clock gate before configuring
	PCC->PCCn[PCC_LPIT_INDEX] |= PCC_PCCn_PCS(3);	 // 0b011: FIRCDIV2_CLK (48Mhz)
	PCC->PCCn[PCC_LPIT_INDEX] |= PCC_PCCn_CGC_MASK;	 // Enable clock gate after configured

	// Reset and Enable LPIT
	LPIT0->MCR |= LPIT_MCR_SW_RST_MASK;	 // Reset LPIT
	LPIT0->MCR &= ~LPIT_MCR_SW_RST_MASK; // Disable Reset LPIT
	LPIT0->MCR |= LPIT_MCR_M_CEN_MASK;	 // Enable LPIT

	// Clear LPIT Channel 0 Interrupt Flag
	LPIT0->MSR |= LPIT_MSR_TIF0_MASK;

	// Settup Periodic Timer for 1 second (48Mhz clock)
	LPIT0->TMR[0].TVAL = 48000000;
	LPIT0->TMR[0].TCTRL |= LPIT_TMR_TCTRL_MODE(0);	 // 0b00 -> Periodic Counter
	LPIT0->TMR[0].TCTRL |= LPIT_TMR_TCTRL_T_EN_MASK; // Enable Timer

	// Enable LPIT Channel 0 Interrupt
	LPIT0->MIER |= LPIT_MIER_TIE0_MASK;

	// Register Interrupt Handler
	INT_SYS_EnableIRQ(LPIT0_Ch0_IRQn);
}

void LPIT0_Ch0_IRQHandler(void)
{
	if (LPIT0->MSR & LPIT_MSR_TIF0_MASK)
	{
		PTD->PTOR |= (1 << g_led);
		LPIT0->MSR |= LPIT_MSR_TIF0_MASK; // Clear LPIT interrupt flag
	}
}

int main(void)
{
	LED_init(g_led);
	LPIT_init();
	while (1)
	{
	}

	return 0;
}
