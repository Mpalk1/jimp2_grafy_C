#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "gsplit.h"


static size_t count_nodes_in_partition(t_graph *graph, int partition) {
    size_t count = 0;
    for (size_t i = 0; i < graph->nodes_num; i++) {
        if (graph->nodes[i].partition == partition) count++;
    }
    return count;
}


static bool is_partition_connected(t_graph *graph, int partition) {
    bool *visited = calloc(graph->nodes_num, sizeof(bool));
    if (!visited) return false;

    
    size_t start_node = graph->nodes_num;  
    for (size_t i = 0; i < graph->nodes_num; i++) {
        if (graph->nodes[i].partition == partition) {
            start_node = i;
            break;
        }
    }

    if (start_node == graph->nodes_num) {  
        free(visited);
        return false;
    }

    
    size_t *queue = malloc(graph->nodes_num * sizeof(size_t));
    if (!queue) {
        free(visited);
        return false;
    }

    size_t front = 0, rear = 0;
    queue[rear++] = start_node;
    visited[start_node] = true;
    size_t visited_count = 1;

    while (front < rear) {
        size_t current = queue[front++];

        for (size_t i = 0; i < graph->nodes[current].connections_num; i++) {
            t_node *neighbor = graph->nodes[current].connections[i];
            size_t neighbor_idx = neighbor - graph->nodes;

            if (neighbor->partition == partition && !visited[neighbor_idx]) {
                visited[neighbor_idx] = true;
                queue[rear++] = neighbor_idx;
                visited_count++;
            }
        }
    }

    free(queue);
    free(visited);
    return (visited_count == count_nodes_in_partition(graph, partition));
}


static void fix_disconnected_partitions(t_graph *graph, int num_parts) {
    for (int p = 0; p < num_parts; p++) {
        if (!is_partition_connected(graph, p)) {
            for (size_t i = 0; i < graph->nodes_num; i++) {
                if (graph->nodes[i].partition == p) {
                    bool has_local_connection = false;
                    for (size_t j = 0; j < graph->nodes[i].connections_num; j++) {
                        if (graph->nodes[i].connections[j]->partition == p) {
                            has_local_connection = true;
                            break;
                        }
                    }

                    if (!has_local_connection && graph->nodes[i].connections_num > 0) {
                        
                        graph->nodes[i].partition = graph->nodes[i].connections[0]->partition;
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

void calculate_partition_sizes(t_graph *graph, int num_parts, size_t sizes[]) {
    for (int i = 0; i < num_parts; i++) {
        sizes[i] = 0;
    }
    
    for (size_t i = 0; i < graph->nodes_num; i++) {
        sizes[graph->nodes[i].partition]++;
    }
}

int compute_gain(t_graph *graph, size_t node_idx, int new_part) {
    int gain = 0;
    int current_part = graph->nodes[node_idx].partition;
    
    for (size_t i = 0; i < graph->nodes[node_idx].connections_num; i++) {
        t_node *neighbor = graph->nodes[node_idx].connections[i];
        if (neighbor->partition == current_part) gain--;
        else if (neighbor->partition == new_part) gain++;
    }
    return gain;
}

void optimize_partitions(t_graph *graph, int num_parts, int margin) {
    size_t sizes[num_parts];
    int improved;
    const int max_iterations = 500;
    int iterations = 0;
    size_t ideal = graph->nodes_num / num_parts;
    int allowed_diff = ideal * margin / 100;

    do {
        improved = 0;
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
            improved = 1;
        }

        iterations++;
    } while (improved && iterations < max_iterations);
}

void balance_partitions(t_graph *graph, int num_parts, int margin) {
    size_t sizes[num_parts];
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

void partition_graph(t_graph *graph, int num_parts, int margin) {
    if (num_parts < 2) return;
    
    initialize_partitions(graph, num_parts);
    optimize_partitions(graph, num_parts, margin);
    balance_partitions(graph, num_parts, margin);
    fix_disconnected_partitions(graph, num_parts);
    optimize_partitions(graph, num_parts, margin); 
}