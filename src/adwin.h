// adwin.h
#ifndef ADWIN_H
#define ADWIN_H

#include <stdbool.h>

// Simple ADWIN-like detector: fixed-capacity window, mean-difference test.
typedef struct {
    double *buffer;
    int capacity;      // max window size (e.g., 512 or 1024)
    int size;          // current number of elements
    int start;         // circular buffer start index
    double delta;      // sensitivity parameter (e.g., 0.002)
} ADWIN;

ADWIN *adwin_create(int capacity, double delta);
void   adwin_destroy(ADWIN *adw);

// Add one numeric value (score or 0/1 prediction).
void   adwin_add(ADWIN *adw, double value);

// Return true if drift is detected between older and newer halves.
bool   adwin_detect_change(ADWIN *adw);

#endif
