#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "httpheaders.h"

int serve(int ctsock)
{
	/* Store the http request */

	/* set sock to non blocking mode */
	int sockflags;
	sockflags = fcntl(ctsock, F_GETFL);
	sockflags |= O_NONBLOCK;
	fcntl(ctsock, F_SETFL, sockflags);

	/* buffer the http request */
	char buffer[1024];

	/* read data from client */
	char *reqbuffp = buffer;
	ssize_t readsize;
	while((reqbuffp < (buffer + sizeof(buffer))) &&
		((readsize = read(ctsock, reqbuffp, sizeof(buffer) - 1)) > 0))

		reqbuffp += readsize;
	*reqbuffp = '\0';

	struct httpreq *httpreq;
	if(!(httpreq = resreq(buffer)))
	{
		return ctsock;//TODO
	}
	
	/* Construct a response */
	struct httpresp response;
	response = constresp(httpreq);

	if(!response.response)
		return ctsock;

	/* send response */
	char *responsep = response.response;
	while(*responsep)
	{
		if((readsize = write(ctsock, responsep, strlen(responsep))) >= 0)
				responsep += readsize;
	}
	free((void *)response.response);

	/* output status massages */
	printf("%d %s -- %s\n", httpreq->method, httpreq->resource, httpreq->version);

	freeheaderlist(httpreq->headers);
	return ctsock;
}
