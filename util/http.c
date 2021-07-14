#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

#include "util.h"
#include "httpheaders.h"
#include "info.h"

/*
   Copy a string from t to s until a NULL
   or until an isstop charecter
*/
static char *cpynline(char *s, char *req, int n)
{
	while(n-- && (*s = *req) && strncmp(req, HTTP_ENDLINE, sizeof(HTTP_ENDLINE) - 1))
		s++, req++;
	/* move req to next line */
	if(*req)
		req += sizeof(HTTP_ENDLINE) - 1;
	*s = '\0';
	return req;
}

/* copy at most n charecters from req to s */
static char *cpynword(char *s, char *req, int n)
{
	if(n == 0) n--;

	while(n-- && (*s = *req) && *req != ' ' && *req != ':')
		s++, req++;
	*s = '\0';
	return ++req;
}

struct httpreq *resreq(char *req)
{
	static struct httpreq httpreq;

	char httpline[1024];
	char *httplinep = httpline;

	/* Parse the request line */
	req = cpynline(httpline, req, sizeof(httpline));
	if(!strncmp(httplinep, S_GET, sizeof(S_GET) - 1))
	{
		httplinep += sizeof(S_GET);

		httpreq.method = M_GET;
		httplinep = cpynword(httpreq.resource, httplinep, PATH_MAX);
		httplinep = cpynword(httpreq.version, httplinep, sizeof(httpreq.version));
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

	while(*(req = cpynline(httpline, req, sizeof(httpline))))
	{
		httplinep = httpline;

		headername = httpline;
		httplinep = strchr(httpline, ':') + 1;
		if(!httplinep)
			return NULL;
		*httplinep = '\0'; 

		/* skip the ':' and the space */
		headercontent = httplinep;
		if(isspace(*headercontent)) headercontent++;

		httpreq.headers = addheader(httpreq.headers, headername, headercontent);
	}
	// Process req body
	//TODO
	return &httpreq;
}

/* selector function for scandir*/
static int selector_nothidden(const struct dirent *dir)
{
	return (dir->d_name[0] == '.')?0:1;
}

/* 
	returnes the media type of a file stored in a staticley allocated array 
	uses file extentions to determin file type
	//TODO use file signatures
*/
static char *getMIMEtype(char *resource)
{
	static char media[64];
	*media = '\0';

	resource = strrchr(resource, '.');
	if(!resource)
	{
		strcat(media, TYPE_TEXT);
		strcat(media, SUP_PLAIN);
	}
	else if(!strcmp(resource, ".html"))
	{
		strcat(media, TYPE_TEXT);
		strcat(media, SUP_HTML);
	}
	else if(!strcmp(resource, ".css"))
	{
		strcat(media, TYPE_TEXT);
		strcat(media, SUP_CSS);
	}
	else if(!strcmp(resource, ".js"))
	{
		strcat(media, TYPE_APP);
		strcat(media, SUP_JSCRIPT);
	}
	else if(!strcmp(resource, ".gif"))
	{
		strcat(media, TYPE_IMAGE);
		strcat(media, SUP_GIF);
	}
	else if(!strcmp(resource, ".jpeg"))
	{
		strcat(media, TYPE_IMAGE);
		strcat(media, SUP_JPEG);
	}
	else if(!strcmp(resource, ".png"))
	{
		strcat(media, TYPE_IMAGE);
		strcat(media, SUP_PNG);
	}
	else if(!strcmp(resource, ".svg"))
	{
		strcat(media, TYPE_IMAGE);
		strcat(media, SUP_SVG);
	}
	else if(!strcmp(resource, ".ico"))
	{
		strcat(media, TYPE_IMAGE);
		strcat(media, SUP_ICO);
	}

	return media;
}

static char *consthttpheader(char *resp, char *version, char *statuscode
		,char *conttype)
{
	/* add status line */
	strcat(resp, version);
	strcat(resp, " ");
	strcat(resp, statuscode);
	strcat(resp, HTTP_ENDLINE);

	/* add repsonse headers */

	// content type header
	cathttpheader(resp, CONTENT_TYPE, conttype);
	//TODO

	strcat(resp, HTTP_ENDLINE);
	return resp;
}

char *constresp(struct httpreq *req)
{
	char *response = NULL;
	char *responsep = NULL;
	size_t filesize;

	DIR *dresource = NULL;
	int resourcefd;

	char path[PATH_MAX] = "./";

	char respheader[1024];
	*respheader = '\0';

	if(req->method == M_GET)
	{
		strcat(path, req->resource);
		resourcefd = open(path, O_RDONLY);

		if(resourcefd >= 0)
		{

			/* if resource is a directory */
			errno = 0;
			if((dresource = fdopendir(resourcefd)) && errno != ENOTDIR)
			{
				consthttpheader(respheader, req->version, S_OK, TYPE_TEXT SUP_HTML);

				response = malloc(1024 /* TODO super magic number */ + strlen(respheader) + 1);
				*response = '\0';
				responsep = response;

				strcat(response, respheader);

				/* create hyperlinks for all files in a dirctory */
				struct dirent *dir;
				struct dirent **dirs;
				void *pdirs;
				int dircount;

				dircount = scandir(path, &dirs, selector_nothidden, alphasort);
				pdirs = dirs;
				while(dircount--)
				{
					dir = *(dirs++);

					/* create the html */
					strcat(response, "<div>");
					strcat(response, "<a href=\"");
					if(strlen(req->resource) > 1)
						strcat(response, req->resource);
					strcat(response, "/");
					strcat(response, dir->d_name);
					strcat(response, "\">");
					strcat(response, dir->d_name);
					strcat(response, "</a>");
					strcat(response, "</div>");

					free((void *)dir);
				}
				free(pdirs);
				closedir(dresource);
				return response;
			}
			/* if resource was a normal file */
			else
			{
				consthttpheader(respheader, req->version, S_OK, getMIMEtype(req->resource));

				filesize = lseek(resourcefd, 0, SEEK_END);
				lseek(resourcefd, 0, SEEK_SET);

				response = malloc((sizeof(char) * filesize) + strlen(respheader) + 1);
				*response = '\0';
				responsep = response;

				/* add header */
				strcat(response, respheader);
				responsep += strlen(response);

				/* add response body */
				int readsize;
				while((readsize = read(resourcefd, responsep, filesize)))
					responsep += readsize;
				*responsep = '\0';
				close(resourcefd);
				return response;
			}
		}
	}
	/* if resource is not found */

	// and if http req is not GET
	//TODO
	consthttpheader(respheader, req->version, S_NOTFOUND, TYPE_TEXT SUP_HTML);
	
	response = malloc((sizeof(char) * sizeof P_NOTFOUND) + strlen(respheader));
	*response = '\0';

	strcat(response, respheader);
	strcat(response, P_NOTFOUND);

	return response;
}
