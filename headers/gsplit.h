#ifndef GSPLIT_H
# define GSPLIT_H

# include <assert.h>
# include <errno.h>
# include <limits.h>
# include <stdbool.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <time.h>
# include <unistd.h> // ssize_t
# include <endian.h>

# define MAX_MARGIN 100 // <-- do ustalenia

# define TEXT_BOLD "\e[1m"
# define TEXT_DEFAULT "\e[m"

# define BUFFER_SIZE 1024

# define OPTIMIZE_MAX_ITER 10000

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
# define ERROR_MARGIN_EXCEEDED "Przekroczono wartość marginesu - program nie jest w stanie podzielić grafu z takimi parametrami"
#define ERROR_DISCONNECTED_PARTITIONS "Podgrafy nie są spójne - program nie jest w stanie podzielić grafu z takimi parametrami"

/*
Zawiera konfiguracje z opcji
*/
typedef struct s_options
{
	char			*input_name;
	char			*output_name;
	int				parts;
	short			margin;
	bool			verbose;
	bool			binary;
	bool			force;
}					t_options;

typedef struct s_node
{
	struct s_node	**connections;
	size_t			connections_num;
	int partition; // numer podgrafu do ktorego nalezy wezel
}					t_node;

typedef struct s_graph
{
	t_node			*nodes;
	size_t			nodes_num;
}					t_graph;

/*
Zawiera ważne zmienne
*/
typedef struct s_gsplit
{
	FILE			*input;
	FILE			*output;
	t_options		*opts;
	int				graphs_num;
	int				max_node_num;
	char			*nodes_in_row;
	char			*node_index;
}					t_gsplit;

typedef struct s_move
{
	size_t			node_index;
	int				gain;
	int				target_part;
}					t_move;

typedef struct s_output_data
{
	__uint16_t		signature;
	__uint32_t		file_size;
	__uint16_t		parts_count;
	__uint16_t		rows_count;
	__uint16_t		columns_count;
	__uint32_t		nodes_count;
	__uint16_t		*indexes_in_row;
	__uint16_t		*first_nodes_indexes;
	__uint32_t		*end_table;
	__uint32_t		*offset_table;
	__uint32_t		*edge_table;

}				t_output_data;

//	DEBUG.C - DELETE LATER

void				print_graphs(t_graph *graph, int graphs_num);

//	MEMORY.C

/*
Zwalnia pamięć z t_gsplit, zamyka otwarte pliki
*/
void				free_gsplit(t_gsplit *info);
void				free_graphs(t_graph *graphs, int graphs_num);
void				err_free_print(t_gsplit *info, char *err_msg, char *to_free,
						t_graph *graphs);
void				err_print(char *err_msg);
t_graph				*alloc_graphs(t_gsplit *info);
bool    			allocate_output(t_output_data *data, size_t end_size, size_t off_size, size_t edge_size);
void    			free_output_data(t_output_data *data);

/*
Zwalnia pamięć z t_graph
*/
void				free_graph(t_graph *graph);

//	MISC.C

unsigned int	strcountch(char *line, char to_find);
void    strreplace(char *str, char from, char to);


//	OPTIONS.C

/*
Sprawdza czy liczba to dodatnia liczba całkowita
*/
bool				is_uint(char *str);

//	OUTPUT.C

void    save_binary(t_gsplit *info, t_graph *graphs);
void    save_text(t_gsplit *info, t_graph *graphs);


/*
Wczytuje opcje, sprawdza ich poprawność
Może wyjść z programu przy błędnych argumentach
*/
void				load_options(t_options *opts, int argc, char **argv);

//	PARSING.C

/*
Otwiera pliki wejściowe i wyjściowe
Może wyjść z programu przy błędach
*/
void				open_files(t_gsplit *info);
void				load_graphs(t_gsplit *info, t_graph *graphs);

//	PRINT.C

/*
Wypisuje możliwe opcje dla opcji -h (--help)
Wychodzi z programu
*/
void				print_help(void);

/*
Wypisuje konfiguracje dla opcji -v (--verbose)
*/
void				print_conf(t_options *options);

void				print_graphs1(t_graph *graphs, int graphs_num);

//	SPLIT.C

bool				partition_graph(t_graph *graph, int num_parts, int margin, t_options *opts);

#endif