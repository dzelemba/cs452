#ifndef __PRIORITIES_H__
#define __PRIORITIES_H__

#define MAX_PRI     0

#define HI_PRI_K    1
#define HI_PRI_1    2
#define HI_PRI      3

#define MED_PRI_K   4
#define MED_PRI_1   5
#define MED_PRI     6

#define LOW_PRI_K   7
#define LOW_PRI_1   8
#define LOW_PRI     9

#define VLOW_PRI_K  10
#define VLOW_PRI_1  11
#define VLOW_PRI    12

#define MIN_PRI     13

#define NUM_PRIORITY_TYPES (14)

int is_kernel_priority(int priority);

#endif
