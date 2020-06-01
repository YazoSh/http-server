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
#include "headerlist.h"
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

/* selector function for scandir*/
static int selector_nothidden(const struct dirent *dir)
{
	return (dir->d_name[0] == '.')?0:1;
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

static char *consthttpheaders(char *resp, char *version, char *statuscode)
{
	strcat(resp, version);
	strcat(resp, " ");
	strcat(resp, statuscode);
	strcat(resp, HTTP_ENDLINE);

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

	char statusline[64];
	*statusline = '\0';

	if(req->method == M_GET)
	{
		strcat(path, req->resource);
		resourcefd = open(path, O_RDONLY);

		if(resourcefd >= 0)
		{
			consthttpheaders(statusline, req->version, S_OK);

			/* if resource is a directory */
			errno = 0;
			if((dresource = fdopendir(resourcefd)) && errno != ENOTDIR)
			{

				response = malloc(1024 /* TODO super magic number */ + strlen(statusline) + 1);
				*response = '\0';
				responsep = response;

				strcat(response, statusline);

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
				filesize = lseek(resourcefd, 0, SEEK_END);
				lseek(resourcefd, 0, SEEK_SET);

				response = malloc((sizeof(char) * filesize) + strlen(statusline) + 1);
				*response = '\0';
				responsep = response;

				/* add response status line */
				strcat(response, statusline);
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
	consthttpheaders(statusline, req->version, S_NOTFOUND);
	
	response = malloc((sizeof(char) * sizeof P_NOTFOUND) + strlen(statusline));
	*response = '\0';

	strcat(response, statusline);
	strcat(response, P_NOTFOUND);

	return response;
	//else
	//TODO
}
