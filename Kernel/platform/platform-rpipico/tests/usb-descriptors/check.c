#include <assert.h>
#include <stdint.h>
#include <stdio.h>
const uint8_t *tud_descriptor_configuration_cb(uint8_t);
int main(void)
{
    const uint8_t *d=tud_descriptor_configuration_cb(0);
    unsigned length=d[2]+256u*d[3], count=0;
    uint32_t interfaces=0;
    assert(d[0]==9 && d[1]==2 && d[4]==NUM_DEV_TTY_USB*2);
    for (unsigned i=0;i<length;) {
        assert(d[i]>=2 && i+d[i]<=length);
        if (d[i+1]==4) {
            assert(d[i]>=9 && d[i+3]==0 && d[i+2]<32);
            assert(!(interfaces&(1u<<d[i+2])));
            interfaces|=1u<<d[i+2]; count++;
        }
        i+=d[i];
    }
    assert(count==d[4] && interfaces==(1u<<count)-1);
    printf("%u CDC ports: %u interfaces, %u descriptor bytes\n",NUM_DEV_TTY_USB,count,length);
}
