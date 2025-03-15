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
