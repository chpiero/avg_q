#include "vogl.h"
#include <stdlib.h>
#include <stdio.h>

/*
 * vallocate
 *
 *	Allocate some memory, barfing if malloc returns NULL.
 */
char *
vallocate(unsigned int size)
{
	char	*p, buf[60];

	if ((p = (char *)malloc(size)) == (char *)0) {
		sprintf(buf,"vallocate: request for %d bytes returned NULL", size);
		verror(buf);
	}

	return (p);
}
