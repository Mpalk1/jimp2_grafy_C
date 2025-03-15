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

static void	err_free_input(t_gsplit *info, char *err_msg, char *to_free)
{
	fprintf(stderr, "Błąd: %s\n", err_msg);
	free(to_free);
	free_gsplit(info);
	exit(EXIT_FAILURE);
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
	char			*saveptr = NULL;
	char			*token = NULL;
	unsigned int	i;
	
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

void	load_graph(t_gsplit *info, t_graph *graph)
{
	char	*line;
	size_t	len;
	int		line_count;
	// int		max_node_num;

	line = NULL;
	line_count = 1;
	len = 0;
	ssize_t read; // z newlinem
	while ((read = getline(&line, &len, info->input)) != -1)
	{
		if (line[read - 1] == '\n')
			line[read - 1] = '\0';
		switch (line_count)
		{
		case 1:
			if (line[0] == '\0' || !is_uint(line))
				err_free_input(info, "Maksymalna możliwa liczba węzłów w wierszu musi być dodatnią liczbą całkowitą", line);
			// max_node_num = atoi(line);
			break ;
		case 2:
			graph->nodes_num = count_nodes(line);
			if (graph->nodes_num < 2)
				err_free_input(info, "Zbyt mało węzłów", line);
			graph->nodes = (t_node *)calloc(sizeof(t_node), graph->nodes_num);
			if (!graph->nodes)
				err_free_input(info, "Nie udało się przydzielić pamięci", line);
			save_nodes(line, graph->nodes, graph->nodes_num);
			if (!graph->nodes)
				err_free_input(info, "Nieprawidłowy indeks węzła", line);
			break ;
		case 3:
			break ;
		case 4:
			// zapisac na pozniej
			break ;
		default:
			// zapisac do line
			// linijka 5 i nastepne
			break ;
		// wykorzystac linie 5, 6, ... i linie 4 zeby zapisac polaczenia
		}
		line_count++;
	}
	free(line);
	(void)line_count;
}
