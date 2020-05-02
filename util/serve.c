#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include "util.h"
#include "headerlist.h"

int serve(int ctsock)
{
	/* Store the http request */

	/* set sock to non blocking mode */
	int sockflags;
	sockflags = fcntl(ctsock, F_GETFL);
	sockflags |= O_NONBLOCK;
	fcntl(ctsock, F_SETFL, sockflags);

	char buffer[1024];
	char *reqbuffp = buffer;

	/* read data from client */
	ssize_t readsize;
	while((reqbuffp < (buffer + sizeof(buffer))) && ((readsize = read(ctsock, reqbuffp, sizeof(buffer))) > 0))
		reqbuffp += readsize;
	*reqbuffp = '\0';

	struct httpreq *httpreq;
	if(!(httpreq = reshttp(buffer)))
	{
		return ctsock;//TODO
	}
	
	/* Construct a response */

	/* useless code */
	printf("%d %s -- %s\n", httpreq->method, httpreq->resource, httpreq->version);
	struct httpheader *t;
	t = httpreq->headers;
	while(t)
	{
		printf("%s: %s\n", t->name, t->content);
		t = t->next;
	}
	/*	*/

	return ctsock;
}
