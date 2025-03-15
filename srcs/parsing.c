#include "gsplit.h"

void	open_files(t_gsplit *info)
{
	if (info->opts->input_name == NULL)
		info->input = stdin;
	else
	{
		info->input = fopen(info->opts->input_name, "r");
		if (!info->input)
		{
			perror("Plik wejściowy");
			exit(EXIT_FAILURE);
		}
	}
	if (info->opts->output_name == NULL)
		info->output = stdout;
	else
	{
		info->output = fopen(info->opts->output_name, "w");
		if (!info->output)
		{
			if (info->opts->input_name != NULL)
				fclose(info->input);
			perror("Plik wyjściowy");
			exit(EXIT_FAILURE);
		}
	}
}

static unsigned int	count_nodes(char *line)
{
	unsigned int	result;

	result = 1;
	for (int i = 0; line[i]; i++)
		if (line[i] == ';')
			result++;
	return (result);
}

static void	save_nodes(char *line, t_node *nodes, unsigned int nodes_num)
{
	char			*saveptr;
	char			*token;
	unsigned int	i;

	saveptr = NULL;
	token = NULL;
	token = strtok_r(line, ";", &saveptr);
	for (i = 0; i < nodes_num && token != NULL; i++)
	{
		if (token[0] == '\0' || !is_uint(token))
		{
			free(nodes);
			nodes = NULL;
		}
		nodes[i].connections = NULL;
		nodes[i].connections_num = 0;
		token = strtok_r(NULL, ";", &saveptr);
	}
	assert(token == NULL);
	assert(i == nodes_num);
}

static char	*init_connections_num(char *indexes, char *connections, t_graph *graphs)
{
	char			*saveptr;
	char			*connect_saveptr;
	char			*connect;
	char			*token;
	int				begin_i;
	int				end_i;
	size_t			node_i;

	if (!connections)
		return (free(indexes), ERROR_ALLOC);
	if (!indexes)
		return (free(connections), ERROR_ALLOC);
	saveptr = NULL;
	connect_saveptr = NULL;
	token = NULL;
	token = strtok_r(indexes, ";", &saveptr);
	if (!token)
		return (free(indexes), free(connections), ERROR_FORMAT);
	begin_i = atoi(token);
	while (true)
	{
		token = strtok_r(NULL, ";", &saveptr);
		if (!connect_saveptr)
			connect = strtok_r(connections, ";", &connect_saveptr);
		else
			connect = strtok_r(NULL, ";", &connect_saveptr);
		if (!connect)
			return (free(indexes), free(connections), ERROR_FORMAT);
		if (!token)
			end_i = INT_MAX;
		else
			end_i = atoi(token);
		node_i = atoi(connect);
		if (!connect || !is_uint(connect) || graphs[0].nodes_num <= node_i)
			return (free(indexes), free(connections), ERROR_INVALID_INDEX);
		for (int i = 0; i < end_i - begin_i - 1 && connect != NULL; i++)
		{
			connect = strtok_r(NULL, ";", &connect_saveptr);
			if (!connect && end_i != INT_MAX)
				return (free(indexes), free(connections), ERROR_FORMAT);
			else if (!connect)
				break ;
			if (!is_uint(connect) || (size_t)atoi(connect) >= graphs[0].nodes_num)
				return (free(indexes), free(connections), ERROR_INVALID_INDEX);
			graphs[0].nodes[atoi(connect)].connections_num++;
			graphs[0].nodes[node_i].connections_num++;
		}
		begin_i = end_i;
		if (end_i == INT_MAX)
			break ;
	}
	free(indexes);
	free(connections);
	return (NULL);
}

static bool	append_conn(t_graph *graphs, int graph_i, int node_i, t_node *node)
{
	size_t	i;

	for (i = 0; i < graphs[graph_i].nodes[node_i].connections_num; i++)
	{
		if (!graphs[graph_i].nodes[node_i].connections[i])
			break ;
		if (graphs[graph_i].nodes[node_i].connections[i] == node)
			return (false);
	}
	graphs[graph_i].nodes[node_i].connections[i] = node;
	return (true);
}

static bool	append_connections(char *indexes, char *connections, t_graph *graphs)
{
	char			*connect;
	char			*token;
	int				begin_i;
	int				end_i;
	size_t			node_i;

	token = NULL;
	token = strtok_r(indexes, ";", &indexes);
	begin_i = atoi(token);
	while (true)
	{
		token = strtok_r(NULL, ";", &indexes);
		if (!connections)
			connect = strtok_r(connections, ";", &connections);
		else
			connect = strtok_r(NULL, ";", &connections);
		if (!connect)
			break ;
		if (!token)
			end_i = INT_MAX;
		else
			end_i = atoi(token);
		node_i = atoi(connect);
		for (int i = 0; i < end_i - begin_i - 1 && connect != NULL; i++)
		{
			connect = strtok_r(NULL, ";", &connections);
			if (!connect)
				break ;
			if (!append_conn(graphs, 0, node_i, &graphs[0].nodes[atoi(connect)]) || !append_conn(graphs, 0, atoi(connect), &graphs[0].nodes[node_i]))
				return (false);
		}
		begin_i = end_i;
		if (end_i == INT_MAX)
			break ;
	}
	return (true);
}

static bool	alloc_connections(t_graph *graphs, int graphs_num)
{
	for (int i = 0; i < graphs_num; i++)
    {
        for (size_t j = 0; j < graphs[i].nodes_num; j++)
        {
            graphs[i].nodes[j].connections = (t_node **)calloc(graphs[i].nodes[j].connections_num, sizeof(t_node *));
			if (!graphs[i].nodes[j].connections)
				return (false);
        }
    }
	return (true);
}

void	load_graphs(t_gsplit *info, t_graph *graphs)
{
	char	*line;
	char	*errorstr;
	char	*connections;
	size_t	len;
	int		line_count;

	// int		max_node_num;
	line = NULL;
	connections = NULL;
	line_count = 1;
	len = 0;
	ssize_t read;
	while ((read = getline(&line, &len, info->input)) != -1)
	{
		if (line[read - 1] == '\n')
			line[read - 1] = '\0';
		switch (line_count)
		{
		case 1:
			if (line[0] == '\0' || !is_uint(line))
				err_free_print(info, ERROR_MAX_NODES_NUM, line, graphs);
			// max_node_num = atoi(line);
			break ;
		case 2:
			graphs[0].nodes_num = count_nodes(line);
			if (graphs[0].nodes_num < 2)
				err_free_print(info, ERROR_NOT_ENOGH_NODES, line, graphs);
			graphs[0].nodes = (t_node *)calloc(sizeof(t_node),
					graphs[0].nodes_num);
			if (!graphs[0].nodes)
				err_free_print(info, ERROR_ALLOC, line,
					graphs);
			save_nodes(line, graphs[0].nodes, graphs[0].nodes_num);
			if (!graphs[0].nodes)
				err_free_print(info, ERROR_INVALID_INDEX, line, graphs);
			break ;
		case 3:
			break ;
		case 4:
			connections = strdup(line);
			if (!connections)
				err_free_print(info, ERROR_ALLOC, line, graphs);
			break ;
		default:
			errorstr = init_connections_num(strdup(line), strdup(connections), graphs);
			if (errorstr)
			{
				if (connections)
					free(connections);
				err_free_print(info, errorstr, line, graphs);
			}
			if (!alloc_connections(graphs, info->graphs_num))
			{
				if (connections)
					free(connections);
				err_free_print(info, ERROR_ALLOC, line, graphs);
			}
			if (!append_connections(line, connections, graphs))
			{
				if (connections)
					free(connections);
				err_free_print(info, ERROR_CONNECTION_DUPLICATE, line, graphs);
			}
			break ;
			// todo dodac wsparcie kilku grafow w pliku
		}
		line_count++;
	}
	if (connections)
		free(connections);
	if (line)
		free(line);
	if (line_count < 6)
		err_free_print(info, ERROR_FORMAT, NULL, graphs);
}
