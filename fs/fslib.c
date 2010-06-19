#include <stdio.h>
#include <string.h>
#include <malloc.h>

const char *get_base_name(const char *path_org)
{
	char *p;
	char *path = strdup(path_org);

	p = strrchr(path, '/');
	if (!p) {
		free(path);
		return path_org;
	}
	/* the /linux/hello/ case */
	if (*(p + 1) == 0) {
		*p = 0;
		p--;
		while (*p != '/' && p >= path) {
			*p = 0;
			p--;
		}
		if (p < path)
			return NULL;
	}

	return p + 1;
}
