#include "gsplit.h"

int	main(int argc, char **argv)
{
	t_options	options;
	t_gsplit	info;
	t_graph		graph;

	graph.nodes = NULL;
	graph.nodes_num = 0;
	options = (t_options){NULL, NULL, 2, 10, false, false};
	load_options(&options, argc, argv);
	info.opts = &options;
	open_files(&info);
	if (options.verbose)
		print_conf(info.opts);
	load_graph(&info, &graph);
	print_graph(&graph);
	// start algorytmu
	// czyszczenie pamieci
	free_gsplit(&info);
	free_graph(&graph);
	return (EXIT_SUCCESS);
}
