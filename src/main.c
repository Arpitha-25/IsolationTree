#include <stdio.h>
#include "core_ds.h"
#include "iforest.h"
#include "stream_manager.h"
#include "utils.h"

// External declarations for stream file handling (defined in stream_manager.c)
extern bool open_stream(const char* filename);
extern void close_stream();

/**
 * @brief The entry point of the IForestASD streaming anomaly detection project.
 */
int main(int argc, char *argv[]) {
    
    // --- 1. Initialization and Setup ---
    
    // Initialize the Random Number Generator (Crucial for IForest randomness)
    initialize_rng();
    
    // Check command line arguments for data file
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path_to_stream_data_file>\n", argv[0]);
        // Note: The data file must be formatted to match the reading logic in get_next_point_from_stream()
        return 1;
    }
    const char* data_filename = argv[1];

    // Open the simulated data stream file
    if (!open_stream(data_filename)) {
        return 1; // Error already printed inside open_stream
    }
    
    // --- 2. Data Structure Allocation ---

    IsolationForest* forest = create_forest();
    SlidingWindow* sw = create_sliding_window();

    if (forest == NULL || sw == NULL) {
        fprintf(stderr, "Fatal error: Failed to allocate core data structures.\n");
        // Clean up any successfully allocated structures before exiting
        if (forest) free_forest(forest);
        if (sw) destroy_sliding_window(sw);
        close_stream();
        return 1;
    }

    // --- 3. Configuration Display ---
    printf("==================================================\n");
    printf("   Isolation Forest Anomaly Detection (IForestASD)\n");
    printf("==================================================\n");
    printf("Configuration:\n");
    printf("  Features (D): %d\n", NUM_FEATURES);
    printf("  Trees (T): %d\n", NUM_TREES);
    printf("  Window Size (W): %d\n", WINDOW_SIZE);
    printf("  Sample Size (ψ): %d\n", SAMPLE_SIZE);
    printf("  Anomaly Score Threshold: %.2f\n", ANOMALY_THRESHOLD);
    printf("  Drift Threshold (u): %.2f\n", DESIRED_ANOMALY_RATE_U);
    printf("  Processing Stream: %s\n", data_filename);
    printf("--------------------------------------------------\n");

    // --- 4. Main Processing Loop ---
    
    // Start the continuous stream processing loop
    // Set a high iteration limit (INT_MAX is good for C, but we use a large number 
    // here to allow the stream logic to handle EOF naturally).
    const int MAX_POINTS_TO_PROCESS = 100000; 

    process_stream(forest, sw, DESIRED_ANOMALY_RATE_U, MAX_POINTS_TO_PROCESS);

    // --- 5. Cleanup ---

    printf("\nStream processing finished. Performing cleanup...\n");
    
    // Free all dynamically allocated memory
    free_forest(forest);
    destroy_sliding_window(sw);
    
    // close_stream() is called inside process_stream() upon EOF, but can be called here 
    // again to ensure closure if the loop terminates early.
    close_stream(); 

    printf("Cleanup complete. Program exit.\n");
    return 0;
}