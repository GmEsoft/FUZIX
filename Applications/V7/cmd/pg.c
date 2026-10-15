#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <wait.h>

#define EOR '\n'
#define NL '\n'
#define CLEAR 014
#define HOME 031
#define BACK 037
#define BUFMAX 16384
#define PAGMAX 64

int file;
int cp, hp;
int page[PAGMAX];
int eof;

char buf[BUFMAX];
int bptr, bnxt, bend;


#define incr(a,b) (a++, a&=b-1)
#define decr(a,b) (a--, a&=b-1)


void losepage(void)
{
	if(hp<cp)
		bptr=page[incr(hp,PAGMAX)];
}

int ensure(void)
{
	for(;;) {
		if(bptr<=bend)
			return(BUFMAX-bend-1);
		if(bptr-bend>=512)
			return(512);
		if(hp==cp)
			return(0);	/* nothing left to discard */
		losepage();
	}
}


void fillbuf(void)
{
	register int r;
	register int n;

	n = ensure();
	if(eof)
		return;
	/* Never read past the end of buf. The original ignored ensure()
	   and read its full 512 bytes into &buf[bend] even when the tail
	   run was shorter, writing over bptr/bnxt and beyond, then
	   "wrapped" bend -- silently losing the overhang and corrupting
	   the buffer bookkeeping. If the tail has nothing to give, move
	   the write point to the free space at the start of the buffer;
	   the read point is only ever moved along with it because fillbuf
	   is entered solely with bnxt==bend. */
	if(n <= 0) {
		if(bnxt != bend)
			return;
		bend = 0;
		bnxt = 0;
		n = 512;
		if(bptr > 0 && bptr <= 512)
			losepage();	/* oldest kept page lies under us */
		if(bptr > 0 && bptr - 1 < n)
			n = bptr - 1;
		if(n <= 0)
			return;
	} else if(n > 512)
		n = 512;
	r=read(file,&buf[bend],n);
	if(r==-1)
		return;
	bend += r; bend &= BUFMAX-1;
	if(r==0)
		eof++;
}

int readchar(void)
{
	register char c;
	if(bnxt==bend)
		fillbuf();
	if(eof)
		return(-1);
	c=buf[bnxt];
	incr(bnxt,BUFMAX);
	return(c);
}

void setpage(void)
{
	incr(cp,PAGMAX);
	if(cp==hp)
		incr(hp,PAGMAX);
	page[cp]=bnxt;
}

void getpage(int i)
{
	if(i==0)
		cp=hp;
	else	if(cp!=hp)
			decr(cp,PAGMAX);
	bnxt=page[cp];
}


void shell(void)
{
	int rc, status, unixpid;
	if( (unixpid=fork())==0 ) {
		close(0); dup(2);
		execl("/bin/sh", "sh", "-t", 0);
		exit(255);
	}
	else if(unixpid == -1)
		return;
	else{	
		signal(SIGCHLD, SIG_DFL);
		signal(SIGINT,SIG_IGN); signal(SIGQUIT,SIG_IGN);
		while( (rc = wait(&status)) != unixpid && rc != -1 ) ;
		signal(SIGINT,SIG_DFL); signal(SIGQUIT,SIG_DFL);
		return;
	}
}

void print(void)
{
	register int nlc;
	char buf[2];

	hp=0; cp=0;
	bptr=bnxt=bend=0;
	eof=0;		/* else the second and later files are empty */
	putchar(CLEAR);
	for(;;) {
		setpage();
		nlc=0;
		putchar(BACK);
		while(nlc<20) {
			char c;
			c = readchar();
			if(eof)
				return;
			if(c==NL) nlc++;
			putchar(c);
		}
		while(read(2,buf,1)==1 && buf[0]!=NL) {
			switch(buf[0]) {
				case '/':
				case HOME:
					putchar(CLEAR);
					getpage(0);
					break;
				case '-':
				case BACK:
					getpage(-1);
					putchar(CLEAR);
					break;
				case '!':
					shell(); buf[0]=NL; break;
			}
		}
	}
}


int main(int argc, char *argv[])
{
	int n=1;
	if(argc<2)
		print();
	else {
		while(argv[n] != NULL) {
			if((file=open(argv[n],0))>=0) {
				print();
				close(file);
			} else	printf("pg: `%s' cannot open\n",argv[n]);
			n++;
		}
	}
	return 0;
}
