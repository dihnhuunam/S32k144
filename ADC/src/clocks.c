#include "clocks.h"

void SIRC_init_8Mhz(void)
{
    while (SCG->SIRCCSR & SCG_SIRCCSR_LK_MASK) // Ensure SIRC unlocked
        ;

    SCG->SIRCCSR &= ~SCG_SIRCCSR_SIRCEN_MASK;                          // Disable SIRC to configure
    SCG->SIRCDIV |= SCG_SIRCDIV_SIRCDIV1(1) | SCG_SIRCDIV_SIRCDIV2(1); // Divide by 1
    SCG->SIRCCFG |= SCG_SIRCCFG_RANGE(1);                              // High range clock 8Mhz

    while (SCG->SIRCCSR & SCG_SIRCCSR_LK_MASK) // Ensure SIRC unlocked
        ;

    SCG->SIRCCSR |= SCG_SIRCCSR_SIRCEN_MASK; // Enable SIRC

    while (!(SCG->SIRCCSR & SCG_SIRCCSR_SIRCVLD_MASK)) // Ensure SIRC Valid
        ;
}

void SOSC_init_40Mhz(void)
{
}

void SPLL_init_160Mhz(void)
{
}
