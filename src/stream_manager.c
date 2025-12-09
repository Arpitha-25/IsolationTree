#include "stream_manager.h"
#include "core_ds.h"
#include "iforest.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#define MAX_LINE_BUFFER 4096

static FILE* stream_file = NULL;
static bool header_skipped = false;

bool open_stream(const char* filename) {
    stream_file = fopen(filename, "r");  // TEXT MODE, NOT "rb"
    if (stream_file == NULL) {
        perror("Error opening data stream file");
        return false;
    }
    header_skipped = false;
    return true;
}

void close_stream() {
    if (stream_file != NULL) {
        fclose(stream_file);
        stream_file = NULL;
    }
}

DataPoint get_next_point_from_stream() {
    DataPoint point;
    char line_buffer[MAX_LINE_BUFFER];
    point.features[0] = NAN;
    
    if (stream_file == NULL) {
        fprintf(stderr, "Error: Stream file not open.\n");
        return point;
    }
    
    // SKIP HEADER ON FIRST CALL
    if (!header_skipped) {
        if (fgets(line_buffer, MAX_LINE_BUFFER, stream_file) == NULL) {
            fprintf(stderr, "Error: Could not read header line.\n");
            return point;
        }
        fprintf(stderr, "Header skipped: %s", line_buffer);
        header_skipped = true;
    }
    
    // READ DATA LINE
    if (fgets(line_buffer, MAX_LINE_BUFFER, stream_file) == NULL) {
        point.features[0] = NAN;
        return point;
    }
    
    // REPLACE COMMAS WITH SPACES
    for (int i = 0; line_buffer[i] != '\0'; i++) {
        if (line_buffer[i] == ',') {
            line_buffer[i] = ' ';
        }
    }
    
    // PARSE 29 VALUES
    int success_count = sscanf(line_buffer,
        "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf"
        "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf"
        "%lf %lf %lf %lf %lf %lf %lf %lf %lf",
        &point.features[0], &point.features[1], &point.features[2], &point.features[3], &point.features[4],
        &point.features[5], &point.features[6], &point.features[7], &point.features[8], &point.features[9],
        &point.features[10], &point.features[11], &point.features[12], &point.features[13], &point.features[14],
        &point.features[15], &point.features[16], &point.features[17], &point.features[18], &point.features[19],
        &point.features[20], &point.features[21], &point.features[22], &point.features[23], &point.features[24],
        &point.features[25], &point.features[26], &point.features[27], &point.features[28]
    );
    
    if (success_count != NUM_FEATURES) {
        fprintf(stderr, "Parse Error: Expected %d features, got %d.\n", NUM_FEATURES, success_count);
        point.features[0] = NAN;
        return point;
    }
    
    return point;
}

void slide_window(SlidingWindow* sw, DataPoint new_point) {
    sw->buffer[sw->tail] = new_point;
    sw->tail = (sw->tail + 1) % WINDOW_SIZE;
    if (sw->current_size < WINDOW_SIZE) {
        sw->current_size++;
    } else {
        sw->head = sw->tail;
    }
}

double evaluate_window_anomaly_rate(IsolationForest* forest, SlidingWindow* sw) {
    if (sw->current_size < WINDOW_SIZE) return 0.0;
    int anomaly_count = 0;
    for (int i = 0; i < WINDOW_SIZE; i++) {
        double score = calculate_score(forest, sw->buffer[i], SAMPLE_SIZE);
        if (score >= ANOMALY_THRESHOLD) {
            anomaly_count++;
        }
    }
    return (double)anomaly_count / (double)WINDOW_SIZE;
}

void process_stream(IsolationForest* forest, SlidingWindow* sw, double desired_u, int max_iterations) {
    int iteration = 0;
    int points_processed = 0;
    
    printf("--- Waiting to fill initial window (W=%d) for first training ---\n", WINDOW_SIZE);
    
    while (sw->current_size < WINDOW_SIZE && iteration < max_iterations) {
        DataPoint new_point = get_next_point_from_stream();
        if (isnan(new_point.features[0])) {
            iteration++;
            continue;
        }
        slide_window(sw, new_point);
        points_processed++;
        iteration++;
    }
    
    if (sw->current_size == WINDOW_SIZE) {
        printf("Window filled with %d points. Initial IForest training...\n", points_processed);
        train_iforest(forest, sw->buffer, WINDOW_SIZE);
    } else {
        printf("Stream ended before window filled (%d/%d).\n", sw->current_size, WINDOW_SIZE);
        close_stream();
        return;
    }
    
    printf("--- Starting Stream Processing ---\n");
    
    while (iteration < max_iterations) {
        DataPoint new_point = get_next_point_from_stream();
        if (isnan(new_point.features[0])) {
            if (points_processed > WINDOW_SIZE) {
                printf("End of stream reached.\n");
                break;
            }
            iteration++;
            continue;
        }
        
        slide_window(sw, new_point);
        double score = calculate_score(forest, new_point, SAMPLE_SIZE);
        printf("Point %d: Score=%.4f (%s)\n", points_processed, score, 
               (score >= ANOMALY_THRESHOLD) ? "ANOMALY" : "Normal");
        
        double rate = evaluate_window_anomaly_rate(forest, sw);
        if (rate > desired_u) {
            printf(">>> DRIFT DETECTED (%.4f > %.4f). Retraining...\n", rate, desired_u);
            train_iforest(forest, sw->buffer, WINDOW_SIZE);
        }
        
        points_processed++;
        iteration++;
    }
    
    close_stream();
    printf("Total points processed: %d\n", points_processed);
}
