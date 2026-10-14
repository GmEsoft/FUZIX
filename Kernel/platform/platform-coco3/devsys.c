#include <kernel.h>
#include <kdata.h>
#include <cpu_ioctl.h>

int plt_dev_ioctl(uarg_t request, char *data)
{
	switch (request) {
	case CPUIOC_6809SWI2:
		return uputw(data, (uint16_t *) 0xfc);
	case CPUIOC_6809SWI3:
		return uputw(data, (uint16_t *) 0xfe);
	default:
		return -1;
	}
}
