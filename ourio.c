#include "ourio.h"
#include "syscall.h"
#include "ts7200.h"
#include "ourlib.h"
#include "string.h"
#include "debug.h"

#define create_string() string s; char mem[MAX_STRING_SIZE]; str_create(&s, mem, MAX_STRING_SIZE);
#define wrapper(exp) create_string(); exp ; _putstring(channel, &s);

void _bwputc(int channel, char c) {
  volatile int *flags, *data;
  switch (channel) {
  case COM1:
    flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
    data = (int *)( UART1_BASE + UART_DATA_OFFSET );
    break;
  case COM2:
    flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );
    data = (int *)( UART2_BASE + UART_DATA_OFFSET );
    break;
  default:
    ERROR("stdio.c: bwputc: Invalid channel");
    return;
  }
  while( ( *flags & TXFF_MASK ) ) ;
  *data = c;
}

void _putstring(int channel, string* s) {
  char* chars = str_get_chars(s);
  int size = str_get_size(s);
  if (in_userspace()) {
    Putstr(channel, chars, size);
  } else {
    int i;
/*
    _bwputc(channel, 'b');
    _bwputc(channel, 'w');
    _bwputc(channel, ' ');
*/
    for (i = 0; i < size; i++) {
      _bwputc(channel, chars[i]);
    }
  }
}

void _putc(string* s, char c) {
  str_append(s, c);
}

void putc(int channel, char c) {
  wrapper(_putc(&s, c));
}

char c2x(char ch) {
  if ( (ch <= 9) ) return '0' + ch;
  return 'a' + ch - 10;
}

void _putx(string* s, char c) {
  char chh, chl;

  chh = c2x( c / 16 );
  chl = c2x( c % 16 );
  _putc(s, chh);
  _putc(s, chl);
}

void putx(int channel, char c) {
  wrapper(_putx(&s, c));
}

void _putr(string* s, unsigned int reg) {
  int byte;
  char *ch = (char *) &reg;

  for( byte = 3; byte >= 0; byte-- ) _putx(s, ch[byte]);
  _putc(s, ' ');
}

void putr(int channel, unsigned int reg) {
  wrapper(_putr(&s, reg));
}

void _putstr(string* s, char *str) {
  while( *str ) {
     _putc(s, *str );
    str++;
  }
}

void putstr(int channel, char *str) {
  wrapper(_putstr(&s, str));
}

void _putbytes(string* s, char *str, int size) {
  int i;
  for (i = 0; i < size; i++) {
    _putc(s, str[i]);
  }
}

void putbytes(int channel, char *str, int size) {
  wrapper(_putbytes(&s, str, size));
}


void _putw(string* s, int n, char fc, char *bf) {
  char ch;
  char *p = bf;

  while( *p++ && n > 0 ) n--;
  while( n-- > 0 ) _putc(s, fc);
  while( ( ch = *bf++ ) ) _putc(s, ch);
}

void putw(int channel, int n, char fc, char *bf) {
  wrapper(_putw(&s, n, fc, bf));
}

int a2d( char ch ) {
  if( ch >= '0' && ch <= '9' ) return ch - '0';
  if( ch >= 'a' && ch <= 'f' ) return ch - 'a' + 10;
  if( ch >= 'A' && ch <= 'F' ) return ch - 'A' + 10;
  return -1;
}

char a2i( char ch, char **src, int base, int *nump ) {
  int num, digit;
  char *p;

  p = *src; num = 0;
  while( ( digit = a2d( ch ) ) >= 0 ) {
    if ( digit > base ) break;
    num = num*base + digit;
    ch = *p++;
  }
  *src = p; *nump = num;
  return ch;
}

void ui2a( unsigned int num, unsigned int base, char *bf ) {
  int n = 0;
  int dgt;
  unsigned int d = 1;

  while( (num / d) >= base ) d *= base;
  while( d != 0 ) {
    dgt = num / d;
    num %= d;
    d /= base;
    if( n || dgt > 0 || d == 0 ) {
      *bf++ = dgt + ( dgt < 10 ? '0' : 'a' - 10 );
      ++n;
    }
  }
  *bf = 0;
}

void i2a(int num, char *bf) {
  if( num < 0 ) {
    num = -num;
    *bf++ = '-';
  }
  ui2a(num, 10, bf);
}

void _io_format (string* s, char *fmt, va_list va) {
  char bf[12];
  char ch, lz;
  int w;

  while ( ( ch = *(fmt++) ) ) {
    if ( ch != '%' )
      _putc(s, ch);
    else {
      lz = '0'; w = 0;
      ch = *(fmt++);
      switch (ch) {
      case '0':
        lz = 1; ch = *(fmt++);
        break;
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        ch = a2i(ch, &fmt, 10, &w);
        break;
      }
      switch( ch ) {
      case 0: return;
      case 'c':
        _putc(s, va_arg(va, char));
        break;
      case 's':
        _putw(s, w, ' ', va_arg( va, char* ));
        break;
      case 'u':
        ui2a(va_arg( va, unsigned int ), 10, bf);
        _putw(s, w, lz, bf);
        break;
      case 'd':
        i2a(va_arg( va, int ), bf);
        _putw(s, w, lz, bf);
        break;
      case 'x':
        ui2a(va_arg( va, unsigned int ), 16, bf);
        _putw(s, w, lz, bf);
        break;
      case '%':
        _putc(s, ch);
        break;
      }
    }
  }
}

void io_format (int channel, char *fmt, va_list va) {
  wrapper(_io_format(&s, fmt, va));
}

void sprintf(string* s, char* format, ...) {
  va_list va;

  va_start(va,format);
  _io_format(s, format, va);
  va_end(va);
}

void s_format(string* s, char* fmt, va_list va) {
  _io_format(s, fmt, va);
}

void printf(int channel, char *fmt, ...) {
  va_list va;

  va_start(va,fmt);
  io_format( channel, fmt, va );
  va_end(va);
}

// BWIO

int bwputc( int channel, char c ) {
  volatile int *flags, *data;
  switch( channel ) {
  case COM1:
    flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
    data = (int *)( UART1_BASE + UART_DATA_OFFSET );
    break;
  case COM2:
    flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );
    data = (int *)( UART2_BASE + UART_DATA_OFFSET );
    break;
  default:
    return -1;
    break;
  }
  while( ( *flags & TXFF_MASK ) ) ;
  *data = c;
  return 0;
}

int bwputx( int channel, char c ) {
  char chh, chl;

  chh = c2x( c / 16 );
  chl = c2x( c % 16 );
  bwputc( channel, chh );
  return bwputc( channel, chl );
}

int bwputr( int channel, unsigned int reg ) {
  int byte;
  char *ch = (char *) &reg;

  for( byte = 3; byte >= 0; byte-- ) bwputx( channel, ch[byte] );
  return bwputc( channel, ' ' );
}

int bwputstr( int channel, char *str ) {
  while( *str ) {
    if( bwputc( channel, *str ) < 0 ) return -1;
    str++;
  }
  return 0;
}

void bwputw( int channel, int n, char fc, char *bf ) {
  char ch;
  char *p = bf;

  while( *p++ && n > 0 ) n--;
  while( n-- > 0 ) bwputc( channel, fc );
  while( ( ch = *bf++ ) ) bwputc( channel, ch );
}

int bwgetc( int channel ) {
  int *flags, *data;
  unsigned char c;

  switch( channel ) {
  case COM1:
    flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
    data = (int *)( UART1_BASE + UART_DATA_OFFSET );
    break;
  case COM2:
    flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );
    data = (int *)( UART2_BASE + UART_DATA_OFFSET );
    break;
  default:
    return -1;
    break;
  }
  while ( !( *flags & RXFF_MASK ) ) ;
  c = *data;
  return c;
}

int bwa2d( char ch ) {
  if( ch >= '0' && ch <= '9' ) return ch - '0';
  if( ch >= 'a' && ch <= 'f' ) return ch - 'a' + 10;
  if( ch >= 'A' && ch <= 'F' ) return ch - 'A' + 10;
  return -1;
}

char bwa2i( char ch, char **src, int base, int *nump ) {
  int num, digit;
  char *p;

  p = *src; num = 0;
  while( ( digit = bwa2d( ch ) ) >= 0 ) {
    if ( digit > base ) break;
    num = num*base + digit;
    ch = *p++;
  }
  *src = p; *nump = num;
  return ch;
}

void bwui2a( unsigned int num, unsigned int base, char *bf ) {
  int n = 0;
  int dgt;
  unsigned int d = 1;

  while( (num / d) >= base ) d *= base;
  while( d != 0 ) {
    dgt = num / d;
    num %= d;
    d /= base;
    if( n || dgt > 0 || d == 0 ) {
      *bf++ = dgt + ( dgt < 10 ? '0' : 'a' - 10 );
      ++n;
    }
  }
  *bf = 0;
}

void bwi2a( int num, char *bf ) {
  if( num < 0 ) {
    num = -num;
    *bf++ = '-';
  }
  bwui2a( num, 10, bf );
}

void bwformat ( int channel, char *fmt, va_list va ) {
  char bf[12];
  char ch, lz;
  int w;


  while ( ( ch = *(fmt++) ) ) {
    if ( ch != '%' )
      bwputc( channel, ch );
    else {
      lz = 0; w = 0;
      ch = *(fmt++);
      switch ( ch ) {
      case '0':
        lz = 1; ch = *(fmt++);
        break;
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        ch = bwa2i( ch, &fmt, 10, &w );
        break;
      }
      switch( ch ) {
      case 0: return;
      case 'c':
        bwputc( channel, va_arg( va, char ) );
        break;
      case 's':
        bwputw( channel, w, 0, va_arg( va, char* ) );
        break;
      case 'u':
        bwui2a( va_arg( va, unsigned int ), 10, bf );
        bwputw( channel, w, lz, bf );
        break;
      case 'd':
        bwi2a( va_arg( va, int ), bf );
        bwputw( channel, w, lz, bf );
        break;
      case 'x':
        bwui2a( va_arg( va, unsigned int ), 16, bf );
        bwputw( channel, w, lz, bf );
        break;
      case '%':
        bwputc( channel, ch );
        break;
      }
    }
  }
}

void bwprintf( int channel, char *fmt, ... ) {
  va_list va;

  va_start(va,fmt);
  bwformat( channel, fmt, va );
  va_end(va);
}

