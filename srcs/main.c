#include "gsplit.h"

int	count_all_nodes(t_graph *graphs, int graphs_num)// todo do przeniesienia
{
	int	result = 0;

	for (int i = 0; i < graphs_num; i++)
		result += graphs[i].nodes_num;
	return (result);
}

int	main(int argc, char **argv)
{
	t_options	options;
	t_gsplit	info;
	t_graph		*graphs;

	options = (t_options){NULL, NULL, 2, 10, false, false, false};
	load_options(&options, argc, argv);
	info.opts = &options;
	info.node_index = NULL;
	info.nodes_in_row = NULL;
	open_files(&info);
	if (options.verbose)
		print_conf(info.opts);
	graphs = alloc_graphs(&info);
	rewind(info.input);
	load_graphs(&info, graphs);
	if (!partition_graph(&graphs[0], options.parts, options.margin, info.opts))
	{
		free_gsplit(&info);
		free_graphs(graphs, info.graphs_num);
		return (EXIT_FAILURE);
	}
	// print_graphs(graphs, 1);
	if (info.opts->verbose)
		print_graphs1(graphs, info.graphs_num);
	if (info.opts->verbose)
		printf("Graf został podzielony, zapisywanie grafu.\n");
	if (info.opts->binary)
		save_binary(&info, graphs);
	else
		save_text(&info, graphs);
	if (info.opts->verbose)
		printf("Graf zapisany, wychodzenie z programu.\n");
	free_gsplit(&info);
	free_graphs(graphs, info.graphs_num);
	return (EXIT_SUCCESS);
}
