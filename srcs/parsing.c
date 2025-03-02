#include "gsplit.h"

void	open_files(t_gsplit *info)
{
	if (info->opts->input_name == NULL)
		info->input = stdin;
	else
	{
		info->input = fopen(info->opts->input_name, "r");
		if (!info->input)
		{
			perror("Plik wejściowy");
			exit(EXIT_FAILURE);
		}
	}
	if (info->opts->output_name == NULL)
		info->output = stdout;
	else
	{
		info->output = fopen(info->opts->output_name, "w");
		if (!info->output)
		{
			if (info->opts->input_name != NULL)
				fclose(info->input);
			perror("Plik wyjściowy");
			exit(EXIT_FAILURE);
		}
	}
}
