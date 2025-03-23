#include "S32K144.h"
#include "clocks.h"
#include "stdint.h"

#define LED_RED 15	 // PTD15
#define LED_GREEN 16 // PTD16
#define LED_BLUE 0	 // PTD0

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

void ADC_init(void)
{
	// Select clock for ADC
	PCC->PCCn[PCC_ADC0_INDEX] &= ~PCC_PCCn_CGC_MASK;
	PCC->PCCn[PCC_ADC0_INDEX] |= PCC_PCCn_PCS(3); // 0b011 -> FIRCDIV2_CLK (48Mhz)
	PCC->PCCn[PCC_ADC0_INDEX] |= PCC_PCCn_CGC_MASK;

	// Select the mode of operation, clock source, clock divide.
	ADC0->CFG1 = ADC_CFG1_MODE(1)	  // 0b01 -> 12-bit resolution
				 | ADC_CFG1_ADICLK(1) // FIRCDIV2_CLK (48Mhz)
				 | ADC_CFG1_ADIV(1);  // Divide ratio = 1

	// Select sample time
	ADC0->CFG2 |= ADC_CFG2_SMPLTS(12);

	// Select conversion trigger, voltage reference
	ADC0->SC2 = ADC_SC2_ADTRG(0)	 // Software Trigger
				| ADC_SC2_REFSEL(0); // External pins VREFH and VREFL

	// Select type of conversion: single-ended
	ADC0->SC3 &= ~ADC_SC3_ADCO_MASK; // One conversion will be performed
}

void Convert_channel(uint8_t channel)
{
	// Clear prior channel and select channel
	ADC0->SC1[0] &= ~ADC_SC1_ADCH_MASK;
	ADC0->SC1[0] |= ADC_SC1_ADCH(channel);

	// Wait for ADC0 changes channel
	while (!(ADC0->SC1[0] & ADC_SC1_COCO_MASK))
		;
}

uint32_t Read_ADC(void)
{
	uint32_t adc_result = ADC0->R[0];
	return adc_result;
}

int main(void)
{
	LED_init(LED_GREEN);
	LED_init(LED_RED);
	LED_init(LED_BLUE);
	ADC_init();

	while (1)
	{
	}
	return 0;
}