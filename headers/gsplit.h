#ifndef GSPLIT_H
# define GSPLIT_H

# include <stdbool.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>

# define MAX_MARGIN 20 // <-- do ustalenia

# define TEXT_BOLD "\e[1m"
# define TEXT_DEFAULT "\e[m"

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

/*
Zawiera ważne zmienne
*/
typedef struct s_gsplit
{
	FILE		*input;
	FILE		*output;
	t_options	*opts;
	// bla bla bla
}				t_gsplit;

//	MEMORY.C

/*
Zwalnia pamięć z t_gsplit, zamyka otwarte pliki
*/
void			free_gsplit(t_gsplit *info);

//	OPTIONS.C

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