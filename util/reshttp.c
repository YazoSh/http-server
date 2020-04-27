#include <string.h>

#include "util.h"
#include "headerlist.h"

#define S_GET 	"GET"
#define S_POST 	"POST"

/*
   Copy a string from t to s until a NULL
   or until an isstop charecter
*/
static char *cpyline(char *s, char *req)
{
	while((*s = *req) && strncmp(req, "\r\n", 2))
		s++, req++;
	/* move req to next line */
	if(*req)
		req += 2;
	*s = '\0';
	return req;
}

static char *cpyword(char *s, char *req)
{
	while((*s = *req) && *req != ' ' && *req != ':')
		s++, req++;
	*s = '\0';
	return ++req;
}

struct httpreq *reshttp(char *req)
{
	static struct httpreq httpreq;

	char httpline[256];
	char *httplinep = httpline;

	/* Parse the request line */
	req = cpyline(httplinep, req);
	if(!strncmp(httplinep, S_GET, sizeof(S_GET) - 1))
	{
		httplinep += sizeof(S_GET);

		httpreq.method = M_GET;
		httplinep = cpyword(httpreq.resource, httplinep);
		httplinep = cpyword(httpreq.version, httplinep);
	}
	//else if(!strncmp(reqline, S_POST, sizeof(S_POST) - 1)
	//TODO
	else
		return NULL;

	/*
		process header fields and load them into a 
		linked list
		breakes when and empty line is encountered 
	*/
	char *headername;
	char *headercontent;
	httpreq.headers = NULL;

	while(*(req = cpyline(httpline, req)))
	{
		httplinep = httpline;

		headername = httpline;
		httplinep = strchr(httpline, ':');
		if(!httplinep)
			return NULL;
		*httplinep = '\0'; 

		/* skip the ':' and the space */
		headercontent = httplinep + 2;

		httpreq.headers = addheader(httpreq.headers, headername, headercontent);
	}
	// Process req body
	//TODO
	return &httpreq;
}