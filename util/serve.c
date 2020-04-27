#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include "util.h"
#include "headerlist.h"

int serve(int ctsock)
{
	/* Store the http request */
	int sockflags;
	sockflags = fcntl(ctsock, F_GETFL);
	sockflags |= O_NONBLOCK;
	fcntl(ctsock, F_SETFL, sockflags);

	char reqbuff[1024];
	char *reqbuffp = reqbuff;
	ssize_t readsize;
	while((reqbuffp < (reqbuff + sizeof(reqbuff) - 1)) && ((readsize = read(ctsock, reqbuffp, 100)) > 0))
		reqbuffp += readsize;
	*reqbuffp = '\0';

	struct httpreq *httpreq;
	if(!(httpreq = reshttp(reqbuff)))
	{
		;//TODO
	}

	/* test stuff */
	printf("%d %s -- %s\n", httpreq->method, httpreq->resource, httpreq->version);
	struct httpheader *t;
	t = httpreq->headers;
	while(t)
	{
		printf("%s: %s\n", t->name, t->content);
		t = t->next;
	}
	return ctsock;
}
