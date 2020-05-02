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

	/* buffer for everything */
	char buffer[1024];

	/* read data from client */
	char *reqbuffp = buffer;
	ssize_t readsize;
	while((reqbuffp < (buffer + sizeof(buffer))) && ((readsize = read(ctsock, reqbuffp, sizeof(buffer))) > 0))
		reqbuffp += readsize;
	*reqbuffp = '\0';

	struct httpreq *httpreq;
	if(!(httpreq = resreq(buffer)))
	{
		return ctsock;//TODO
	}
	
	/* Construct a response */
	char *response;
	response = constresp(httpreq);

	/* send response */
	while(*response)
		write(ctsock, response++, 1);

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
