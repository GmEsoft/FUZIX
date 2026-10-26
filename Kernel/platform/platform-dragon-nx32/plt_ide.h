extern volatile uint8_t *ide_base;

#define	data	&ide_base[0]
#define error	&ide_base[1]
#define count	&ide_base[2]
#define sec	&ide_base[3]
#define cyll	&ide_base[4]
#define cylh	&ide_base[5]
#define devh	&ide_base[6]
#define status	&ide_base[7]
#define	cmd	&ide_base[7]

#define CONFIG_TD_IDE_SELECT

