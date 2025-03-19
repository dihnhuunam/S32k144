#ifndef _CLOCKS_H_
#define _CLOCKS_H_

#include "S32K144.h"

void SIRC_init_8Mhz(void);
void SOSC_init_40Mhz(void);
void SPLL_init_160Mhz(void);

#endif /* _CLOCKS_H_ */
