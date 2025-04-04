#include "gsplit.h"

void    save_binary()
{
    //calculate size
    //write header
    //write some data in text format
}

void    save_text(t_gsplit *info, t_graph *graphs)
{
    int matrix_columns = strcountch(info->node_index, ',') - 1;
    fprintf(info->output, "%d\n%d\n%d\n%d\n", info->opts->parts, info->max_node_num, matrix_columns, (int)graphs[0].nodes_num);
    fprintf(info->output, "%s\n%s\n", info->nodes_in_row, info->node_index);

    int *end_table = malloc(sizeof(int) * info->opts->parts);
    int *offset_table = malloc(sizeof(int) * (graphs[0].nodes_num + 1));
    int *edge_table = malloc(sizeof(int) * (graphs[0].nodes_num * 2));

    if (!end_table || !offset_table || !edge_table)
        err_free_print(info, ERROR_ALLOC, NULL, &graphs[0]);
    int edge_index = 0;
    for (size_t i = 0; i < graphs[0].nodes_num; i++)
    {
        t_node *node = &graphs[0].nodes[i];
        offset_table[i] = edge_index;
        for (size_t j = 0; j < node->connections_num; j++)
        {
            int neighbor_index = node->connections[j] - graphs[0].nodes;

            if (i < (size_t)neighbor_index)
                edge_table[edge_index++] = neighbor_index;
        }
    }

    offset_table[graphs[0].nodes_num] = edge_index;
    int current_partition = 0;
    for (size_t i = 0; i < graphs[0].nodes_num; i++)
    {
        if (graphs[0].nodes[i].partition > current_partition)
        {
            end_table[current_partition] = i - 1;
            current_partition = graphs[0].nodes[i].partition;
        }
    }
    end_table[current_partition] = graphs[0].nodes_num - 1;

    for (int i = 0; i < info->opts->parts; i++)
    {
        fprintf(info->output, "%d", end_table[i]);
        if (i != info->opts->parts - 1)
            fprintf(info->output, ",");
    }
    fprintf(info->output, "\n");
    for (size_t i = 0; i <= graphs[0].nodes_num; i++)
    {
        fprintf(info->output, "%d", offset_table[i]);
        if (i != graphs[0].nodes_num - 1)
            fprintf(info->output, ",");
    }
    fprintf(info->output, "\n");
    for (int i = 0; i < edge_index; i++)
    {
        fprintf(info->output, "%d", edge_table[i]);
        if (i != edge_index - 1)
            fprintf(info->output, ",");
    }
    fprintf(info->output, "\n");
    free(end_table);
    free(offset_table);
    free(edge_table);
    
}
