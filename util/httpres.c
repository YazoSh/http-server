#include <string.h>

#include "util.h"

static int ispace(char c)
{
	return (c == ' ' || c == '\t');
}

static int islinebreak(char c)
{
	return (c == '\n');
}

/*
   Copy a string from t to s until a NULL
   or until an isstop charecter
*/
static char *fstrcpy(char *s, char *t, int (*isstop)(char))
{
	while(!isstop(*t) && (*s = *t))
		t++, s++;
	return t;
}

struct httpreq *reshttp(char *req)
{
	static struct httpreq httpreq;

	char reqline[100];
	char *reqlinep = reqline;

	while(*req)
	{
		req = fstrcpy(reqline, req, islinebreak);
		if(!strncmp(reqline, S_GET, sizeof(S_GET) - 1))
		{
			reqlinep += sizeof(S_GET);

			httpreq.method = M_GET;
			reqlinep = fstrcpy(httpreq.resource, reqlinep, ispace);
			reqlinep = fstrcpy(httpreq.version, reqlinep, islinebreak);
		}
		else
		{
			httpreq.method = 69;
			break;
		}
	}
	return &httpreq;
}
