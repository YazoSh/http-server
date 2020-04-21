#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include "util.h"

int serve(int ctsock)
{
	/* Store the http request */
	int sockflags;
	sockflags = fcntl(ctsock, F_GETFL);
	sockflags |= O_NONBLOCK;
	fcntl(ctsock, F_SETFL, sockflags);

	sleep(1);
	char reqbuff[512];
	char *reqbuffp = reqbuff;
	ssize_t readsize;
	while((reqbuffp < (reqbuff + sizeof(reqbuff))) && ((readsize = read(ctsock, reqbuffp, 1)) > 0))
		reqbuffp += readsize;
	*reqbuffp = '\0';

	struct httpreq *httpreq;
	httpreq = reshttp(reqbuff);

	printf("%d %s -- %s\n", httpreq->method, httpreq->resource, httpreq->version);
	return ctsock;
}
