#ifndef CORE_DS_H
#define CORE_DS_H

#include <stdlib.h> // For size_t and NULL
#include <float.h>  // For DBL_MAX, DBL_MIN

// --- Configuration Parameters ---
#define NUM_FEATURES 29 // D: The dimensionality of your data (e.g., 10 features)
#define NUM_TREES 100    // T: The number of Isolation Trees in the Forest
#define WINDOW_SIZE 256  // W: The size of the Sliding Window
#define SAMPLE_SIZE 256  // psi (ψ): The number of points sampled for each tree (often W)

// The anomaly score threshold for the basic IForestASD heuristic
// Points with score > ANOMALY_THRESHOLD are considered anomalies (e.g., 0.6)
#define ANOMALY_THRESHOLD 0.6 
// The desired anomaly rate (u) for the basic drift detection heuristic (e.g., 5%)
#define DESIRED_ANOMALY_RATE_U 0.05 

// --- Core Data Structure Definitions ---

/**
 * @brief Represents a single data point in the stream.
 */
typedef struct {
    double features[NUM_FEATURES];
} DataPoint;

/**
 * @brief Represents a node in an Isolation Tree (iTree).
 */
typedef struct Node {
    int is_external;          // 1 if leaf node, 0 if internal node
    int size;                 // Number of data points that reached this node (for leaves)
    int height;               // Depth of the node (0 for root)

    // Split information (used only for internal nodes)
    int split_feature_index;  // The feature dimension used for the split (d)
    double split_value;       // The random value used for the split (v)

    struct Node* left;        // Pointer to the left child
    struct Node* right;       // Pointer to the right child
} Node;

/**
 * @brief Represents the entire Isolation Forest (collection of iTrees).
 */
typedef struct {
    Node* trees[NUM_TREES];
} IsolationForest;

/**
 * @brief Represents the Sliding Window, storing the most recent W data points.
 */
typedef struct {
    DataPoint buffer[WINDOW_SIZE];
    int current_size;  // Current number of points in the window (<= WINDOW_SIZE)
    int head;          // Index of the oldest element (where the next one will be evicted from)
    int tail;          // Index of the newest element (where the next one will be inserted)
} SlidingWindow;


// --- Function Prototypes for Memory Management (core_ds.c) ---

// Node Management
Node* create_node(int is_external, int size, int height);
void free_tree(Node* root);

// Forest Management
IsolationForest* create_forest();
void free_forest(IsolationForest* forest);

// Window Management
SlidingWindow* create_sliding_window();
void destroy_sliding_window(SlidingWindow* sw);


#endif // CORE_DS_H