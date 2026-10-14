#include <kernel.h>
#include <kdata.h>

usize_t valaddr(const uint8_t *base, usize_t size, usize_t need, uint_fast8_t is_write)
{
        uaddr_t address = (uaddr_t)base;
        usize_t available = 0;
        /* Both bounds are checked before subtracting: a size near MAXUSIZE
         * must not wrap past the check, and an address above p_top must not
         * wrap into range. */
        if (address >= PROGLOAD && address < udata.u_ptab->p_top) {
                available = udata.u_ptab->p_top - address;
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
