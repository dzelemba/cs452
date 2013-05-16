
void pass() {
  asm("swi 2");
}

void create(int priority, void (*code)) {
  asm("swi 1");
}


