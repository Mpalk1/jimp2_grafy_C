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

        // Count nodes per partition
        int part_counts[4] = {0}; // Assuming max 4 partitions
        for (size_t j = 0; j < graphs[i].nodes_num; j++) {
            int part = graphs[i].nodes[j].partition;
            if (part >= 0 && part < 4) part_counts[part]++;
        }

        printf("  Partition counts: ");
        for (int p = 0; p < 4; p++) {
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
    }
}