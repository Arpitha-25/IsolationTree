// kswin.c
#include "kswin.h"
#include <stdlib.h>
#include <math.h>

static int cmp_double(const void *a, const void *b) {
    double da = *(const double*)a;
    double db = *(const double*)b;
    if (da < db) return -1;
    if (da > db) return 1;
    return 0;
}

KSWIN *kswin_create(int capacity, int r, double alpha) {
    KSWIN *k = (KSWIN*)malloc(sizeof(KSWIN));
    if (!k) return NULL;
    k->buffer = (double*)malloc(sizeof(double) * capacity);
    if (!k->buffer) { free(k); return NULL; }
    k->capacity = capacity;
    k->size = 0;
    k->r = r;
    k->alpha = alpha;
    return k;
}

void kswin_destroy(KSWIN *k) {
    if (!k) return;
    free(k->buffer);
    free(k);
}

void kswin_add(KSWIN *k, double value) {
    if (k->size < k->capacity) {
        k->buffer[k->size++] = value;
    } else {
        // shift left (simple, O(n), fine for demo)
        for (int i = 1; i < k->capacity; ++i) {
            k->buffer[i - 1] = k->buffer[i];
        }
        k->buffer[k->capacity - 1] = value;
    }
}

bool kswin_detect_change(KSWIN *k) {
    if (k->size < k->r * 2) return false;

    int n = k->size;
    int r = k->r;

    // Split: newest r values vs previous r values (simple version)
    double *old_vals   = (double*)malloc(sizeof(double) * r);
    double *recent_vals = (double*)malloc(sizeof(double) * r);
    if (!old_vals || !recent_vals) { free(old_vals); free(recent_vals); return false; }

    for (int i = 0; i < r; ++i) {
        old_vals[i]    = k->buffer[n - 2*r + i];
        recent_vals[i] = k->buffer[n - r + i];
    }

    qsort(old_vals, r, sizeof(double), cmp_double);
    qsort(recent_vals, r, sizeof(double), cmp_double);

    // Compute KS statistic: max |F_old(x) - F_recent(x)|
    int i = 0, j = 0;
    double d_max = 0.0;
    while (i < r && j < r) {
        double x = (old_vals[i] < recent_vals[j]) ? old_vals[i] : recent_vals[j];

        while (i < r && old_vals[i] <= x) i++;
        while (j < r && recent_vals[j] <= x) j++;

        double f_old    = (double)i / r;
        double f_recent = (double)j / r;
        double d = fabs(f_old - f_recent);
        if (d > d_max) d_max = d;
    }

    free(old_vals);
    free(recent_vals);

    // Critical value for two-sample KS: ~ c(alpha) * sqrt((2r)/(r^2))
    // Here: d_max > c * sqrt(1.0 / r) ⇒ drift. Choose c from alpha.
    double c = 1.36; // approx for alpha ~ 0.05 (demo)
    double threshold = c * sqrt(1.0 / (double)r);
    return d_max > threshold;
}
