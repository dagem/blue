#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#define BLUE_VERSION "0.0.2"
#define BLUE_TAB_STOP 8
#define BLUE_QUIT_TIMES 3
#define CTRL_KEY(k) ((k) & 0x1F) //  Ctrl(k) = 0B, k & 0x1f = b

