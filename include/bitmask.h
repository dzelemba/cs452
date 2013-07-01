#ifndef __BITMASK_H__
#define __BITMASK_H__

typedef struct bitmask {
  int bmask;
} bitmask;

/*
 * Note: 1 is bit 1.
 */

void bm_create(bitmask* bm);

void bm_set(bitmask* bm, int bit);

void bm_unset(bitmask* bm, int bit);

int bm_isset(bitmask* bm, int bit);

/*
 * Both return 0 if empty.
 */
int bm_getlowbit(bitmask* bm);
int bm_gethibit(bitmask* bm);

#endif
