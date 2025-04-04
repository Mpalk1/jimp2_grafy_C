#include "gsplit.h"

unsigned int	strcountch(char *line, char to_find)
{
	unsigned int	result;

	result = 1;
	for (int i = 0; line[i]; i++)
		if (line[i] == to_find)
			result++;
	return (result);
}

void    strreplace(char *str, char from, char to)
{
    int i = 0;
    while (str[i])
    {
        if (str[i] == from)
            str[i] = to;
        i++;
    }
}
