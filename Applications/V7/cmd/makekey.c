/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

/*
 * You send it 10 bytes.
 * It sends you 13 bytes.
 * The transformation is expensive to perform
 * (a significant part of a second).
 */

#include <stdlib.h>
#include <unistd.h>

int main(int argc, const char *argv[])
{
	char key[9];	/* +1: crypt() wants a NUL terminated string,
	  	  	   * read() stops at 8 and leaves one on the stack */
	char salt[3];	/* ditto: 2 salt characters plus the terminator */

	if (read(0, key, 8) != 8)
		return 1;
	key[8] = 0;
	if (read(0, salt, 2) != 2)
		return 1;
	salt[2] = 0;
	write(1, crypt(key, salt), 13);
	return(0);
}
