#ifndef __MESSENGER_H__
#define __MESSENGER_H__

void init_messenger();

int messenger_send(int from, int to, char *msg, int msglen, char *reply, int replylen);
int messenger_receive(int to, int* tid, char *msg, int msglen);

#endif
