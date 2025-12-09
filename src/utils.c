#include "utils.h"
#include <stdlib.h> // For rand(), srand(), RAND_MAX
#include <time.h>   // For time()
#include <string.h> // For memcpy

// --- Randomization Implementation ---

/**
 * @brief Initializes the random number generator using the current time.
 */
void initialize_rng() {
    // Seed the random number generator using the current time
    srand((unsigned int)time(NULL));
}

/**
 * @brief Generates a random integer within a specified range [min, max], inclusive.
 */
int get_random_integer(int min, int max) {
    if (min > max) {
        // Handle invalid range gracefully
        return min; 
    }
    // Formula: min + rand() % (max - min + 1)
    return min + (rand() % (max - min + 1));
}

/**
 * @brief Generates a random double-precision floating point number 
 * uniformly distributed within the specified range [min, max].
 */
double get_random_uniform(double min, double max) {
    if (min >= max) {
        return min; // Return min if range is invalid or zero
    }
    // rand() / (double)RAND_MAX generates a value in [0.0, 1.0]
    double normalized_rand = (double)rand() / (double)RAND_MAX;
    
    // Formula: min + normalized_rand * (max - min)
    return min + normalized_rand * (max - min);
}


// --- Sampling Implementation ---

/**
 * @brief Randomly samples a specified number of data points using the 
 * "sampling without replacement" technique.
 * Note: If window_size < sample_size, it will just copy all available data.
 */
void sample_data_stream(DataPoint* window_data, int window_size, DataPoint* sample_data, int sample_size) {
    if (window_size <= 0 || sample_size <= 0) {
        return;
    }

    // Determine the actual number of points to sample
    int count_to_sample = (window_size < sample_size) ? window_size : sample_size;

    // Use Reservoir Sampling or simple index mapping for efficiency.
    // Since sample_size (ψ) is usually close to window_size (W), 
    // we'll use a simple approach by generating random indices.
    
    // To ensure sampling without replacement (each point chosen only once),
    // we use a boolean array to track used indices, but for large W and small ψ,
    // simple repeated sampling with replacement might be easier and faster in C.
    // However, IForest usually implies sampling without replacement.
    
    // Simple approach: Reservoir Sampling (can be complex) or just
    // using an array to track if an index was picked.

    // Using an array of indices for sampling without replacement (the preferred way for IForest):
    int available_indices[window_size];
    for (int i = 0; i < window_size; i++) {
        available_indices[i] = i; // Initialize with 0, 1, 2, ..., W-1
    }

    // Fisher-Yates shuffle variant to select the first 'count_to_sample' elements
    for (int i = 0; i < count_to_sample; i++) {
        // Choose a random index 'j' from the remaining indices [i, window_size - 1]
        int j = get_random_integer(i, window_size - 1);
        
        // Copy the chosen data point to the sample array
        // available_indices[i] now holds the index in window_data
        sample_data[i] = window_data[available_indices[j]]; 

        // Swap the chosen index (j) with the current index (i) to exclude it from future picks
        int temp = available_indices[i];
        available_indices[i] = available_indices[j];
        available_indices[j] = temp;
    }

    // If actual sampled count is less than target sample_size (e.g., if W < ψ), 
    // the remaining spots in sample_data are left undefined or zeroed out (not strictly necessary 
    // since build_iTree uses the actual count from train_iforest, but safe practice).
}