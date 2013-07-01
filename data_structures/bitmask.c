#include "bitmask.h"

void bm_create(bitmask* bm) {
  bm->bmask = 0;
}

void bm_set(bitmask* bm, int bit) {
  bm->bmask |= (1 << (bit - 1));
}

void bm_unset(bitmask* bm, int bit) {
  bm->bmask &= ~(1 << (bit - 1));
}

int bm_isset(bitmask* bm, int bit) {
  return bm->bmask & (1 << (bit -1));
}

/*
 * Note these were grabbed from ffs and fls from the linux
 * kernel source.
 */

int bm_getlowbit(bitmask* bm) {
  int x = bm->bmask;

  int r = 1;

  if (!x)
          return 0;
  if (!(x & 0xffff)) {
          x >>= 16;
          r += 16;
  }
  if (!(x & 0xff)) {
          x >>= 8;
          r += 8;
  }
  if (!(x & 0xf)) {
          x >>= 4;
          r += 4;
  }
  if (!(x & 3)) {
          x >>= 2;
          r += 2;
  }
  if (!(x & 1)) {
          x >>= 1;
          r += 1;
  }
  return r;
}

int bm_gethibit(bitmask* bm) {
  int x = bm->bmask;

  int r = 32;

  if (!x)
          return 0;
  if (!(x & 0xffff0000u)) {
          x <<= 16;
          r -= 16;
  }
  if (!(x & 0xff000000u)) {
          x <<= 8;
          r -= 8;
  }
  if (!(x & 0xf0000000u)) {
          x <<= 4;
          r -= 4;
  }
  if (!(x & 0xc0000000u)) {
          x <<= 2;
          r -= 2;
  }
  if (!(x & 0x80000000u)) {
          x <<= 1;
          r -= 1;
  }
  return r;
}
