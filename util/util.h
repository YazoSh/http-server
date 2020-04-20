#ifndef util
#define util

struct httpreq {
	int method;
	char version[16];
	char resource[128];
};

extern int serve(int);

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

static char S_GET[] = "GET";
//TODO

extern struct httpreq *reshttp(char *);

#endif
