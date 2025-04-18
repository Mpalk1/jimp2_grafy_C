#include "gsplit.h"

void	print_help(void)
{
	printf(TEXT_BOLD "Użycie: ./gsplit [opcje]\n");
	printf("Opcje:\n" TEXT_DEFAULT);
	printf("  -h, --help\t\tWyświetl pomoc\n");
	printf("  -f, --force\t\tWymuś utworzenie grafu\n");
	printf("  -v, --verbose\t\tWłącz tryb szczegółowy\n");
	printf("  -i, --input\t\tPlik wejściowy (domyślnie stdin)\n");
	printf("  -o, --output\t\tPlik wyjściowy (domyślnie stdout)\n");
	printf("  -p, --parts\t\tLiczba części (domyślnie 2)\n");
	printf("  -m, --margin\t\tMargines procentowy (domyślnie 10%%)\n");
	printf("  -b, --binary\t\tZapis w formacie binarnym\n");
	exit(EXIT_SUCCESS);
}

void	print_conf(t_options *options)
{
	printf("Tryb szczegółowy włączony\n");
	printf("Plik wejściowy: %s\n", options->input_name ? options->input_name : "stdin");
	printf("Plik wyjściowy: %s\n", options->output_name ? options->output_name : "stdout");
	printf("Liczba części: %d\n", options->parts);
	printf("Margines: %d%%\n", options->margin);
	printf("Format binarny: %s\n", options->binary ? "Tak" : "Nie");
	printf("Wymuszać utworzenie grafu: %s\n", options->force ? "Tak" : "Nie");
}
