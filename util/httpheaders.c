#include <stdlib.h>
#include <string.h>

#include "httpheaders.h"
#include "util.h"

/*
	Add a node to the header list containing the header information

	return if no malloc failed to allocate memory,
	returns the root otherwise.
*/
struct httpheader *addheader(struct httpheader *node, char *name, char *content)
{
	if(node)
	{
		node->next = addheader(node->next, name, content);
		return node;
	}

	struct httpheader * new;
	new = malloc(sizeof(struct httpheader));
	if(!new) return NULL;

	new->name = malloc(strlen(name) + 1);
	if(!new->name) return NULL;
	strcpy(new->name, name);

	new->content = malloc(strlen(content) + 1);
	if(!new->content) return NULL;
	strcpy(new->content, content);

	new->next = NULL;

	return new;
}

/* free the httpheader list memory */
void freeheaderlist(struct httpheader *node)
{
	if(!node)
		return;
	free((void *)node->name);
	free((void *)node->content);
	freeheaderlist(node->next);
	free((void *)node);
}

/* 
	constructs an HTTP response header
 	and concatenates to to respbuffer 
 */
char *cathttpheader(char *respbuffer, char *name, char *content)
{
	strcat(respbuffer, name);
	strcat(respbuffer, ": ");
	strcat(respbuffer, content);
	strcat(respbuffer, HTTP_ENDLINE);
}
