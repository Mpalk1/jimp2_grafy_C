#include "gsplit.h"

void	free_gsplit(t_gsplit *info)
{
	if (info->opts->input_name != NULL && info->input != NULL)
		fclose(info->input);
	if (info->opts->output_name != NULL && info->output != NULL)
		fclose(info->output);
}

void	free_graphs(t_graph *graphs, int graphs_num)
{
	if (graphs == NULL || graphs_num == 0)
		return ;
	for (int i = 0; i < graphs_num; i++)
	{
		for (size_t j = 0; j < graphs[i].nodes_num; j++)
			if (graphs[i].nodes[j].connections)
				free(graphs[i].nodes[j].connections);
		if (graphs[i].nodes != NULL)
			free(graphs[i].nodes);
	}
	free(graphs);
}

void	err_free_print(t_gsplit *info, char *err_msg, char *to_free, t_graph *graphs)
{
	fprintf(stderr, "Błąd: %s\n", err_msg);
	if (to_free)
		free(to_free);
	free_graphs(graphs, info->graphs_num);
	free_gsplit(info);
	exit(EXIT_FAILURE);
}

int	count_lines(FILE *file)
{
	char	buf[BUFFER_SIZE];
	int		counter;
	size_t	res;

	counter = 0;
	while (true)
	{
		res = fread(buf, 1, BUFFER_SIZE, file);
		if (ferror(file))
			return (-1);
		for (size_t i = 1; i < res; i++)
			if (buf[i - 1] == '\n' && buf[i] != '\n')
				counter++;
		if (feof(file))
			break ;
	}
	return (counter);
}

t_graph	*alloc_graphs(t_gsplit *info)
{
	t_graph	*result;
	info->graphs_num = count_lines(info->input);
	if (info->graphs_num == -1)
		err_free_print(info, strerror(errno), NULL, NULL);
	else if (info->graphs_num < 4)
		err_free_print(info, ERROR_FORMAT, NULL, NULL);
	info->graphs_num -= 3;
	result = (t_graph *)calloc(info->graphs_num, sizeof(t_graph));
	if (!result)
		err_free_print(info, ERROR_ALLOC, NULL, NULL);
	for (int i = 0; i < info->graphs_num; i++)
	{
		result[i].nodes = NULL;
		result[i].nodes_num = 0;
	}
	return (result);
}
