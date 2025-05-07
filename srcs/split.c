#include "gsplit.h"

static void calculate_subgraph_sizes(t_graph *graph, int num_parts,
                                     size_t *sizes) {
    for (int i = 0; i < num_parts; i++)
        sizes[i] = 0;
    for (size_t i = 0; i < graph->nodes_num; i++)
        sizes[graph->nodes[i].subgraph]++;
}

static int count_gain(t_graph *graph, size_t node_idx, int new_part) {
    int gain = 0;
    int current_part = graph->nodes[node_idx].subgraph;

    for (size_t i = 0; i < graph->nodes[node_idx].connections_num; i++) {
        t_node *neighbor = graph->nodes[node_idx].connections[i];
        if (neighbor->subgraph == current_part)
            gain--;
        else if (neighbor->subgraph == new_part)
            gain++;
    }
    return gain;
}

static bool subgraphs_are_balanced(size_t *sizes, int num_parts, size_t ideal,
                                   int margin) {
    int allowed_diff = ideal * margin / 100;
    for (int i = 0; i < num_parts; i++) {
        if (sizes[i] < ideal - allowed_diff ||
            sizes[i] > ideal + allowed_diff) {
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

typedef struct {
    size_t node;
    int partition;
} bfs_entry;

void subgraphs_init(t_graph *graph, int num_parts, int margin) {
    size_t ideal = graph->nodes_num / num_parts;
    int allowed_diff = ideal * margin / 100;
    size_t max_size = ideal + allowed_diff;

    bool *visited = calloc(graph->nodes_num, sizeof(bool));
    size_t *sizes = calloc(num_parts, sizeof(size_t));
    bfs_entry *queue = malloc(graph->nodes_num * sizeof(bfs_entry));
    if (!visited || !sizes || !queue) {
        free(visited);
        free(sizes);
        free(queue);
        return;
    }

    size_t picked = 0, q_end = 0;
    for (size_t i = 0; i < graph->nodes_num && picked < (size_t)num_parts;
         i++) {
        if (visited[i] || graph->nodes[i].connections_num == 0)
            continue;

        visited[i] = true;
        queue[q_end++] = (bfs_entry){.node = i, .partition = picked++};
    }

    size_t q_start = 0;
    while (q_start < q_end) {
        bfs_entry current = queue[q_start++];
        size_t u = current.node;
        int part = current.partition;

        if (sizes[part] >= max_size)
            continue;

        bool connected = sizes[part] == 0;
        for (size_t j = 0; j < graph->nodes[u].connections_num && !connected;
             j++) {
            if (graph->nodes[u].connections[j]->subgraph == part)
                connected = true;
        }
        if (!connected)
            continue;

        graph->nodes[u].subgraph = part;
        sizes[part]++;

        if (sizes[part] >= max_size)
            continue;

        for (size_t j = 0; j < graph->nodes[u].connections_num; j++) {
            size_t v = graph->nodes[u].connections[j] - graph->nodes;
            if (!visited[v]) {
                visited[v] = true;
                queue[q_end++] = (bfs_entry){.node = v, .partition = part};
            }
        }
    }

    for (size_t i = 0; i < graph->nodes_num; i++) {
        if (visited[i])
            continue;

        int best_part = -1, best_score = -1;
        for (int p = 0; p < num_parts; p++) {
            if (sizes[p] >= max_size)
                continue;

            if (graph->nodes[i].connections_num == 0) {
                if (best_part == -1 || sizes[p] < sizes[best_part]) {
                    best_part = p;
                }
                continue;
            }

            int score = 0;
            for (size_t j = 0; j < graph->nodes[i].connections_num; j++) {
                if (graph->nodes[i].connections[j]->subgraph == p)
                    score++;
            }

            if (score > best_score) {
                best_score = score;
                best_part = p;
            }
        }

        if (best_part == -1)
            best_part = i % num_parts;

        graph->nodes[i].subgraph = best_part;
        sizes[best_part]++;
        visited[i] = true;
    }

    free(queue);
    free(visited);
    free(sizes);
}

static void optimize_subgraphs(t_graph *graph, int num_parts, int margin,
                               size_t *sizes) {
    int iterations = 0;
    size_t ideal = graph->nodes_num / num_parts;
    int allowed_diff = ideal * margin / 100;

    while (iterations < OPTIMIZE_MAX_ITER) {
        t_move best_move = {0, INT_MIN, -1};
        calculate_subgraph_sizes(graph, num_parts, sizes);

        for (size_t i = 0; i < graph->nodes_num; i++) {
            int current_part = graph->nodes[i].subgraph;

            for (int p = 0; p < num_parts; p++) {
                if (p == current_part)
                    continue;

                bool has_neighbor_in_p = false;
                for (size_t j = 0; j < graph->nodes[i].connections_num; j++) {
                    if (graph->nodes[i].connections[j]->subgraph == p) {
                        has_neighbor_in_p = true;
                        break;
                    }
                }
                if (!has_neighbor_in_p)
                    continue;
                if (sizes[p] >= ideal + allowed_diff)
                    continue;
                if (sizes[current_part] <= ideal - allowed_diff)
                    continue;

                int gain = count_gain(graph, i, p);
                int balance_bonus = 0;
                if (sizes[p] < ideal)
                    balance_bonus += 2;
                if (sizes[current_part] > ideal)
                    balance_bonus += 2;

                if ((gain + balance_bonus) > best_move.gain) {
                    best_move.node_index = i;
                    best_move.gain = gain + balance_bonus;
                    best_move.target_part = p;
                }
            }
        }

        if (best_move.target_part != -1) {
            int old_part = graph->nodes[best_move.node_index].subgraph;
            graph->nodes[best_move.node_index].subgraph = best_move.target_part;
            sizes[old_part]--;
            sizes[best_move.target_part]++;
        }

        iterations++;
    }
}

static void fix_disconnected_subgraphs(t_graph *graph, int num_parts,
                                       int margin) {
    size_t ideal = graph->nodes_num / num_parts;
    int allowed_diff = ideal * margin / 100;
    size_t *sizes = calloc(num_parts, sizeof(size_t));
    if (!sizes)
        return;
    calculate_subgraph_sizes(graph, num_parts, sizes);

    bool *visited = calloc(graph->nodes_num, sizeof(bool));
    int *component_id = malloc(graph->nodes_num * sizeof(int));
    size_t *component_sizes = NULL;
    size_t *queue = malloc(graph->nodes_num * sizeof(size_t));
    if (!visited || !component_id || !queue) {
        free(sizes);
        free(visited);
        free(component_id);
        free(queue);
        return;
    }

    for (int p = 0; p < num_parts; p++) {

        memset(visited, 0, graph->nodes_num * sizeof(bool));

        int comp_idx = 0;

        for (size_t i = 0; i < graph->nodes_num; i++) {
            if (graph->nodes[i].subgraph != p || visited[i])
                continue;

            size_t queue_head = 0, queue_tail = 0;
            queue[queue_tail++] = i;
            visited[i] = true;
            component_id[i] = comp_idx;

            while (queue_head < queue_tail) {
                size_t node_idx = queue[queue_head++];

                for (size_t j = 0; j < graph->nodes[node_idx].connections_num;
                     j++) {
                    size_t neighbor_idx =
                        graph->nodes[node_idx].connections[j] - graph->nodes;

                    if (graph->nodes[neighbor_idx].subgraph == p &&
                        !visited[neighbor_idx]) {
                        queue[queue_tail++] = neighbor_idx;
                        visited[neighbor_idx] = true;
                        component_id[neighbor_idx] = comp_idx;
                    }
                }
            }

            comp_idx++;
        }

        if (comp_idx <= 1)
            continue;

        component_sizes = calloc(comp_idx, sizeof(size_t));
        if (!component_sizes)
            continue;

        for (size_t i = 0; i < graph->nodes_num; i++) {
            if (graph->nodes[i].subgraph == p) {
                component_sizes[component_id[i]]++;
            }
        }

        int main_component = 0;
        for (int c = 1; c < comp_idx; c++) {
            if (component_sizes[c] > component_sizes[main_component]) {
                main_component = c;
            }
        }

        for (int c = 0; c < comp_idx; c++) {
            if (c == main_component)
                continue;

            int *connections_to_part = calloc(num_parts, sizeof(int));
            if (!connections_to_part)
                continue;

            for (size_t i = 0; i < graph->nodes_num; i++) {
                if (graph->nodes[i].subgraph != p || component_id[i] != c)
                    continue;

                for (size_t j = 0; j < graph->nodes[i].connections_num; j++) {
                    int neighbor_part =
                        graph->nodes[i].connections[j]->subgraph;
                    if (neighbor_part != p) {
                        connections_to_part[neighbor_part]++;
                    }
                }
            }

            int best_part = -1;
            int most_connections = 0;

            for (int target_p = 0; target_p < num_parts; target_p++) {
                if (target_p == p)
                    continue;

                if (connections_to_part[target_p] > most_connections &&
                    sizes[target_p] + component_sizes[c] <=
                        ideal + allowed_diff) {
                    most_connections = connections_to_part[target_p];
                    best_part = target_p;
                }
            }

            if (best_part == -1 &&
                component_sizes[c] <=
                    ideal + allowed_diff - component_sizes[main_component]) {

                free(connections_to_part);
                continue;
            }

            if (best_part != -1) {
                for (size_t i = 0; i < graph->nodes_num; i++) {
                    if (graph->nodes[i].subgraph == p && component_id[i] == c) {
                        graph->nodes[i].subgraph = best_part;
                        sizes[p]--;
                        sizes[best_part]++;
                    }
                }
            }

            free(connections_to_part);
        }

        free(component_sizes);
        component_sizes = NULL;
    }

    for (size_t i = 0; i < graph->nodes_num; i++) {
        if (graph->nodes[i].connections_num == 0) {

            int min_part = 0;
            for (int p = 1; p < num_parts; p++) {
                if (sizes[p] < sizes[min_part]) {
                    min_part = p;
                }
            }

            if (graph->nodes[i].subgraph != min_part) {
                sizes[graph->nodes[i].subgraph]--;
                graph->nodes[i].subgraph = min_part;
                sizes[min_part]++;
            }
        }
    }

    free(visited);
    free(component_id);
    free(queue);
    free(sizes);
}

static size_t remove_subgraphs_connections(t_graph *graph) {
    size_t removed_directed = 0;

    for (size_t u = 0; u < graph->nodes_num; u++) {
        t_node *node = &graph->nodes[u];
        size_t old_deg = node->connections_num;

        size_t keep_count = 0;
        for (size_t i = 0; i < old_deg; i++) {
            if (node->connections[i]->subgraph == node->subgraph) {
                keep_count++;
            }
        }

        t_node **new_conns = calloc(keep_count, sizeof(t_node *));
        size_t idx = 0;
        for (size_t i = 0; i < old_deg; i++) {
            t_node *nbr = node->connections[i];
            if (nbr->subgraph == node->subgraph) {
                new_conns[idx++] = nbr;
            }
        }

        removed_directed += (old_deg - keep_count);
        free(node->connections);
        node->connections = new_conns;
        node->connections_num = keep_count;
    }

    return removed_directed / 2;
}

static bool check_partitions_connected(t_graph *graph, int num_parts) {

    size_t *sizes = calloc(num_parts, sizeof(size_t));
    calculate_subgraph_sizes(graph, num_parts, sizes);

    bool *visited = calloc(graph->nodes_num, sizeof(bool));
    size_t *queue = malloc(graph->nodes_num * sizeof(size_t));
    if (!sizes || !visited || !queue) {
        free(sizes);
        free(visited);
        free(queue);
        return false;
    }

    for (int p = 0; p < num_parts; p++) {
        if (sizes[p] == 0)
            continue;

        size_t start = 0;
        while (start < graph->nodes_num && graph->nodes[start].subgraph != p)
            start++;
        if (start == graph->nodes_num)
            continue;

        memset(visited, 0, graph->nodes_num * sizeof(bool));
        size_t qh = 0, qt = 0, cnt = 0;
        visited[start] = true;
        queue[qt++] = start;

        while (qh < qt) {
            size_t u = queue[qh++];
            cnt++;
            for (size_t i = 0; i < graph->nodes[u].connections_num; i++) {
                size_t v = graph->nodes[u].connections[i] - graph->nodes;
                if (!visited[v] && graph->nodes[v].subgraph == p) {
                    visited[v] = true;
                    queue[qt++] = v;
                }
            }
        }

        if (cnt != sizes[p]) {
            free(sizes);
            free(visited);
            free(queue);
            return false;
        }
    }

    free(sizes);
    free(visited);
    free(queue);
    return true;
}

bool make_subgraphs(t_graph *graph, int num_parts, int margin,
                    t_options *opts) {
    size_t *sizes = calloc(num_parts, sizeof(size_t));
    if (!sizes)
        return false;

    size_t ideal = graph->nodes_num / num_parts;

    if (opts->verbose) {
        printf("*****");
        printf("\nIlość krawędzi grafu: %zu\n", count_edges(graph));
        printf("*****\n");
    }

    subgraphs_init(graph, num_parts, margin);
    optimize_subgraphs(graph, num_parts, margin, sizes);
    fix_disconnected_subgraphs(graph, num_parts, margin);

    if (!check_partitions_connected(graph, num_parts) && !opts->force) {
        err_print(ERROR_DISCONNECTED_PARTITIONS);
        free(sizes);
        return false;
    }

    size_t cuts = remove_subgraphs_connections(graph);


    if (opts->verbose){
        printf("*****\n");
        printf("Ilość krawędzi grafu po przecięciach: %zu\n", count_edges(graph));
        printf("Ilość przeciętych krawędzi: %zu\n", cuts);
        int *part_counts = calloc(num_parts, sizeof(int));
        if (part_counts) {
            for (size_t j = 0; j < graph->nodes_num; j++) {
                int part = graph->nodes[j].subgraph;
                if (part >= 0 && part < num_parts) {
                    part_counts[part]++;
                }
            }
            printf("Ilość węzłów w podgrafach: ");
            for (int p = 0; p < num_parts; p++) {
                printf("%d:%d ", p, part_counts[p]);
            }
            printf("\n");
        }
    }

    calculate_subgraph_sizes(graph, num_parts, sizes);
    if (!subgraphs_are_balanced(sizes, num_parts, ideal, margin)) {
        if (opts->verbose && opts->force) {
            printf("Rożnica pomiędzy podgrafami nie mieści sie w marginesie, wymuszanie tworzenia grafu.\n");
        } else if (!opts->force) {
            err_print(ERROR_MARGIN_EXCEEDED);
            free(sizes);
            return false;
        }
    } else if (opts->verbose) {
        printf("Rożnica pomiędzy podgrafami mieści sie w marginesie.\n");
    }
    if(!check_partitions_connected(graph, num_parts)) {
        if (opts->verbose && opts->force) {
            printf("Podgrafy nie są spójne, wymuszanie tworzenia grafu.\n");
        } else if (!opts->force) {
            err_print(ERROR_DISCONNECTED_PARTITIONS);
            free(sizes);
            return false;
        }
    } else if (opts->verbose) {
        printf("Podgrafy są spójne.\n");
    }

    free(sizes);
    return true;
}
