#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char *arg0;
static unsigned squishy = 0;
static unsigned number;
static unsigned num_nb;
static unsigned break5;
static unsigned show_end;
static unsigned show_ctrl;
static unsigned show_tabs;
static unsigned nyan;
static unsigned fanfold;

void usage(void)
{
    fprintf(stderr, "%s [-5] [-b] [-e] [-f] [-n] [-s] [-t] files...\n", arg0);
    exit(1);
}

void decorate(unsigned line)
{
    if (fanfold && (line & 1))
        printf("\033[48;5;156m");
    if (nyan) {
        switch(line % 6) {
        case 0:
            printf("\033[48;5;196m");
            break;
        case 1:
            printf("\033[48;5;208m");
            break;
        case 2:
            printf("\033[48;5;226m");
            break;
        case 3:
            printf("\033[48;5;118m");
            break;
        case 4:
            printf("\033[48;5;69m");
            break;
        case 5:
            printf("\033[48;5;99m");
            break;
        }
    }
}

void end_decorate(unsigned line)
{
    if (fanfold || nyan)
        printf("\033[K\033[0m");
}

void do_text(const char *buf)
{
    unsigned char c;
    while((c = *buf) != 0 && c != '\n') {
        if ((show_ctrl && c != '\t') || (show_tabs && c == '\t')) {
            if (c > 127) {
                printf("M-");
                c &= 127;
            }
            if (c < 32)
                printf("^%c", c + 32);
            else
                putchar(c);
        } else
            putchar(c);
        buf++;
    }
    if (show_end) {
        if (show_ctrl)
            printf("^M");
        putchar('$');
    }
}


int cat_file(FILE *fp)
{
    unsigned line = 0;
    char buf[512];
    while(fgets(buf, sizeof(buf), fp)) {
        if (*buf == '\n') {
            if (squishy) {
                if (!num_nb)
                    line++;
                continue;
            }
        }
        line++;
        decorate(line);
        if (number)
            printf("%6d ", line);
        do_text(buf);
        end_decorate(line);
        putchar('\n');
        if (break5 && (line % 5) == 0)
            putchar('\n');
    }
    if (ferror(fp))
        return 1;
    return 0;
}


int main(int argc, char *const argv[])
{
    int opt;
    int err = 0;

    arg0 = strrchr(*argv, '/');
    if (arg0)
        arg0++;
    else
        arg0 = *argv;

    while ((opt = getopt(argc, argv, "5befnstv")) != -1) {
        switch(opt) {
        case '5':
            break5 = 1;
            break;
        case 'b':
            num_nb = 1;
            number = 1;
            squishy = 1;
            break;
        case 'e':
            show_end = 1;
            break;
        case 'f':
            fanfold = 1;
            break;
        case 'n':
            number = 1;
            break;
        case 's':
            squishy = 1;
            break;
        case 't':
            show_tabs = 1;
            break;
        case 'v':
            show_ctrl = 1;
            break;
        default:
            usage();
        }
    }
    if (strcmp(arg0, "nyancat") == 0)
        nyan = 1;

    argv += optind;
    if (*argv == NULL)
        err = cat_file(stdin);
    else while(*argv) {
        FILE *fp = fopen(*argv, "r");
        if (fp == NULL)
            perror(*argv);
        else {
            err |= cat_file(fp);
            fclose(fp);
        }
        argv++;
    }
    if (fanfold || nyan)
        printf("\033[0m");
    return err;
}
