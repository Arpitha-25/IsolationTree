#include "iforest.h"
#include "core_ds.h"
#include "utils.h" // Assumed to contain get_random_integer, get_random_uniform, etc.

#include <stdio.h>
#include <math.h>
#include <limits.h>

// Helper function prototype (used internally for recursion)
static void find_min_max(DataPoint* data, int count, int feature_index, double* min_val, double* max_val);
static void partition_data(DataPoint* data, int count, int feature_index, double split_value, DataPoint* left_set, int* left_count, DataPoint* right_set, int* right_count);

// --- IForest Core Implementation ---

/**
 * @brief Recursively builds a single Isolation Tree (iTree).
 * * This is the heart of the training process.
 */
Node* build_iTree(DataPoint* data, int count, int height, int max_depth) {
    // 1. Check Base Cases (Stop Conditions)
    if (count <= 1 || height >= max_depth) {
        // Stop if isolated (count=1) or max depth reached
        return create_node(1, count, height); // External (Leaf) Node
    }

    // 2. Prepare Internal Node
    Node* node = create_node(0, count, height); // Internal Node
    if (node == NULL) {
        return NULL; // Allocation error
    }

    // 3. Choose Random Split
    
    // a) Choose a random feature (dimension) d
    int feature_index = get_random_integer(0, NUM_FEATURES - 1);
    node->split_feature_index = feature_index;

    // b) Find min/max values in the current subset for that feature
    double min_val, max_val;
    find_min_max(data, count, feature_index, &min_val, &max_val);

    if (min_val == max_val) {
        // If all values are the same, isolation is complete (treat as a leaf)
        free(node);
        return create_node(1, count, height);
    }
    
    // c) Choose a random split value v between min_val and max_val
    double split_value = get_random_uniform(min_val, max_val);
    node->split_value = split_value;

    // 4. Partition Data and Recurse
    
    // Temporary storage for partitioned subsets (must be large enough)
    DataPoint left_set[count], right_set[count];
    int left_count = 0, right_count = 0;

    // Partition the data based on the split condition
    partition_data(data, count, feature_index, split_value, left_set, &left_count, right_set, &right_count);
    
    // Recursively build children
    node->left = build_iTree(left_set, left_count, height + 1, max_depth);
    node->right = build_iTree(right_set, right_count, height + 1, max_depth);

    return node;
}

/**
 * @brief Trains the entire Isolation Forest (T trees).
 */
void train_iforest(IsolationForest* forest, DataPoint* window_data, int window_size) {
    if (forest == NULL || window_size == 0) return;

    // Calculate maximum depth for the iTrees: ceil(log2(SAMPLE_SIZE))
    // We use log2((double)SAMPLE_SIZE) for accurate calculation
    int max_depth = (int)ceil(log2((double)SAMPLE_SIZE)); 
    if (max_depth == 0) max_depth = 1;

    for (int i = 0; i < NUM_TREES; i++) {
        // 1. Sample Data (ψ points)
        // This array will hold the sampled data for the current tree
        DataPoint sample_data[SAMPLE_SIZE]; 
        
        // This utility function is crucial: it randomly selects SAMPLE_SIZE points 
        // from window_data (size W) and stores them in sample_data.
        sample_data_stream(window_data, window_size, sample_data, SAMPLE_SIZE); 

        // 2. Build the iTree
        // Free old tree if retraining (important for concept drift)
        if (forest->trees[i] != NULL) {
            free_tree(forest->trees[i]);
            forest->trees[i] = NULL;
        }

        forest->trees[i] = build_iTree(sample_data, SAMPLE_SIZE, 0, max_depth);
    }
}


// --- Scoring Implementation ---

/**
 * @brief Traverses a single iTree to find the path length (depth) for a given point.
 */
double get_path_length(Node* root, DataPoint x, double current_path_len) {
    if (root == NULL) {
        // Should not happen if data is processed correctly, but safety check.
        return current_path_len; 
    }

    if (root->is_external) {
        // If external (leaf) node, adjust for the case where the leaf contains >1 point.
        // We add the correction factor C(size) to the path length.
        return current_path_len + average_path_length_constant(root->size);
    }

    // Internal node: check split condition
    int feature_index = root->split_feature_index;

    if (x.features[feature_index] <= root->split_value) {
        return get_path_length(root->left, x, current_path_len + 1);
    } else {
        return get_path_length(root->right, x, current_path_len + 1);
    }
}

/**
 * @brief Calculates the normalization constant c(n) using the Harmonic Number approximation.
 * The formula is c(n) = 2*H(n-1) - 2*(n-1)/n
 * The Harmonic number H(i) is approximated by ln(i) + 0.5772156649 (Euler-Mascheroni constant).
 */
double average_path_length_constant(int n) {
    if (n <= 1) {
        return 0.0;
    }
    double h_n_minus_1;
    
    // Check if n-1 is very large (use approximation) or small (use direct value or approximation)
    // For n>2, approximation is generally sufficient for IF.
    if (n > 2) {
        // Approximation using Euler-Mascheroni constant (gamma)
        const double EULER_MASCHERONI = 0.5772156649;
        h_n_minus_1 = log((double)n - 1.0) + EULER_MASCHERONI;
    } else {
        // For n=2, H(1) = 1
        h_n_minus_1 = 1.0; 
    }

    return 2.0 * h_n_minus_1 - (2.0 * (n - 1.0) / (double)n);
}

/**
 * @brief Computes the final anomaly score s(x) for a point x across the entire Forest.
 */
double calculate_score(IsolationForest* forest, DataPoint x, int sample_size) {
    if (forest == NULL || sample_size <= 0) return 0.0;
    
    double total_path_length = 0.0;

    // 1. Calculate E[h(x)] - Average Path Length
    for (int i = 0; i < NUM_TREES; i++) {
        if (forest->trees[i] != NULL) {
            total_path_length += get_path_length(forest->trees[i], x, 0.0);
        }
    }
    double avg_path_length = total_path_length / (double)NUM_TREES;

    // 2. Calculate Normalization Constant c(n)
    double c_n = average_path_length_constant(sample_size);
    if (c_n == 0.0) {
        // Avoid division by zero, return score of normal point
        return 0.5; 
    }

    // 3. Compute Final Score s(x) = 2 ^ (-E[h(x)] / c(n))
    // We use the pow() function from <math.h>
    double exponent = -(avg_path_length / c_n);
    double score = pow(2.0, exponent); // 

    return score;
}

// --- Internal Helper Functions (Static) ---

/**
 * Finds the minimum and maximum values for a specified feature in the data subset.
 */
static void find_min_max(DataPoint* data, int count, int feature_index, double* min_val, double* max_val) {
    *min_val = DBL_MAX;
    *max_val = DBL_MIN;

    for (int i = 0; i < count; i++) {
        double val = data[i].features[feature_index];
        if (val < *min_val) *min_val = val;
        if (val > *max_val) *max_val = val;
    }
}

/**
 * Partitions the data into left (<= split_value) and right (> split_value) subsets.
 */
static void partition_data(DataPoint* data, int count, int feature_index, double split_value, 
                           DataPoint* left_set, int* left_count, 
                           DataPoint* right_set, int* right_count) {
    *left_count = 0;
    *right_count = 0;

    for (int i = 0; i < count; i++) {
        if (data[i].features[feature_index] <= split_value) {
            left_set[(*left_count)++] = data[i];
        } else {
            right_set[(*right_count)++] = data[i];
        }
    }
}