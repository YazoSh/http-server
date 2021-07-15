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
	else if(!(strcmp(resource, ".jpeg") && strcmp(resource, ".jpg")))
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

static char *consthttp_statusline(char *resp, char *version, char *statuscode)
{
	/* add status line */
	*resp = '\0';
	strcat(resp, version);
	strcat(resp, " ");
	strcat(resp, statuscode);
	strcat(resp, HTTP_ENDLINE);


	strcat(resp, HTTP_ENDLINE);

	return resp;
}

struct httpresp constresp(struct httpreq *req)
{
	struct httpresp response;
	response.response = NULL;
	response.size = 0;

	char *responsep = NULL;

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
				consthttp_statusline(respheader, req->version, S_OK);
				cathttpheader(respheader, CONTENT_TYPE, TYPE_TEXT SUP_HTML);

				//TODO
				// could cause a heap overflow
				response.response = malloc(1024 /* TODO super magic number*/ + strlen(respheader) + 1);
				responsep = response.response;
				*responsep = '\0';

				strcat(response.response, respheader);
				responsep += strlen(response.response);

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
					strcat(response.response, "<div>");
					strcat(response.response, "<a href=\"");
					if(strlen(req->resource) > 1)
						strcat(response.response, req->resource);
					strcat(response.response, "/");
					strcat(response.response, dir->d_name);
					strcat(response.response, "\">");
					strcat(response.response, dir->d_name);
					strcat(response.response, "</a>");
					strcat(response.response, "</div>");

					free((void *)dir);
				}
				
				// This calculates the lenght of the http body
				response.size = strlen(response.response) + response.response - responsep;

				free(pdirs);
				closedir(dresource);
				return response;
			}
			/* if resource was a normal file */
			else
			{

				response.size = lseek(resourcefd, 0, SEEK_END);
				lseek(resourcefd, 0, SEEK_SET);
				
				consthttp_statusline(respheader, req->version, S_OK);
				cathttpheader(respheader, CONTENT_TYPE, getMIMEtype(req->resource));

				char s_length[100];
				snprintf(s_length, sizeof(s_length), "%d", response.size);

				cathttpheader(respheader, CONTENT_LENGTH, s_length);

				response.response = malloc((sizeof(char) * response.size) + strlen(respheader) + 1);
				*response.response = '\0';
				responsep = response.response;

				/* add header */
				strcat(response.response, respheader);
				responsep += strlen(response.response);

				/* add response body */
				while(responsep != (responsep += read(resourcefd, responsep, response.size)))
						;
				*responsep = '\0';

				close(resourcefd);
				return response;
			}
		}
	}
	/* if resource is not found */

	// and if http req is not GET
	//TODO
	consthttp_statusline(respheader, req->version, S_NOTFOUND);
	cathttpheader(respheader, CONTENT_TYPE, TYPE_TEXT SUP_HTML);
	
	responsep = response.response = malloc((sizeof(char) * sizeof P_NOTFOUND) + strlen(respheader));
	*response.response = '\0';

	strcat(response.response, respheader);
	responsep += strlen(response.response);

	strcat(response.response, P_NOTFOUND);
	
	// calculate the size of the http body
	response.size = strlen(response.response) + response.response - responsep;

	return response;
}
