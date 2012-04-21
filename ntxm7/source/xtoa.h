#ifndef _XTOA_H_
#define _XTOA_H_

char *xtoa(unsigned long val, char *buf, unsigned radix, int negative);
char *itoa(int val, char *buf, int radix);
char *ltoa(long val, char *buf, int radix);
char *ultoa(unsigned long val, char *buf, int radix);

#endif
