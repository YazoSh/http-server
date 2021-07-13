#ifndef util
#define util

#include <limits.h>

#include "httpheaders.h"

struct httpreq {
	int method;
	char version[16];
	char resource[PATH_MAX];
	struct httpheader *headers;
};

int serve(int);

/* HTTP constant suff */

/*
   request methods 
   only GET is implemented
*/

#define M_NONE		0
#define M_GET		1
#define M_HEAD 		2
#define M_POST 		3
#define M_PUT 		4
#define M_DELETE 	5
#define M_TRACE 	6
#define M_OPTIONS	7
#define M_CONNECT	8
#define M_PATCH		9

/* stuff */
#define HTTP_ENDLINE "\r\n"

/* Status codes */
#define S_OK 		"200 OK"
#define S_NOTFOUND	"404 Not Found"
//TODO

/* HTTP method's strings */
#define S_GET "GET"
#define S_POST "POST"

struct httpreq *resreq(char *);
char *constresp(struct httpreq *req);

#endif
