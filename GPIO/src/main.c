#include "S32K144.h"
#include <stdint.h>
#include <stdbool.h>

#define LED_RED 15   // PTD15
#define LED_GREEN 16 // PTD16
#define LED_BLUE 0   // PTD0

#define SW2 12 // PTC12
#define SW3 13 // PTC13

void delay(volatile int cycles);
void InitLED(uint8_t ledNumber);
void InitSW(uint8_t swNumber);
void ToggleLED(uint8_t ledNumber);

int main(void)
{
    InitLED(LED_RED);
    InitSW(SW2);

    // 1: At first, button is not pressed
    bool prevButtonState = true;

    while (true)
    {
        // 0: button is pressed
        // 1: button is not pressed
        bool currentButtonState = ((PTC->PDIR & (1 << SW2)) ? true : false);
        if (!prevButtonState && currentButtonState)
        {
            ToggleLED(LED_RED);
            delay(10000);
        }
        prevButtonState = currentButtonState;
    }
    return 0;
}

void delay(volatile int cycles)
{
    while (cycles--)
        ;
}

void InitLED(uint8_t ledNumber)
{
    PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC(1);

    PORTD->PCR[ledNumber] |= PORT_PCR_MUX(1); // Select MUX as GPIO
    PORTD->PCR[ledNumber] |= PORT_PCR_DSE(1); // Enable Drive Strength

    PTD->PDDR |= (1 << ledNumber);  // Set data direction as Output
    PTD->PDOR &= ~(1 << ledNumber); // Set default LED is ON
}

void InitSW(uint8_t swNumber)
{
    PCC->PCCn[PCC_PORTC_INDEX] |= PCC_PCCn_CGC(1);

    PORTC->PCR[swNumber] |= PORT_PCR_MUX(1);   // Select MUX as GPIO
    PORTC->PCR[swNumber] &= ~PORT_PCR_PE_MASK; // Disable pull-up
    PORTC->PCR[swNumber] |= PORT_PCR_PFE(1);   // Enable Passive Filter

    PTC->PDDR &= ~(1 << swNumber); // Set data direction as Input
}

void ToggleLED(uint8_t ledNumber)
{
    PTD->PTOR |= (1 << ledNumber);
}
