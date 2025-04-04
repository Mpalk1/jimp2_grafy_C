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

	options = (t_options){NULL, NULL, 2, 10, false, false};
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
	print_graphs(graphs, info.graphs_num);
	// start algorytmu
	partition_graph(&graphs[0], options.parts, options.margin);
	print_graphs1(graphs, info.graphs_num);
	// sprawdzic czy w marginesie
	// zliczyc ilosc przeciec
	save_text(&info, graphs);
	free_gsplit(&info);
	free_graphs(graphs, info.graphs_num);
	return (EXIT_SUCCESS);
}
