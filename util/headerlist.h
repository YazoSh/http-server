#ifndef hlist
#define hlist

struct httpheader {
	char *name;
	char *content;
	struct httpheader *next;
};

struct httpheader *addheader(struct httpheader *, char *name, char *content);
void freeheaderlist(struct httpheader *node);

#endif
