#include "gsplit.h"
#include <stdint.h>
#include <arpa/inet.h>
#include <endian.h>

bool    copy_matrix_data(t_output_data *data, char *nodes_in_row, char *node_index)
{
    char			*saveptr = NULL;
	char			*token = NULL;
	unsigned int	i;
    data->indexes_in_row = (__uint16_t *)malloc(sizeof(__uint16_t) * data->nodes_count);
    if (!data->indexes_in_row)
    {
        free_output_data(data);
        return (false);
    }
    data->first_nodes_indexes = (__uint16_t *)malloc(sizeof(__uint16_t) * (data->columns_count + 1));
    if (!data->first_nodes_indexes)
    {
        free_output_data(data);
        return (false);
    }
	token = strtok_r(nodes_in_row, ",", &saveptr);
	for (i = 0; i < data->nodes_count && token != NULL; i++)
	{
		if (token[0] == '\0' || !is_uint(token))
		{
			free_output_data(data);
            return (false);
		}
		data->indexes_in_row[i] = atoi(token);
		token = strtok_r(NULL, ",", &saveptr);
	}
    token = strtok_r(node_index, ",", &saveptr);
	for (i = 0; i < (data->columns_count + 1) && token != NULL; i++)
	{
		if (token[0] == '\0' || !is_uint(token))
		{
			free_output_data(data);
            return (false);
		}
		data->first_nodes_indexes[i] = atoi(token);
		token = strtok_r(NULL, ",", &saveptr);
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
    data->signature = ('G' << 8) | 'S';
    if (!copy_matrix_data(data, info->nodes_in_row, info->node_index))
        err_free_print(info, ERROR_ALLOC, NULL, graphs);
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
    data->file_size = 16 + sizeof(__uint16_t) * data->nodes_count + sizeof(__uint16_t) * (data->columns_count + 1) + sizeof(__uint32_t) * data->parts_count + sizeof(__uint32_t) * graphs[0].nodes_num + sizeof(__uint32_t) * connections;
}

size_t  count_connections(t_graph *graphs)
{
    size_t result = 0;

    for (size_t i = 0; i < graphs[0].nodes_num; i++)
        result += graphs[0].nodes[i].connections_num;
    return (result);
}

void    save_binary(t_gsplit *info, t_graph *graphs)
{
    size_t          wrote = 0;
    t_output_data   data;
    uint16_t        tmp16;
    uint32_t        tmp32;
    const size_t    connections = count_connections(graphs);
    
    get_output_tables(&data, info, graphs, connections);
    tmp16 = htole16(data.signature);
    wrote += fwrite(&tmp16, sizeof(tmp16), 1, info->output) * sizeof(tmp16);
    tmp32 = htole32(data.file_size);
    wrote += fwrite(&tmp32, sizeof(tmp32), 1, info->output) * sizeof(tmp32);
    tmp16 = htole16(data.parts_count);
    wrote += fwrite(&tmp16, sizeof(tmp16), 1, info->output) * sizeof(tmp16);
    tmp16 = htole16(data.rows_count);
    wrote += fwrite(&tmp16, sizeof(tmp16), 1, info->output) * sizeof(tmp16);
    tmp16 = htole16(data.columns_count);
    wrote += fwrite(&tmp16, sizeof(tmp16), 1, info->output) * sizeof(tmp16);
    tmp32 = htole32(data.nodes_count);
    wrote += fwrite(&tmp32, sizeof(tmp32), 1, info->output) * sizeof(tmp32);

    for (uint32_t i = 0; i < data.nodes_count; i++)
    {
        tmp16 = htons(data.indexes_in_row[i]);
        wrote += fwrite(&tmp16, sizeof(tmp16), 1, info->output) * sizeof(tmp16);
    }
    for (uint32_t i = 0; i < (data.columns_count + 1); i++)
    {
        tmp16 = htons(data.first_nodes_indexes[i]);
        wrote += fwrite(&tmp16, sizeof(tmp16), 1, info->output) * sizeof(tmp16);
    }
    for (uint32_t i = 0; i < data.parts_count; i++)
    {
        tmp32 = htons(data.end_table[i]);
        wrote += fwrite(&tmp32, sizeof(tmp32), 1, info->output) * sizeof(tmp32);
    }
    for (uint32_t i = 0; i < graphs[0].nodes_num; i++)
    {
        tmp32 = htons(data.offset_table[i]);
        wrote += fwrite(&tmp32, sizeof(tmp32), 1, info->output) * sizeof(tmp32);
    }
    for (uint32_t i = 0; i < connections; i++)
    {
        tmp32 = htons(data.edge_table[i]);
        wrote += fwrite(&tmp32, sizeof(tmp32), 1, info->output) * sizeof(tmp32);
    }
    free_output_data(&data);
    if (wrote != data.file_size)
        printf("ERROR: the file size is not correct!!, %zu but should be %u\n", wrote, data.file_size);
}

void    save_text(t_gsplit *info, t_graph *graphs)
{
    t_output_data   data;
    const size_t  connections = count_connections(graphs);
    
    get_output_tables(&data, info, graphs, connections);
    fprintf(info->output, "%d\n%d\n%d\n%d\n", data.parts_count, data.rows_count, data.columns_count, data.nodes_count);
    for (unsigned int i = 0; i < data.nodes_count; i++)
    {
        fprintf(info->output, "%d", data.indexes_in_row[i]);
        if (i != data.nodes_count - 1)
            fprintf(info->output, ",");
    }
    fprintf(info->output, "\n");
    for (unsigned int i = 0; i < (data.columns_count + 1); i++)
    {
        fprintf(info->output, "%d", data.first_nodes_indexes[i]);
        if (i != data.columns_count)
            fprintf(info->output, ",");
    }
    fprintf(info->output, "\n"); 

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
