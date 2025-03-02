#include "gsplit.h"

int	main(int argc, char **argv)
{
	t_options	options;
	t_gsplit	info;

	options = (t_options){NULL, NULL, 2, 10, false, false};
	load_options(&options, argc, argv);
	info.opts = &options;
	open_files(&info);
	if (options.verbose)
		print_conf(info.opts);
	// wczytywanie grafu
	// start algorytmu
	// czyszczenie pamieci
	free_gsplit(&info);
	return (EXIT_SUCCESS);
}
