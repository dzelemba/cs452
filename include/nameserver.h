#ifndef __NAMESERVER_H__
#define __NAMESERVER_H__

void start_nameserver();


// Only to be used by kernel at end of program.
char* name_of(int tid);

#endif
