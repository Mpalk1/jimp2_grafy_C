#ifndef GSPLIT_H
# define GSPLIT_H

# include <stdbool.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <assert.h>

# define MAX_MARGIN 20 // <-- do ustalenia

# define TEXT_BOLD "\e[1m"
# define TEXT_DEFAULT "\e[m"

# define DELETED_NODE -42

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
	struct s_node	*connections;
	size_t			connections_num;
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
	// bla bla bla
}				t_gsplit;

//	DEBUG.C - DELETE LATER

void    print_graph(t_graph *graph);

//	MEMORY.C

/*
Zwalnia pamięć z t_gsplit, zamyka otwarte pliki
*/
void			free_gsplit(t_gsplit *info);

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
void			load_graph(t_gsplit *info, t_graph *graph);

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

#endif