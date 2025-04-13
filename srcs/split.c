#include "gsplit.h"

static void calculate_partition_sizes(t_graph *graph, int num_parts, size_t *sizes) {
    for (int i = 0; i < num_parts; i++) sizes[i] = 0;
    for (size_t i = 0; i < graph->nodes_num; i++) sizes[graph->nodes[i].partition]++;
}

static void ensure_internal_connectivity(t_graph *graph, int num_parts) {
    bool changed;
    do {
        changed = false;
        for (int p = 0; p < num_parts; p++) {
            bool *visited = calloc(graph->nodes_num, sizeof(bool));
            int *queue = calloc(graph->nodes_num, sizeof(int));
            size_t q_start = 0, q_end = 0;

            int first = -1;
            for (size_t i = 0; i < graph->nodes_num; i++) {
                if (graph->nodes[i].partition == p) {
                    first = i;
                    break;
                }
            }
            if (first == -1) {
                free(visited); free(queue);
                continue;
            }
            queue[q_end++] = first;
            visited[first] = true;

            while (q_start < q_end) {
                int cur = queue[q_start++];
                for (size_t j = 0; j < graph->nodes[cur].connections_num; j++) {
                    t_node *neighbor = graph->nodes[cur].connections[j];
                    int ni = neighbor - graph->nodes;
                    if (!visited[ni] && neighbor->partition == p) {
                        visited[ni] = true;
                        queue[q_end++] = ni;
                    }
                }
            }

            for (size_t i = 0; i < graph->nodes_num; i++) {
                if (graph->nodes[i].partition == p && !visited[i]) {
                    for (size_t j = 0; j < graph->nodes[i].connections_num; j++) {
                        int np = graph->nodes[i].connections[j]->partition;
                        if (np != p) {
                            graph->nodes[i].partition = np;
                            changed = true;
                            break;
                        }
                    }
                }
            }
            free(visited); free(queue);
        }
    } while (changed);
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
                            if (graph->nodes[n].partition != i) continue;
                            for (size_t c = 0; c < graph->nodes[n].connections_num; c++) {
                                if (graph->nodes[n].connections[c]->partition == j) {
                                    graph->nodes[n].partition = j;
                                    sizes[i]--;
                                    sizes[j]++;
                                    balanced = false;
                                    break;
                                }
                            }
                            if (!balanced) break;
                        }
                        if (!balanced) break;
                    }
                }
            }
        }
    }
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
                bool has_neighbor = false;
                for (size_t j = 0; j < graph->nodes[i].connections_num; j++) {
                    if (graph->nodes[i].connections[j]->partition == p) {
                        has_neighbor = true;
                        break;
                    }
                }
                if (!has_neighbor) continue;
                if (sizes[p] >= ideal + allowed_diff) continue;
                if (sizes[current_part] <= ideal - allowed_diff) continue;
                int gain = compute_gain(graph, i, p);
                if (gain > best_move.gain) {
                    best_move = (t_move){i, gain, p};
                }
            }
        }
        if (best_move.target_part != -1) {
            int old = graph->nodes[best_move.node_index].partition;
            graph->nodes[best_move.node_index].partition = best_move.target_part;
            sizes[old]--;
            sizes[best_move.target_part]++;
            improved = true;
        }
        iterations++;
    } while (improved && iterations < OPTIMIZE_MAX_ITER);
}

void initialize_partitions(t_graph *graph, int num_parts) {
    size_t part_size = graph->nodes_num / num_parts;
    size_t remainder = graph->nodes_num % num_parts;
    size_t current_part = 0;
    size_t count = 0;
    for (size_t i = 0; i < graph->nodes_num; i++) {
        graph->nodes[i].partition = current_part;
        count++;
        size_t target = part_size + (current_part < remainder ? 1 : 0);
        if (count >= target && current_part < (size_t)(num_parts - 1)) {
            current_part++;
            count = 0;
        }
    }
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

bool partition_graph(t_graph *graph, int num_parts, int margin) {
    size_t *sizes = calloc(num_parts, sizeof(size_t));
    if (!sizes) return false;
    initialize_partitions(graph, num_parts);
    calculate_partition_sizes(graph, num_parts, sizes);
    ensure_internal_connectivity(graph, num_parts);
    balance_partitions(graph, num_parts, margin, sizes);
    optimize_partitions(graph, num_parts, margin, sizes);
    ensure_internal_connectivity(graph, num_parts);
    balance_partitions(graph, num_parts, margin, sizes);
    remove_cross_partition_connections(graph);
    free(sizes);
    return true;
}
