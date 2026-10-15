#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <exec.h>

uaddr_t pagemap_base(void)
{
    return DATABASE;
}

/* This checks to see if a user-supplied address is legitimate */
usize_t valaddr(const uint8_t *base, usize_t size, usize_t need, uint_fast8_t is_write)
{
	uaddr_t address = (uaddr_t)base;
	usize_t available = 0;

	/*
	 *	A user process owns [PROGLOAD, u_top). The UDATA_SIZE bytes
	 *	below PROGLOAD hold the kernel's u_data for the process, so
	 *	they must not be reachable as a user pointer. Both bounds are
	 *	tested before any subtraction: a size near MAXUSIZE must not
	 *	wrap past the check, and an address above u_top must not wrap
	 *	into range.
	 */
	if (address >= PROGLOAD && address < (uaddr_t)udata.u_top) {
		available = (uaddr_t)udata.u_top - address;
		if (size > available)
			size = available;
	} else {
		size = 0;
	}
	if (size < need) {
		udata.u_error = EFAULT;
		return 0;
	}
	return size;
}

usize_t valaddr_r(const uint8_t *pp, usize_t l, usize_t n)
{
	return valaddr(pp, l, n, 0);
}

usize_t valaddr_w(const uint8_t *pp, usize_t l, usize_t n)
{
	return valaddr(pp, l, n, 1);
}

void swap_blocks(void* p1, void* p2, size_t len)
{
    uint32_t* u1 = p1;
    uint32_t* u2 = p2;

    while (len != 0)
    {
        uint32_t t = *u1;
        *u1 = *u2;
        *u2 = t;
        u1++;
        u2++;
        len -= 4;
    }
}

/* vim: sw=4 ts=4 et: */


