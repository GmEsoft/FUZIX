/*
  Header file for manipulation of the fuxiz console keyboard
*/

#ifndef _SYS_KD_H
#define _SYS_KD_H

/* IOCTL numbers */
#define KBMAPSIZE	0x20
#define KBMAPGET	0x21
#define KBSETTRANS	(0x23|__IOCTL_SUPER)
#define KBRATE          0x25


/* struct for setting keyboard repeat
   delays are demarked in tenths of seconds
 */
struct key_repeat {
	uint8_t first;
	uint8_t continual;
};

#endif

