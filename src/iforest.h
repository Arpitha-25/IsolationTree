#ifndef IFOREST_H
#define IFOREST_H

#include "core_ds.h" // Includes DataPoint, Node, IsolationForest structs

// --- IForest Core Functions ---

/**
 * @brief Recursively builds a single Isolation Tree (iTree).
 * * @param data Array of DataPoints used for training this node.
 * @param count Number of DataPoints in the data array.
 * @param height The current depth of the node (0 for root).
 * @param max_depth The maximum path length for this tree (ceil(log2(sample_size))).
 * @return The root Node of the built iTree.
 */
Node* build_iTree(DataPoint* data, int count, int height, int max_depth);

/**
 * @brief Trains the entire Isolation Forest by building NUM_TREES iTrees.
 * * Note: This function will typically handle the random sampling (ψ) of the window data 
 * before calling build_iTree for each tree.
 * * @param forest Pointer to the IsolationForest structure to populate.
 * @param window_data All data points currently in the Sliding Window.
 * @param window_size The total number of points in the window (W).
 */
void train_iforest(IsolationForest* forest, DataPoint* window_data, int window_size);


// --- Scoring Functions ---

/**
 * @brief Traverses a single iTree to find the path length (depth) for a given point.
 * * @param root The root node of the iTree.
 * @param x The DataPoint to score.
 * @param current_path_len Initial path length (usually 0).
 * @return The path length h(x) for point x in this tree.
 */
double get_path_length(Node* root, DataPoint x, double current_path_len);

/**
 * @brief Calculates the normalization constant c(n) for a given sample size n.
 * * c(n) = 2*H(n-1) - 2*(n-1)/n, where H(i) is the i-th Harmonic Number.
 * * @param n The sample size (ψ).
 * @return The normalization constant.
 */
double average_path_length_constant(int n);

/**
 * @brief Computes the final anomaly score s(x) for a point x across the entire Forest.
 * * s(x) = 2 ^ (-E[h(x)] / c(n))
 * * @param forest The trained IsolationForest.
 * @param x The DataPoint to score.
 * @param sample_size The sample size (ψ) used to train the trees.
 * @return The final anomaly score s(x), ranging from 0 to 1.
 */
double calculate_score(IsolationForest* forest, DataPoint x, int sample_size);

#endif // IFOREST_H