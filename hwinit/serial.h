#ifndef _SERIAL_H
#define _SERIAL_H

void serial_putc (const char c);
void serial_puts (const char *s);
int serial_getc (void);
int serial_tstc (void);
void serial_init (void);
void serial_putb (unsigned int d);
void serial_puth (unsigned int d);
void serial_put_regb (const char *name, unsigned int value);
void serial_put_regh (const char *name, unsigned int value);

#endif

