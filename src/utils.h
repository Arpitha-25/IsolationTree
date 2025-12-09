#ifndef UTILS_H
#define UTILS_H

#include "core_ds.h" // Needed for DataPoint structure

// --- Randomization Functions ---

/**
 * @brief Initializes the random number generator using the current time.
 */
void initialize_rng();

/**
 * @brief Generates a random integer within a specified range [min, max], inclusive.
 * @param min The minimum integer value.
 * @param max The maximum integer value.
 * @return A random integer.
 */
int get_random_integer(int min, int max);

/**
 * @brief Generates a random double-precision floating point number 
 * uniformly distributed within the specified range [min, max].
 * @param min The minimum double value.
 * @param max The maximum double value.
 * @return A random double.
 */
double get_random_uniform(double min, double max);


// --- Sampling Functions ---

/**
 * @brief Randomly samples a specified number of data points (sample_size) 
 * from a larger dataset (window_data). This is used to select the ψ points 
 * for building each iTree.
 * @param window_data The source array of data points (size W).
 * @param window_size The current size of the source data (W).
 * @param sample_data The destination array to store the sampled points (size ψ).
 * @param sample_size The number of points to sample (ψ).
 */
void sample_data_stream(DataPoint* window_data, int window_size, DataPoint* sample_data, int sample_size);

#endif // UTILS_H