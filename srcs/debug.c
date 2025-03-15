#include "gsplit.h"

void    print_graph(t_graph *graph)
{
    for (size_t i = 0; i < graph->nodes_num; i++)
    {
        printf("Node %zu\n", i);
    }
}
