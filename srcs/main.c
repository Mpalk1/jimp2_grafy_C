#include "gsplit.h"

int	main(int argc, char **argv)
{
	t_options	options;
	t_gsplit	info;
	t_graph		*graphs;

	options = (t_options){NULL, NULL, 2, 10, false, false};
	load_options(&options, argc, argv);
	info.opts = &options;
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
	// czyszczenie pamieci
	free_gsplit(&info);
	free_graphs(graphs, info.graphs_num);
	return (EXIT_SUCCESS);
}
