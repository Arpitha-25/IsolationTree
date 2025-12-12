// kswin.h
#ifndef KSWIN_H
#define KSWIN_H

#include <stdbool.h>

typedef struct {
    double *buffer;
    int size;       // current size
    int capacity;   // total window length n
    int r;          // size of "recent" segment
    double alpha;   // significance level (e.g., 0.001)
} KSWIN;

KSWIN *kswin_create(int capacity, int r, double alpha);
void   kswin_destroy(KSWIN *k);
void   kswin_add(KSWIN *k, double value);

// Returns true if KS-distance between old and recent segments is large.
bool   kswin_detect_change(KSWIN *k);

#endif
