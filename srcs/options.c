#include "gsplit.h"
#include <getopt.h>

/* 
-i, --input <plik> - Plik wejściowy
-o, --output <plik> - Plik wyjściowy
-p, --parts <liczba> - Liczba oczekiwanych grafów (domyślnie: 2)
-m, --margin <procent> - Dopuszczalna różnica liczby wierzchołków (domyślnie: 10%)
-b, --binary - Zapis jako plik binarny
-v, --verbose - Więcej szczegółów
-h, --help - Wyświetlenie pomocy
 */

/* struct option {
	const char	*name;
	int			has_arg;
	int			*flag;
	int			val;
}; */

bool	is_uint(char *str)
{
	int	i = 0;

	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (false);
		i++;
	}
	return (true);
}

void	load_options(t_options *opts, int argc, char **argv)
{
	static struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"force", no_argument, 0, 'f'},
		{"verbose", no_argument, 0, 'v'},
		{"input", required_argument, 0, 'i'},
		{"output", required_argument, 0, 'o'},
		{"parts", required_argument, 0, 'p'},
		{"margin", required_argument, 0, 'm'},
		{"binary", no_argument, 0, 'b'},
		{0, 0, 0, 0}
	};
	int opt;

	while ((opt = getopt_long(argc, argv, "hfvi:o:p:m:b", long_options, NULL)) != -1) {
		switch (opt) {
			case 'h':
				print_help();
				break;
			case 'f':
				opts->force = true;
				break;
			case 'v':
				opts->verbose = true;
				break;
			case 'i':
				opts->input_name = optarg;
				if (strlen(opts->input_name) < 7 && !strcmp(opts->input_name, ".csrrg"))
					err_print(ERROR_INVALID_NAME);
				else if (strlen(opts->input_name) < 7)
					err_print(ERROR_INVALID_EXTENSION);
				else if (strcmp(opts->input_name + strlen(opts->input_name) - 6, ".csrrg"))
					err_print(ERROR_INVALID_EXTENSION);
				break;
			case 'o':
				opts->output_name = optarg;
				break;
			case 'p':
				opts->parts = atoi(optarg);
				if (!is_uint(optarg) || opts->parts < 2)
					err_print(ERROR_INVALID_PARTS);
				break;
			case 'm':
				if (!is_uint(optarg))
					err_print(ERROR_MARGIN_NOT_UINT);
				opts->margin = atoi(optarg);
				if (opts->margin < 0 || opts->margin > MAX_MARGIN)
				{
					fprintf(stderr, "Błąd: Margines musi być między 0%% a %i%%\n", MAX_MARGIN);
					exit(EXIT_FAILURE);
				}
				break;
			case 'b':
				opts->binary = true;
				break;
			default:
				exit(EXIT_FAILURE);
		}
	}
}
