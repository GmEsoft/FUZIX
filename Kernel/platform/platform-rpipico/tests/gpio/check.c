#include <kernel.h>
#include <gpio.h>
#include <hardware/gpio.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
struct u_data udata;
static unsigned functions[30];
static bool directions[30], outputs[30], inputs[30];
static bool fail_get, fail_put;
unsigned gpio_get_function(unsigned pin) { assert(pin<30); return functions[pin]; }
bool gpio_get_dir(unsigned pin) { assert(pin<30); return directions[pin]; }
bool gpio_get_out_level(unsigned pin) { assert(pin<30); return outputs[pin]; }
bool gpio_get(unsigned pin) { assert(pin<30); return inputs[pin]; }
void gpio_set_function(unsigned pin,unsigned fn) { assert(pin<30); functions[pin]=fn; }
void gpio_set_dir(unsigned pin,bool out) { assert(pin<30); directions[pin]=out; }
void gpio_put(unsigned pin,bool out) { assert(pin<30); outputs[pin]=out; }
void gpio_init(unsigned pin) { assert(pin<30); functions[pin]=GPIO_FUNC_SIO; directions[pin]=outputs[pin]=false; }
int uget(const void *p,void *out,unsigned n) { if(fail_get) { udata.u_error=EFAULT; return -1; } memcpy(out,p,n); return 0; }
int uput(const void *p,void *out,unsigned n) { if(fail_put) { udata.u_error=EFAULT; return -1; } memcpy(out,p,n); return 0; }
static int call(unsigned request,unsigned pin,unsigned value)
{
    struct gpioreq gr={pin,value};
    return gpio_ioctl(request,(char *)&gr);
}
int main(int argc,char **argv)
{
    assert(argc==2);
    for(unsigned i=0;i<30;i++) functions[i]=GPIO_FUNC_SIO;
    if(!strcmp(argv[1],"masks")) {
        assert(!call(GPIOC_SET,10,0x05));
        assert(outputs[8] && outputs[10] && !outputs[9] && !outputs[11]);
        assert(!directions[8] && !directions[10]);
        assert(!call(GPIOC_SET,8,0x80) && outputs[15]);
        assert(!call(GPIOC_CLR,9,0x01) && !outputs[8] && outputs[10] && outputs[15]);
        assert(!call(GPIOC_SETBYTE,8,0x02));
        for(unsigned i=8;i<16;i++) assert(outputs[i]==(i==9));
        assert(!call(GPIOC_SET,28,0x30) && outputs[28] && outputs[29]);
    } else if(!strcmp(argv[1],"direction")) {
        functions[8]=functions[9]=2; directions[10]=true; outputs[8]=true;
        assert(!call(GPIOC_SETRW,8,0x01));
        assert(functions[8]==GPIO_FUNC_SIO && directions[8] && outputs[8]);
        assert(functions[9]==2 && !directions[9] && !directions[10]);
        assert(!call(GPIOC_SETRW,28,0xff) && directions[28] && directions[29]);
    } else if(!strcmp(argv[1],"input")) {
        inputs[24]=inputs[28]=inputs[29]=true; directions[28]=true;
        assert(call(GPIOC_GETBYTE,28,0)==0x21);
    } else if(!strcmp(argv[1],"info")) {
        struct gpio info={.pinmask=28};
        directions[28]=outputs[29]=true;
        assert(gpio_ioctl(GPIOC_COUNT,NULL)==30);
        assert(!gpio_ioctl(GPIOC_GETINFO,(char *)&info));
        assert(info.pinmask==0x3f && info.group==3 && info.flags==GPIO_BIT_CONFIG);
        assert(info.wmask==0x10 && info.wdata==0x20 && !strcmp((char *)info.name,"GPIO24"));
    } else if(!strcmp(argv[1],"errors")) {
        assert(call(GPIOC_SET,30,1)==-1 && udata.u_error==ENODEV);
        assert(call(GPIOC_SET,255,1)==-1 && udata.u_error==ENODEV);
        fail_get=true;
        assert(gpio_ioctl(0x9999,NULL)==-1 && udata.u_error==ENOTTY);
        assert(call(GPIOC_SET,0,1)==-1 && udata.u_error==EFAULT);
        fail_get=false; fail_put=true;
        struct gpio info={0};
        assert(gpio_ioctl(GPIOC_GETINFO,(char *)&info)==-1 && udata.u_error==EFAULT);
    } else abort();
    puts(argv[1]);
}
