#ifndef __CHANNELIO_H__
#define __CHANNELIO_H__

typedef char *va_list;

#define __va_argsiz(t)	\
		(((sizeof(t) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

#define va_start(ap, pN) ((ap) = ((va_list) __builtin_next_arg(pN)))

#define va_end(ap)	((void)0)

#define va_arg(ap, t)	\
		 (((ap) = (ap) + __va_argsiz(t)), *((t*) (void*) ((ap) - __va_argsiz(t))))

#define COM1	0
#define COM2	1

#define MAX_STRING_SIZE 127

#include "string.h"

void putc(int channel, char c);

void putx(int channel, char c);

void putstr(int channel, char *str);

void putbytes(int channel, char *str, int size);

void putr(int channel, unsigned int reg);

void putw(int channel, int n, char fc, char *bf);

void printf(int channel, char *format, ...);

void io_format(int channel, char *fmt, va_list va);

void sprintf(string* s, char* format, ...);

void s_format(string* s, char* fmt, va_list va);

int bwputc( int channel, char c );

int bwgetc( int channel );

int bwputx( int channel, char c );

int bwputstr( int channel, char *str );

int bwputr( int channel, unsigned int reg );

void bwputw( int channel, int n, char fc, char *bf );

void bwprintf( int channel, char *format, ... );

void bwformat( int channel, char *fmt, va_list va );
#endif
