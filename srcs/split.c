#include "gsplit.h"

static void initialize_partitions(t_graph *graph, int num_parts) {
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

static void calculate_partition_sizes(t_graph *graph, int num_parts, size_t *sizes) {
    for (int i = 0; i < num_parts; i++) {
        sizes[i] = 0;
    }
    
    for (size_t i = 0; i < graph->nodes_num; i++) {
        sizes[graph->nodes[i].partition]++;
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

static void    optimize_partitions(t_graph *graph, int num_parts, int margin, size_t *sizes) {
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
    } while (improved && iterations < OPTIMIZE_MAX_ITER);
}

static void balance_partitions(t_graph *graph, int num_parts, int margin, size_t *sizes) {
    calculate_partition_sizes(graph, num_parts, sizes);
    
    size_t ideal = graph->nodes_num / num_parts;
    int allowed_diff = ideal * margin / 100;

    for (int i = 0; i < num_parts; i++) {
        while (sizes[i] < ideal - allowed_diff) {
            int max_part = 0;
            for (int p = 0; p < num_parts; p++) {
                if (sizes[p] > sizes[max_part] && p != i) {
                    max_part = p;
                }
            }

            size_t best_node = 0;
            int best_gain = INT_MIN;
            
            for (size_t n = 0; n < graph->nodes_num; n++) {
                if (graph->nodes[n].partition != max_part) continue;
                
                int current_gain = compute_gain(graph, n, i) - 
                                 compute_gain(graph, n, max_part);
                
                if (current_gain > best_gain) {
                    best_gain = current_gain;
                    best_node = n;
                }
            }

            if (best_gain > INT_MIN) {
                graph->nodes[best_node].partition = i;
                sizes[max_part]--;
                sizes[i]++;
            } else {
                break;
            }
        }
    }
}

bool partition_graph(t_graph *graph, int num_parts, int margin) {
    size_t  *sizes;

    sizes = (size_t *)calloc(sizeof(size_t), num_parts);
    if (!sizes)
        return (false);
    srand(time(NULL));
    initialize_partitions(graph, num_parts);
    optimize_partitions(graph, num_parts, margin, sizes);
    balance_partitions(graph, num_parts, margin, sizes);
    optimize_partitions(graph, num_parts, margin, sizes);
    free(sizes);
    return (true);
}