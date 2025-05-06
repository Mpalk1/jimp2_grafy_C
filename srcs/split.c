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

static bool partitions_are_balanced(size_t *sizes, int num_parts, size_t ideal, int margin) {
    int allowed_diff = ideal * margin / 100;
    for (int i = 0; i < num_parts; i++) {
        if (sizes[i] < ideal - allowed_diff || sizes[i] > ideal + allowed_diff) {
            return false;
        }
    }
    return true;
}

static size_t count_edges(t_graph *graph) {
    size_t total_connections = 0;
    for (size_t i = 0; i < graph->nodes_num; i++) {
        total_connections += graph->nodes[i].connections_num;
    }
    return total_connections / 2;
}


void initialize_partitions(t_graph *graph, int num_parts, int margin) {
    size_t *sizes = calloc(num_parts, sizeof(size_t));
    if (!sizes) return;
    size_t ideal = graph->nodes_num / num_parts;
    int allowed_diff = ideal * margin / 100; // domyślny 10% margines

    bool *visited = calloc(graph->nodes_num, sizeof(bool));
    if (!visited) { free(sizes); return; }


    int part = 0;
    for (size_t start = 0; start < graph->nodes_num; start++) {
        if (visited[start]) continue;

        size_t queue_size = graph->nodes_num;
        size_t *queue = malloc(queue_size * sizeof(size_t));
        if (!queue) break;
        size_t qh = 0, qt = 0;

        queue[qt++] = start;
        visited[start] = true;

        while (qh < qt) {
            size_t u = queue[qh++];

            graph->nodes[u].partition = part;
            sizes[part]++;

            if (sizes[part] >= ideal + allowed_diff && part < num_parts - 1) part++;

            for (size_t i = 0; i < graph->nodes[u].connections_num; i++) {
                size_t v = graph->nodes[u].connections[i] - graph->nodes;
                if (!visited[v]) {
                    queue[qt++] = v;
                    visited[v] = true;
                }
            }
        }
        free(queue);
    }


    for (size_t i = 0; i < graph->nodes_num; i++) {
        if (!visited[i]) {
            graph->nodes[i].partition = part;
            sizes[part]++;
            if (sizes[part] >= ideal + allowed_diff && part < num_parts - 1) part++;
        }
    }

    free(visited);
    free(sizes);
}

static void optimize_partitions(t_graph *graph, int num_parts, int margin, size_t *sizes) {
    int iterations = 0;
    size_t ideal = graph->nodes_num / num_parts;
    int allowed_diff = ideal * margin / 100;

    do {
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
        }

        iterations++;
    } while (iterations < OPTIMIZE_MAX_ITER);
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
                if (!balanced) break;
            }
        }
    }
}

static void fix_disconnected_partitions(t_graph *graph, int num_parts, int margin) {
    size_t ideal = graph->nodes_num / num_parts;
    int allowed_diff = ideal * margin / 100;
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

                t_node *node = &graph->nodes[i];
                if (node->connections_num == 0)
                    continue;

                bool has_local_connection = false;
                for (size_t j = 0; j < node->connections_num; j++) {
                    if (node->connections[j]->partition == p) {
                        has_local_connection = true;
                        break;
                    }
                }

                if (has_local_connection)
                    continue;

                int *partition_counts = calloc(num_parts, sizeof(int));
                if (!partition_counts)
                    continue;

                int best_partition = -1;
                int most_connections = 0;

                for (size_t j = 0; j < node->connections_num; j++) {
                    int neighbor_part = node->connections[j]->partition;
                    partition_counts[neighbor_part]++;
                }

                for (int np = 0; np < num_parts; np++) {
                    if (np == p) continue;
                    if (partition_counts[np] > most_connections && sizes[np] < ideal + allowed_diff) {
                        most_connections = partition_counts[np];
                        best_partition = np;
                    }
                }

                free(partition_counts);

                if (best_partition != -1 && most_connections >= 1) {
                    graph->nodes[i].partition = best_partition;
                    sizes[best_partition]++;
                    sizes[p]--;
                    changes_made = true;
                }
            }
        }
    } while (changes_made);

    free(sizes);
}

static size_t remove_cross_partition_connections(t_graph *graph) {
    size_t total_removed = 0;
    for (size_t i = 0; i < graph->nodes_num; i++) {
        t_node *node = &graph->nodes[i];
        int current_part = node->partition;

        size_t keep_count = 0;
        for (size_t j = 0; j < node->connections_num; j++) {
            if (node->connections[j]->partition == current_part) {
                keep_count++;
            }
        }

        if (keep_count < node->connections_num)
            total_removed += node->connections_num - keep_count;

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
    return total_removed;
}

bool partition_graph(t_graph *graph, int num_parts, int margin, t_options *opts) {
    size_t *sizes = calloc(num_parts, sizeof(size_t));
    if (!sizes) return false;

    size_t ideal = graph->nodes_num / num_parts;

    initialize_partitions(graph, num_parts, margin);
    fix_disconnected_partitions(graph, num_parts, margin);
    optimize_partitions(graph, num_parts, margin, sizes);
    fix_disconnected_partitions(graph, num_parts, margin);
    balance_partitions(graph, num_parts, margin, sizes);
    fix_disconnected_partitions(graph, num_parts, margin);
    optimize_partitions(graph, num_parts, margin, sizes);
    fix_disconnected_partitions(graph, num_parts, margin);
    balance_partitions(graph, num_parts, margin, sizes);
    fix_disconnected_partitions(graph, num_parts, margin);
    size_t cuts = remove_cross_partition_connections(graph);
    fix_disconnected_partitions(graph, num_parts, margin);

    
    


    if (opts->verbose){
        printf("*****");
        printf("\nIlość krawędzi grafu: %zu\n", count_edges(graph));
        printf("Ilość przeciętych krawędzi: %zu\n", cuts);
        printf("*****");
    }

    calculate_partition_sizes(graph, num_parts, sizes);
    if (!partitions_are_balanced(sizes, num_parts, ideal, margin) && !opts->force) {
        err_print(ERROR_MARGIN_EXCEEDED);
        free(sizes);
        return false;
    } else if (opts->verbose) {
        printf("Rożnica pomiędzy podgrafami mieści sie w marginesie.\n");
    }

    free(sizes);
    return true;
}
