#include "gsplit.h"

void    save_binary()
{
    //calculate size
    //write header
    //write some data in text format
}

size_t  count_connections(t_graph *graphs)
{
    size_t result = 0;

    for (size_t i = 0; i < graphs[0].nodes_num; i++)
        result += graphs[0].nodes[i].connections_num;
    return (result);
}

void    free_output_data(t_output_data *data)
{
    free(data->edge_table);
    free(data->offset_table);
    free(data->end_table);
}

bool    allocate_output(t_output_data *data, size_t end_size, size_t off_size, size_t edge_size)
{
    data->edge_table = NULL;
    data->offset_table = NULL;
    data->end_table = NULL;
    data->end_table = malloc(sizeof(__uint16_t) * end_size);
    if (!data->end_table)
        return (false);
    data->offset_table = malloc(sizeof(__uint16_t) * off_size);
    if (!data->offset_table)
    {
        free_output_data(data);
        return (false);
    }
    data->edge_table = malloc(sizeof(__uint16_t) * edge_size);
    if (!data->edge_table)
    {
        free_output_data(data);
        return (false);
    }
    return (true);
}

void    get_output_tables(t_output_data *data, t_gsplit *info, t_graph *graphs, size_t connections)
{
    if (!allocate_output(data, info->opts->parts, graphs[0].nodes_num, connections))
        err_free_print(info, ERROR_ALLOC, NULL, graphs);
    data->rows_count = strcountch(info->node_index, ',') - 1;
    data->nodes_count = (int)graphs[0].nodes_num;
    data->columns_count = info->max_node_num;
    data->parts_count = info->opts->parts;
    //split nodes in row and node index
    //info->nodes_in_row, info->node_index
    // data->file_size = 13 + 2 * data->nodes_count + 2 * (data->columns_count + 1) + 2 * data->parts_count + 2 * (ostatni element tabeli końców + 1) + 2 * (ostatni element tabeli przesunięć + 1);
    int edge_index = 0;
    int offset_index = 0;
    int end_table_index = 0;
    int last_node_in_partition = -1;
    for (int part = 0; part < info->opts->parts; part++)
    {
        for (size_t i = 0; i < graphs[0].nodes_num; i++)
        {
            if (part == graphs[0].nodes[i].partition)
            {
                t_node *node = &graphs[0].nodes[i];
                for (size_t j = 0; j < node->connections_num; j++)
                {
                    int neighbor_index = node->connections[j] - graphs[0].nodes;
                    data->edge_table[edge_index++] = neighbor_index;
                }
                data->offset_table[offset_index++] = edge_index - 1;
                last_node_in_partition = offset_index - 1;
            }
        }
        data->end_table[end_table_index++] = last_node_in_partition;
    }
}

void    save_text(t_gsplit *info, t_graph *graphs)
{
    t_output_data   data;
    const size_t  connections = count_connections(graphs);
    
    get_output_tables(&data, info, graphs, connections);
    fprintf(info->output, "%d\n%d\n%d\n%d\n", data.parts_count, data.rows_count, data.columns_count, data.nodes_count);
    fprintf(info->output, "%s\n%s\n", "temp", "temp");//todo
    for (int i = 0; i < info->opts->parts; i++)
    {
        fprintf(info->output, "%d", data.end_table[i]);
        if (i != info->opts->parts - 1)
            fprintf(info->output, ",");
    }
    fprintf(info->output, "\n");
    for (size_t i = 0; i < graphs[0].nodes_num; i++)
    {
        fprintf(info->output, "%d", data.offset_table[i]);
        if (i != graphs[0].nodes_num - 1)
            fprintf(info->output, ",");
    }
    fprintf(info->output, "\n");
    for (size_t i = 0; i < connections; i++)
    {
        fprintf(info->output, "%d", data.edge_table[i]);
        if (i != connections - 1)
            fprintf(info->output, ",");
    }
    fprintf(info->output, "\n");
    free_output_data(&data);
}
