#include "nameserver.h"
#include "task.h"
#include "queue.h"
#include "strings.h"
#include "syscall.h"
#include "priorities.h"
#include "debug.h"

#define NAMESERVER_PRIORITY MED_PRI_K

#define WHO_IS 0
#define REGISTER_AS 1

typedef struct nameserver_request {
  int type;
  char* name;
  int tid;
} nameserver_request;

#define MAX_NAME_LENGTH 16
#define MAX_ENTRIES 64

typedef struct entry {
  char name[MAX_NAME_LENGTH];
  int tid;
} entry;

static entry entries[MAX_ENTRIES];
static int next_entry;

static int nameserver_tid;

int find_entry(char* name) {
  int i;
  for (i = 0; i < next_entry; i++) {
    if (string_equal(entries[i].name, name)) {
      return i;
    }
  }
  return -1;
}

int who_is(char* name) {
  int index = find_entry(name);
  if (index == -1) {
    return -1;
  } else {
    return entries[index].tid;
  }
}

int register_as(int tid, char* name) {
  if (next_entry == MAX_ENTRIES) {
    ERROR("Nameserver ran out of name entries\n");
    return -1;
  }

  // Check if this name has already been registered.
  int index = find_entry(name);
  if (index == -1) {
    index = next_entry++;
  }

  entry* new_entry = &entries[index];

  new_entry->tid = tid;
  string_copy(name, MAX_NAME_LENGTH, new_entry->name);
  return 0;
}

void nameserver_run() {
  int tid;
  nameserver_request req;
  while (1) {
    Receive(&tid, (char *)&req, sizeof(nameserver_request));
    int retval;
    switch (req.type) {
      case REGISTER_AS:
        retval = register_as(req.tid, req.name);
        Reply(tid, (char *)&retval, sizeof(int));
        break;
      case WHO_IS:
        retval = who_is(req.name);
        Reply(tid, (char *)&retval, sizeof(int));
        break;
      default:
        ERROR("Invalid nameserver request type %d, from: %d\n", req.type, req.tid);
    }
  }
}

/* Public Methods */

void start_nameserver() {
  next_entry = 0;
  nameserver_tid = Create(NAMESERVER_PRIORITY, &nameserver_run);
}

int RegisterAs(char* name) {
  nameserver_request req;
  req.type = REGISTER_AS;
  req.tid = MyTid();
  req.name = name;

  int reply;
  Send(nameserver_tid, (char *)&req, sizeof(nameserver_request), (char *)&reply, sizeof(int));

  return reply;
}

int WhoIs(char* name) {
  nameserver_request req;
  req.type = WHO_IS;
  req.name = name;

  int reply;
  Send(nameserver_tid, (char *)&req, sizeof(nameserver_request), (char *)&reply, sizeof(int));

  return reply;
}
