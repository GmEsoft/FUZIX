#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#define used(x) ((void)(x))
#define __packed __attribute__((packed))
#define le16_to_cpu(x) (x)
#define le32_to_cpu(x) (x)
#define cpu_to_le32(x) (x)
#define BLKSHIFT 9
#define BLKSIZE 512
#define MAX_BLKDEV 2
#define kputs(x) ((void)(x))
#define kprintf(...) ((void)0)
static unsigned printed;
static void kputchar(int c) { (void)c; printed++; }
typedef uintptr_t uarg_t;
typedef uint16_t blkno_t;
struct { int u_error; uint16_t u_nblock,u_block,u_page; uint8_t *u_dptr,*u_base; unsigned u_count; struct { struct { uint8_t bytes[16]; } addrbuf; } u_net; } udata;
static int d_blkoff(int n) { (void)n; return 0; }

#define SS_BOUND 3
struct ksockaddr{uint8_t bytes[16];};struct socket{struct ksockaddr src_addr,dst_addr;};static int netn_synchronous_event(struct socket*s,unsigned n){(void)s;(void)n;return 0;}
/* FUNCTION dev/net/net_native.c netproto_bind */
/* FUNCTION dev/net/net_native.c netproto_autobind */
int main(void){struct socket s={0};memset(&udata.u_net.addrbuf,42,16);assert(netproto_bind(&s)==0&&s.src_addr.bytes[0]==42&&s.dst_addr.bytes[0]==0);memset(&s,0,sizeof(s));assert(netproto_autobind(&s)==0&&s.src_addr.bytes[0]==42&&s.dst_addr.bytes[0]==0);return 0;}
