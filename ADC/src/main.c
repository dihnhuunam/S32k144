#include "S32K144.h"
#include "clocks.h"
#include "stdint.h"

int main(void)
{
	SOSC_init_8Mhz();
	SPLL_init_160Mhz();
	Normal_RUNmode_80Mhz();

	while (1)
	{
	}
	return 0;
}
