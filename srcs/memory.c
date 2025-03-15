#include "gsplit.h"

void	free_gsplit(t_gsplit *info)
{
	if (info->opts->input_name != NULL)
		fclose(info->input);
	if (info->opts->output_name != NULL)
		fclose(info->output);
}

void	free_graph(t_graph *graph)
{
	if (graph->nodes != NULL)
		free(graph->nodes);
}
