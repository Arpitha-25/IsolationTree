#include "core_ds.h"
#include <stdio.h>

// --- Node Management ---

/**
 * @brief Allocates memory for a new Node and initializes its basic properties.
 * * @param is_external 1 for leaf node, 0 for internal node.
 * @param size The number of points contained in the node.
 * @param height The depth of the node (path length from the root).
 * @return A pointer to the newly created Node, or NULL on failure.
 */
Node* create_node(int is_external, int size, int height) {
    // Allocate memory for the Node structure
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (new_node == NULL) {
        perror("Error: Memory allocation failed for new Node");
        return NULL;
    }

    // Initialize common properties
    new_node->is_external = is_external;
    new_node->size = size;
    new_node->height = height;
    new_node->left = NULL;
    new_node->right = NULL;

    // Initialize split properties (important even for external nodes, though unused)
    new_node->split_feature_index = -1;
    new_node->split_value = DBL_MIN;

    return new_node;
}

/**
 * @brief Recursively frees all memory allocated for an Isolation Tree.
 * * @param root The root node of the tree to be freed.
 */
void free_tree(Node* root) {
    if (root == NULL) {
        return;
    }
    // Post-order traversal to ensure children are freed before parent
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

// --- Forest Management ---

/**
 * @brief Allocates memory for the IsolationForest structure.
 * * @return A pointer to the newly created IsolationForest, or NULL on failure.
 */
IsolationForest* create_forest() {
    IsolationForest* forest = (IsolationForest*)malloc(sizeof(IsolationForest));
    if (forest == NULL) {
        perror("Error: Memory allocation failed for IsolationForest");
        return NULL;
    }
    // Initialize all tree pointers to NULL
    for (int i = 0; i < NUM_TREES; i++) {
        forest->trees[i] = NULL;
    }
    return forest;
}

/**
 * @brief Frees all memory allocated for the IsolationForest, including all its trees.
 * * @param forest The IsolationForest structure to be freed.
 */
void free_forest(IsolationForest* forest) {
    if (forest == NULL) {
        return;
    }
    // Free each tree in the forest
    for (int i = 0; i < NUM_TREES; i++) {
        if (forest->trees[i] != NULL) {
            free_tree(forest->trees[i]);
            forest->trees[i] = NULL; // Prevent double freeing
        }
    }
    free(forest);
}

// --- Sliding Window Management ---

/**
 * @brief Allocates memory for the SlidingWindow structure and initializes pointers.
 * * @return A pointer to the newly created SlidingWindow.
 */
SlidingWindow* create_sliding_window() {
    SlidingWindow* sw = (SlidingWindow*)malloc(sizeof(SlidingWindow));
    if (sw == NULL) {
        perror("Error: Memory allocation failed for SlidingWindow");
        return NULL;
    }
    // Initialize the window as empty
    sw->current_size = 0;
    sw->head = 0;
    sw->tail = 0;
    
    // Note: The buffer (DataPoint buffer[WINDOW_SIZE]) is statically allocated 
    // within the struct, so no separate malloc is needed for the buffer itself.
    
    return sw;
}

/**
 * @brief Frees the memory allocated for the SlidingWindow structure.
 * * @param sw The SlidingWindow structure to be destroyed.
 */
void destroy_sliding_window(SlidingWindow* sw) {
    if (sw != NULL) {
        free(sw);
    }
}