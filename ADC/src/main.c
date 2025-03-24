#include "S32K144.h"
#include "clocks.h"
#include "stdint.h"
#include "interrupt_manager.h"

#define LED_RED 15		  // PTD15
#define LED_GREEN 16	  // PTD16
#define LED_BLUE 0		  // PTD0
#define POT_CHANNEL 12	  // Channel 12 for potentiometer
#define VREFSH_CHANNEL 29 // Voltage reference: VREFSH

void Delay(volatile int cycles)
{
	while (cycles--)
		;
}

void LED_init(void)
{
	// Enable clock for PORTD
	PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC_MASK;

	// Select MUX as GPIO
	PORTD->PCR[LED_RED] = PORT_PCR_MUX(1);
	PORTD->PCR[LED_GREEN] = PORT_PCR_MUX(1);
	PORTD->PCR[LED_BLUE] = PORT_PCR_MUX(1);

	// Set data direction as output
	PTD->PDDR |= (1 << LED_RED) | (1 << LED_GREEN) | (1 << LED_BLUE);

	// Turn off all LEDs initially
	PTD->PSOR |= (1 << LED_RED) | (1 << LED_GREEN) | (1 << LED_BLUE);
}

// Turn LED on
void LED_on(uint8_t ledPin)
{
	PTD->PCOR |= (1 << ledPin);
}

// Turn LED off
void LED_off(uint8_t ledPin)
{
	PTD->PSOR |= (1 << ledPin);
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
	// Keep your original clock source (FIRCDIV)
	PCC->PCCn[PCC_ADC0_INDEX] &= ~PCC_PCCn_CGC_MASK;
	PCC->PCCn[PCC_ADC0_INDEX] |= PCC_PCCn_PCS(3); // 0b011 -> FIRCDIV2_CLK (48Mhz)
	PCC->PCCn[PCC_ADC0_INDEX] |= PCC_PCCn_CGC_MASK;

	// Select the mode of operation, clock source, clock divide
	ADC0->CFG1 = ADC_CFG1_MODE(1)	  // 0b01 -> 12-bit resolution
				 | ADC_CFG1_ADICLK(0) // ALTCLK1: FIRCDIV2_CLK (48Mhz)
				 | ADC_CFG1_ADIV(0);  // Divide ratio = 1

	// Select sample time
	ADC0->CFG2 |= ADC_CFG2_SMPLTS(12);

	// Select conversion trigger, voltage reference
	ADC0->SC2 = ADC_SC2_ADTRG(0)	 // Software Trigger
				| ADC_SC2_REFSEL(0); // External pins VREFH and VREFL

	// Select type of conversion: single-ended
	ADC0->SC3 &= ~ADC_SC3_ADCO_MASK; // One conversion will be performed

	// Enable interrupt
	ADC0->SC1[0] |= ADC_SC1_AIEN_MASK;
}

void Select_channel(uint8_t channel)
{
	// Clear prior channel and select channel
	ADC0->SC1[0] &= ~ADC_SC1_ADCH_MASK;
	ADC0->SC1[0] |= ADC_SC1_ADCH(channel);
}

uint8_t ADC_complete(void)
{
	return (ADC0->SC1[0] & ADC_SC1_COCO_MASK) >> ADC_SC1_COCO_SHIFT;
}

uint32_t Read_ADC(void)
{
	uint32_t adc_result = 0;
	adc_result = ADC0->R[0];
	return (uint32_t)((adc_result * 5000) / 0xFFF); // Convert result to mv for 0-5V range
}

void Interrupt_init(void)
{
	// Clear pending
	INT_SYS_ClearPending(ADC0_IRQn);

	// Set priority
	INT_SYS_SetPriority(ADC0_IRQn, 201);

	// Enable Interrupt
	INT_SYS_EnableIRQ(ADC0_IRQn);
}

void ADC0_IRQHandler(void)
{
	uint32_t adcResultInMV = 0; // ADC0 result in millivolts
	static uint8_t current_channel = POT_CHANNEL;

	// Read the result of the completed conversion
	adcResultInMV = Read_ADC();

	// Process result based on which channel was converted
	if (current_channel == POT_CHANNEL)
	{
		// Control LEDs based on ADC result
		if (adcResultInMV > 3750)
		{ // If result > 3.75V, turn led red on
			LED_on(LED_RED);
			LED_off(LED_BLUE);
			LED_off(LED_GREEN);
		}
		else if (adcResultInMV > 2500)
		{ // If result > 2.5V, turn led green on
			LED_off(LED_RED);
			LED_on(LED_GREEN);
			LED_off(LED_BLUE);
		}
		else if (adcResultInMV > 1250)
		{ // If result > 1.25V, turn led blue on
			LED_off(LED_RED);
			LED_off(LED_GREEN);
			LED_on(LED_BLUE);
		}
		else
		{
			LED_off(LED_RED);
			LED_off(LED_GREEN);
			LED_off(LED_BLUE);
		}

		// Switch to the next channel for the next conversion
		current_channel = VREFSH_CHANNEL; // VREFSH channel
	}
	else
	{
		// Switch back to the potentiometer channel
		current_channel = POT_CHANNEL;
	}

	// Start the next conversion
	Select_channel(current_channel);
}

int main(void)
{
	FIRCDIV_configure();
	ADC_init();
	LED_init();
	Interrupt_init();

	// Start the first conversion to kick off the interrupt cycle
	Select_channel(POT_CHANNEL);

	while (1)
	{
	}

	return 0;
}