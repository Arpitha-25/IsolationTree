#ifndef STREAM_MANAGER_H
#define STREAM_MANAGER_H

#include "core_ds.h"  // For SlidingWindow, DataPoint, IsolationForest
#include <stdbool.h>  // For bool type

// --- Stream Interface (Simulation) ---

/**
 * @brief Placeholder function to simulate reading the next DataPoint from the stream.
 * In the final implementation, this will read from your file or network connection.
 * @return The next DataPoint object.
 */
DataPoint get_next_point_from_stream();


// --- Sliding Window Management ---

/**
 * @brief Inserts a new DataPoint into the Sliding Window, potentially evicting the oldest point.
 * Implements the circular buffer logic.
 * @param sw The SlidingWindow structure.
 * @param new_point The incoming data point.
 */
void slide_window(SlidingWindow* sw, DataPoint new_point);


// --- IForestASD Logic ---

/**
 * @brief Evaluates the current anomaly rate within the full Sliding Window.
 * Scores every point in the window and counts how many exceed the ANOMALY_THRESHOLD.
 * @param forest The current IsolationForest model.
 * @param sw The current SlidingWindow data.
 * @return The calculated anomaly rate (e.g., 0.05 for 5%).
 */
double evaluate_window_anomaly_rate(IsolationForest* forest, SlidingWindow* sw);

/**
 * @brief The main loop that simulates stream processing, scoring, and drift detection.
 * @param forest The IsolationForest model.
 * @param sw The SlidingWindow structure.
 * @param desired_u The desired anomaly rate (u) for the drift heuristic.
 * @param max_iterations Maximum points to process before stopping (for testing).
 */
void process_stream(IsolationForest* forest, SlidingWindow* sw, double desired_u, int max_iterations);

#endif // STREAM_MANAGER_H