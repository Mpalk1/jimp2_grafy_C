#include "gsplit.h"

void	free_gsplit(t_gsplit *info)
{
	if (info->opts->input_name != NULL)
		fclose(info->input);
	if (info->opts->output_name != NULL)
		fclose(info->output);
}
