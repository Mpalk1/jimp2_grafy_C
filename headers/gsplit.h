#ifndef GSPLIT_H
# define GSPLIT_H

# include <stdbool.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <assert.h>
# include <errno.h>
# include <limits.h>

# define MAX_MARGIN 20 // <-- do ustalenia

# define TEXT_BOLD "\e[1m"
# define TEXT_DEFAULT "\e[m"

# define DELETED_NODE -42

# define BUFFER_SIZE 1024

# define ERROR_ALLOC "Nie udało się przydzielić pamięci"
# define ERROR_FORMAT "Nieprawidłowy format pliku wejściowego"
# define ERROR_MAX_NODES_NUM "Maksymalna możliwa liczba węzłów w wierszu musi być dodatnią liczbą całkowitą"
# define ERROR_NOT_ENOGH_NODES "Zbyt mało węzłów"
# define ERROR_INVALID_INDEX "Nieprawidłowy indeks"
# define ERROR_CONNECTION_DUPLICATE "Powiązanie węzłów zduplikowane"
# define ERROR_INVALID_PARTS "Nieprawidłowa liczba oczekiwanych grafów"
# define ERROR_MARGIN_NOT_UINT "Margines musi być w formacie dodatniej liczby całkowitej"
# define ERROR_INVALID_EXTENSION "Nieprawidłowe rozszerzenie pliku - oczekiwano .csrrg"
# define ERROR_INVALID_NAME "Nieprawidłowa nazwa pliku - plik musi posiadać nazwę"

/*
Zawiera konfiguracje z opcji
*/
typedef struct s_options
{
	char		*input_name;
	char		*output_name;
	int			parts;
	short		margin;
	bool		verbose;
	bool		binary;
}				t_options;

typedef struct s_node
{
	struct s_node	**connections;
	size_t			connections_num;
	int partition; 
}				t_node;

typedef struct s_graph
{
	t_node		*nodes;
	size_t		nodes_num;
}				t_graph;

/*
Zawiera ważne zmienne
*/
typedef struct s_gsplit
{
	FILE		*input;
	FILE		*output;
	t_options	*opts;
	t_graph		graph;
	int			graphs_num;
	// bla bla bla
}				t_gsplit;

//	DEBUG.C - DELETE LATER

void    print_graphs(t_graph *graph, int graphs_num);

//	MEMORY.C

/*
Zwalnia pamięć z t_gsplit, zamyka otwarte pliki
*/
void			free_gsplit(t_gsplit *info);
void	free_graphs(t_graph *graphs, int graphs_num);
void	err_free_print(t_gsplit *info, char *err_msg, char *to_free, t_graph *graphs);
void	err_print(char *err_msg);
t_graph	*alloc_graphs(t_gsplit *info);

/*
Zwalnia pamięć z t_graph
*/
void	free_graph(t_graph *graph);

//	OPTIONS.C

/*
Sprawdza czy liczba to dodatnia liczba całkowita
*/
bool	is_uint(char *str);

/*
Wczytuje opcje, sprawdza ich poprawność
Może wyjść z programu przy błędnych argumentach
*/
void			load_options(t_options *opts, int argc, char **argv);

//	PARSING.C

/*
Otwiera pliki wejściowe i wyjściowe
Może wyjść z programu przy błędach
*/
void			open_files(t_gsplit *info);
void			load_graphs(t_gsplit *info, t_graph *graphs);

//	PRINT.C

/*
Wypisuje możliwe opcje dla opcji -h (--help)
Wychodzi z programu
*/
void			print_help(void);

/*
Wypisuje konfiguracje dla opcji -v (--verbose)
*/
void			print_conf(t_options *options);

typedef struct s_move {
    size_t node_index;
    int gain;
    int target_part;
} t_move;
void print_graphs1(t_graph *graphs, int graphs_num);

void initialize_partitions(t_graph *graph, int num_parts);
void calculate_partition_sizes(t_graph *graph, int num_parts, size_t sizes[]);
int compute_gain(t_graph *graph, size_t node_idx, int new_part);
void optimize_partitions(t_graph *graph, int num_parts, int margin);
void balance_partitions(t_graph *graph, int num_parts, int margin);
void partition_graph(t_graph *graph, int num_parts, int margin);
#endif