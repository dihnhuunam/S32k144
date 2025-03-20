#include "S32K144.h"
#include "clocks.h"

void SOSC_init_8Mhz(void)
{
    // Ensure SOSC is unlocked
    while (SCG->SOSCCSR & SCG_SOSCCSR_LK_MASK)
        ;

    // Disable SOSC before configuring
    SCG->SOSCCSR &= ~SCG_SOSCCSR_SOSCEN_MASK;

    // Select range: 0b10 -> Medium frequency (4Mhz to 8Mhz)
    // Select source for the external reference clock: XTAL
    SCG->SOSCCFG = SCG_SOSCCFG_RANGE(2) | SCG_SOSCCFG_EREFS_MASK;

    // Select DIV1 and DIV2: 0b001 -> Divide by 1
    SCG->SOSCDIV = SCG_SOSCDIV_SOSCDIV1(1) | SCG_SOSCDIV_SOSCDIV2(1);

    // Enable SOSC after configured
    SCG->SOSCCSR = SCG_SOSCCSR_SOSCEN_MASK;

    // Wait for SOSC valid
    while (!(SCG->SOSCCSR & SCG_SOSCCSR_SOSCVLD_MASK))
        ;
}

void SPLL_init_160Mhz(void)
{
    // Ensure SPLL is unlocked
    while (SCG->SPLLCSR & SCG_SPLLCSR_LK_MASK)
        ;

    // Disable SPLL before configuring
    SCG->SPLLCSR &= ~SCG_SPLLCSR_SPLLEN_MASK;

    // Select MULT: 0b01000 -> 24
    // Select PREDIV: 0b0000 -> 1
    SCG->SPLLCFG = SCG_SPLLCFG_MULT(8) | SCG_SPLLCFG_PREDIV(0);

    // Select DIV1: Divide by 2
    // Select DIV2: Divide by 4
    SCG->SPLLDIV |= SCG_SPLLDIV_SPLLDIV1(2) | SCG_SPLLDIV_SPLLDIV2(3);

    // Enable SPLL after configured
    SCG->SPLLCSR |= SCG_SPLLCSR_SPLLEN_MASK;

    // Wait for SPLL valid
    while (!(SCG->SPLLCSR & SCG_SPLLCSR_SPLLVLD_MASK))
        ;
}

void Normal_RUNmode_80Mhz(void)
{
    // Select SPLL as clock source (160Mhz)
    SCG->RCCR |= SCG_CSR_SCS(6);

    // Select SYS_CLK and CORE_CLK = SPLL_CLK / 2 (160Mhz / 2 = 80Mhz) -> DIVCORE = 0b0001
    SCG->RCCR |= SCG_RCCR_DIVCORE(1);

    // Select BUS_CLK = CORE_CLK / 2 -> (80Mhz / 2 = 40Mhz) -> DIVBUS = 0b0001
    SCG->RCCR |= SCG_RCCR_DIVBUS(1);

    // Select FLASH_CLK = CORE_CLK / 3 -> (80Mhz / 3 = ...) -> DIVSLOW = 0b0010
    SCG->RCCR |= SCG_RCCR_DIVSLOW(2);

    // Wait for clock source is SPLL
    while (((SCG->CSR & SCG_CSR_SCS_MASK) >> SCG_CSR_SCS_SHIFT) != 6)
        ;
}
