// adwin.c
#include "adwin.h"
#include <stdlib.h>
#include <math.h>

ADWIN *adwin_create(int capacity, double delta) {
    ADWIN *a = (ADWIN*)malloc(sizeof(ADWIN));
    if (!a) return NULL;
    a->buffer   = (double*)malloc(sizeof(double) * capacity);
    if (!a->buffer) { free(a); return NULL; }
    a->capacity = capacity;
    a->size     = 0;
    a->start    = 0;
    a->delta    = delta;
    return a;
}

void adwin_destroy(ADWIN *a) {
    if (!a) return;
    free(a->buffer);
    free(a);
}

static double adwin_get(const ADWIN *a, int idx) {
    int pos = (a->start + idx) % a->capacity;
    return a->buffer[pos];
}

void adwin_add(ADWIN *a, double value) {
    if (a->size < a->capacity) {
        int pos = (a->start + a->size) % a->capacity;
        a->buffer[pos] = value;
        a->size++;
    } else {
        // overwrite oldest
        a->buffer[a->start] = value;
        a->start = (a->start + 1) % a->capacity;
    }
}

bool adwin_detect_change(ADWIN *a) {
    if (a->size < 20) return false; // need some data

    int mid = a->size / 2;
    double sum1 = 0.0, sum2 = 0.0;

    for (int i = 0; i < mid; ++i) {
        sum1 += adwin_get(a, i);
    }
    for (int i = mid; i < a->size; ++i) {
        sum2 += adwin_get(a, i);
    }

    double mean1 = sum1 / mid;
    double mean2 = sum2 / (a->size - mid);
    double diff  = fabs(mean1 - mean2);

    // Very simple threshold: diff > delta ⇒ drift.
    // (Real ADWIN uses Hoeffding bounds; here we approximate.)
    return diff > a->delta;
}
