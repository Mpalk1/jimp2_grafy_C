#include "gsplit.h"

void    print_graphs(t_graph *graph, int graphs_num)
{
    for (int i = 0; i < graphs_num; i++)
    {
        printf("\tGraph %i\n", i);
        for (size_t j = 0; j < graph[i].nodes_num; j++)
        {
            printf("Node %zu(%zu):", j, graph[i].nodes[j].connections_num);
            fflush(stdout); 
            for (size_t k = 0; k < graph[i].nodes[j].connections_num; k++)
            {
                printf(" %li", graph[i].nodes[j].connections[k] - graph[i].nodes );
                // printf(" %zu", k);
            }
            printf("\n");
        }
    }
}

void print_graphs1(t_graph *graphs, int graphs_num) {
    for (int i = 0; i < graphs_num; i++) {
        printf("Graph %d:\n", i + 1);

        if (graphs[i].nodes == NULL) {
            printf("  No nodes in this graph.\n");
            continue;
        }

        if (graphs[i].nodes_num == 0) {
            printf("  No nodes to display.\n");
            continue;
        }
        
     
        int max_partition = 0;
        for (size_t j = 0; j < graphs[i].nodes_num; j++) {
            if (graphs[i].nodes[j].partition > max_partition) {
                max_partition = graphs[i].nodes[j].partition;
            }
        }
        int partition_count = max_partition + 1;
        
        
        int *part_counts = calloc(partition_count, sizeof(int));
        if (!part_counts) {
            printf("  Memory allocation failed for partition counts\n");
            continue;
        }

        
        for (size_t j = 0; j < graphs[i].nodes_num; j++) {
            int part = graphs[i].nodes[j].partition;
            if (part >= 0 && part < partition_count) {
                part_counts[part]++;
            }
        }

        printf("  Partition counts: ");
        for (int p = 0; p < partition_count; p++) {
            printf("%d:%d ", p, part_counts[p]);
        }
        printf("\n");

        
        for (size_t j = 0; j < graphs[i].nodes_num; j++) {
            printf("  Node %zu (Partition %d): ", j, graphs[i].nodes[j].partition);
            
            if (graphs[i].nodes[j].connections_num > 0) {
                printf("Connects to: ");
                for (size_t k = 0; k < graphs[i].nodes[j].connections_num; k++) {
                    t_node *connected = graphs[i].nodes[j].connections[k];
                    printf("%ld ", connected - graphs[i].nodes);
                }
            }
            printf("\n");
        }

        free(part_counts);
    }
}