#include "gsplit.h"

static void calculate_partition_sizes(t_graph *graph, int num_parts, size_t *sizes) {
    for (int i = 0; i < num_parts; i++) sizes[i] = 0;
    for (size_t i = 0; i < graph->nodes_num; i++) sizes[graph->nodes[i].partition]++;
}

static int compute_gain(t_graph *graph, size_t node_idx, int new_part) {
    int gain = 0;
    int current_part = graph->nodes[node_idx].partition;

    for (size_t i = 0; i < graph->nodes[node_idx].connections_num; i++) {
        t_node *neighbor = graph->nodes[node_idx].connections[i];
        if (neighbor->partition == current_part) gain--;
        else if (neighbor->partition == new_part) gain++;
    }
    return gain;
}

static void optimize_partitions(t_graph *graph, int num_parts, int margin, size_t *sizes) {
    bool improved;
    int iterations = 0;
    size_t ideal = graph->nodes_num / num_parts;
    int allowed_diff = ideal * margin / 100;

    do {
        improved = false;
        t_move best_move = {0, INT_MIN, -1};
        calculate_partition_sizes(graph, num_parts, sizes);

        for (size_t i = 0; i < graph->nodes_num; i++) {
            int current_part = graph->nodes[i].partition;

            for (int p = 0; p < num_parts; p++) {
                if (p == current_part) continue;

                bool has_neighbor_in_p = false;
                for (size_t j = 0; j < graph->nodes[i].connections_num; j++) {
                    if (graph->nodes[i].connections[j]->partition == p) {
                        has_neighbor_in_p = true;
                        break;
                    }
                }
                if (!has_neighbor_in_p) continue;

                if (sizes[p] >= ideal + allowed_diff) continue;
                if (sizes[current_part] <= ideal - allowed_diff) continue;

                int gain = compute_gain(graph, i, p);
                int balance_bonus = 0;
                if (sizes[p] < ideal) balance_bonus += 2;
                if (sizes[current_part] > ideal) balance_bonus += 2;

                if ((gain + balance_bonus) > best_move.gain) {
                    best_move.node_index = i;
                    best_move.gain = gain + balance_bonus;
                    best_move.target_part = p;
                }
            }
        }

        if (best_move.target_part != -1) {
            int old_part = graph->nodes[best_move.node_index].partition;
            graph->nodes[best_move.node_index].partition = best_move.target_part;
            sizes[old_part]--;
            sizes[best_move.target_part]++;
            improved = true;
        }

        iterations++;
    } while (iterations < OPTIMIZE_MAX_ITER);
}

static void fix_disconnected_partitions(t_graph *graph, int num_parts) {
    size_t ideal = graph->nodes_num / num_parts;
    int allowed_diff = ideal * 10 / 100;
    size_t *sizes = calloc(num_parts, sizeof(size_t));
    if (!sizes) return;
    calculate_partition_sizes(graph, num_parts, sizes);

    bool changes_made;
    do {
        changes_made = false;

        for (int p = 0; p < num_parts; p++) {
            for (size_t i = 0; i < graph->nodes_num; i++) {
                if (graph->nodes[i].partition != p)
                    continue;

                bool has_local_connection = false;
                for (size_t j = 0; j < graph->nodes[i].connections_num; j++) {
                    if (graph->nodes[i].connections[j]->partition == p) {
                        has_local_connection = true;
                        break;
                    }
                }

                if (!has_local_connection && graph->nodes[i].connections_num > 0) {
                    int best_partition = -1;
                    int most_connections = 0;

                    int *partition_counts = calloc(num_parts, sizeof(int));
                    if (!partition_counts) continue;

                    for (size_t j = 0; j < graph->nodes[i].connections_num; j++) {
                        int neighbor_part = graph->nodes[i].connections[j]->partition;
                        partition_counts[neighbor_part]++;
                        if (partition_counts[neighbor_part] > most_connections && sizes[neighbor_part] < ideal + allowed_diff) {
                            most_connections = partition_counts[neighbor_part];
                            best_partition = neighbor_part;
                        }
                    }

                    free(partition_counts);

                    if (best_partition != -1) {
                        int old_part = graph->nodes[i].partition;
                        graph->nodes[i].partition = best_partition;
                        sizes[old_part]--;
                        sizes[best_partition]++;
                        changes_made = true;
                    }
                }
            }
        }
    } while (changes_made);

    free(sizes);
}

static void remove_cross_partition_connections(t_graph *graph) {
    for (size_t i = 0; i < graph->nodes_num; i++) {
        t_node *node = &graph->nodes[i];
        int current_part = node->partition;

        size_t keep_count = 0;
        for (size_t j = 0; j < node->connections_num; j++) {
            if (node->connections[j]->partition == current_part) {
                keep_count++;
            }
        }

        if (keep_count == 0 || keep_count == node->connections_num) continue;

        t_node **new_connections = (t_node **)calloc(keep_count, sizeof(t_node *));
        if (!new_connections) continue;

        size_t new_idx = 0;
        for (size_t j = 0; j < node->connections_num; j++) {
            if (node->connections[j]->partition == current_part) {
                new_connections[new_idx++] = node->connections[j];
            }
        }

        free(node->connections);
        node->connections = new_connections;
        node->connections_num = keep_count;
    }
}

static bool partitions_are_balanced(size_t *sizes, int num_parts, size_t ideal, int margin) {
    int allowed_diff = ideal * margin / 100;
    for (int i = 0; i < num_parts; i++) {
        if (sizes[i] < ideal - allowed_diff || sizes[i] > ideal + allowed_diff) {
            return false;
        }
    }
    return true;
}

static void balance_partitions(t_graph *graph, int num_parts, int margin, size_t *sizes) {
    calculate_partition_sizes(graph, num_parts, sizes);
    size_t ideal = graph->nodes_num / num_parts;
    int allowed_diff = ideal * margin / 100;

    bool balanced = false;
    while (!balanced) {
        balanced = true;

        for (int i = 0; i < num_parts; i++) {
            if (sizes[i] > ideal + allowed_diff) {
                for (int j = 0; j < num_parts; j++) {
                    if (sizes[j] < ideal - allowed_diff) {
                        for (size_t n = 0; n < graph->nodes_num; n++) {
                            if (graph->nodes[n].partition != i)
                                continue;

                            bool has_connection_to_j = false;
                            for (size_t c = 0; c < graph->nodes[n].connections_num; c++) {
                                if (graph->nodes[n].connections[c]->partition == j) {
                                    has_connection_to_j = true;
                                    break;
                                }
                            }

                            if (has_connection_to_j) {
                                graph->nodes[n].partition = j;
                                sizes[i]--;
                                sizes[j]++;
                                balanced = false;
                                break;
                            }
                        }
                        if (!balanced) break;
                    }
                }
            }
        }
    }
}

void initialize_partitions(t_graph *graph, int num_parts) {
    size_t part_size = graph->nodes_num / num_parts;
    size_t remainder = graph->nodes_num % num_parts;

    size_t current_part = 0;
    size_t count_in_part = 0;

    for (size_t i = 0; i < graph->nodes_num; i++) {
        graph->nodes[i].partition = current_part;
        count_in_part++;

        size_t target_size = part_size + (current_part < remainder ? 1 : 0);
        if (count_in_part >= target_size && current_part < (size_t)(num_parts - 1)) {
            current_part++;
            count_in_part = 0;
        }
    }
}

bool partition_graph(t_graph *graph, int num_parts, int margin) {
    size_t *sizes = calloc(num_parts, sizeof(size_t));
    if (!sizes) return false;

    size_t ideal = graph->nodes_num / num_parts;

    initialize_partitions(graph, num_parts);
    optimize_partitions(graph, num_parts, margin, sizes);
    balance_partitions(graph, num_parts, margin, sizes);
    fix_disconnected_partitions(graph, num_parts);
    optimize_partitions(graph, num_parts, margin, sizes);
    fix_disconnected_partitions(graph, num_parts);
    balance_partitions(graph, num_parts, margin, sizes);
    remove_cross_partition_connections(graph);

    calculate_partition_sizes(graph, num_parts, sizes);
    if (!partitions_are_balanced(sizes, num_parts, ideal, margin)) {
        fprintf(stderr, "Warning: partition balance exceeded margin\n");
    }

    free(sizes);
    return true;
}
