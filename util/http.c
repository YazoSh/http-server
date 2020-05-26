#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

#include "util.h"
#include "headerlist.h"
#include "info.h"

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

/* selector function for scandir*/
static int selector_nothidden(const struct dirent *dir)
{
	return (dir->d_name[0] == '.')?0:1;
}

struct httpreq *resreq(char *req)
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
	FILE *resource = NULL;
	int resourcefd;

	char path[PATH_MAX] = "./";

	char header[64];
	*header = '\0';

	if(req->method == M_GET)
	{
		strcat(path, req->resource);

		resourcefd = open(path, O_RDONLY);
		if(resourcefd >= 0)
		{
			/* if resource is a directory */
			if((dresource = fdopendir(resourcefd)))
			{
				consthttpheaders(header, req->version, S_OK);

				response = malloc(1024 /* TODO super magic number */ + strlen(header) + 1);
				*response = '\0';
				responsep = response;

				strcat(response, header);

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
					strcat(response, "<a href=\"/");
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
			else if((resource = fdopen(resourcefd, "r")))
			{
				consthttpheaders(header, req->version, S_OK);

				fseek(resource, 0, SEEK_END);
				filesize = ftell(resource);
				fseek(resource, 0, SEEK_SET);

				response = malloc((sizeof(char) * filesize) + strlen(header) + 1);
				*response = '\0';
				responsep = response;

				strcat(response, header);
				responsep += strlen(response);

				int c;
				while((c = fgetc(resource)) > 0)
					*responsep++ = c;
				*responsep = '\0';
				fclose(resource);
				return response;
			}
		}
		/* if resource is not found */
		else
		{
			consthttpheaders(header, req->version, S_NOTFOUND);
	
			response = malloc((sizeof(char) * sizeof P_NOTFOUND) + strlen(header));
			*response = '\0';

			strcat(response, header);
			strcat(response, P_NOTFOUND);

			return response;
		}
	}
	//else
	//TODO
}
