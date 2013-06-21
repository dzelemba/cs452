#ifndef __STDIO_H__
#define __STDIO_H__

typedef char *va_list;

#define __va_argsiz(t)	\
		(((sizeof(t) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

#define va_start(ap, pN) ((ap) = ((va_list) __builtin_next_arg(pN)))

#define va_end(ap)	((void)0)

#define va_arg(ap, t)	\
		 (((ap) = (ap) + __va_argsiz(t)), *((t*) (void*) ((ap) - __va_argsiz(t))))

#define COM1	0
#define COM2	1

int putc(int channel, char c);

int putx(int channel, char c );

int putstr(int channel, char *str );

int putr(int channel, unsigned int reg );

void putw(int channel, int n, char fc, char *bf );

void printf(int channel, char *format, ... );

void io_format(int channel, char *fmt, va_list va );

#endif
