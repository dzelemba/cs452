#ifndef __MESSENGER_H__
#define __MESSENGER_H__

void init_messenger();

void messenger_send(int from, int to, char *msg, int msglen, char *reply, int replylen);
int messenger_receive(int receiver, int* tid, char *msg, int msglen);
int messenger_reply(int tid, char *reply, int replylen);
void messenger_incomplete(int tid);

#endif
