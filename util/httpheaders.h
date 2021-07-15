#ifndef httph
#define httph

struct httpheader {
	char *name;
	char *content;
	struct httpheader *next;
};

struct httpheader *addheader(struct httpheader *, char *name, char *content);
void freeheaderlist(struct httpheader *node);
int cathttpheader(char *respbuffer, char *name, char *content);

/* http header fields */
#define CONTENT_TYPE	"Content-Type"
#define CONTENT_LENGTH	"Content-Lenght" 

/* media types */
#define TYPE_TEXT 	"text/"
#define TYPE_IMAGE 	"image/"
#define TYPE_APP 	"application/"

/* media subtypes */

#define SUP_HTML	"html"
#define SUP_CSS		"css"
#define SUP_PLAIN	"plain"
#define SUP_JSCRIPT	"javascript"

#define SUP_GIF		"gif"
#define SUP_JPEG	"jpeg"
#define SUP_PNG		"png"
#define SUP_SVG		"svg+xml"
#define SUP_ICO		"x-icon"

#endif
