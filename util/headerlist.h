#ifndef hlist
#define hlist

struct httpheader {
	char *name;
	char *content;
	struct httpheader *next;
};

extern struct httpheader *addheader(struct httpheader *, char *name, char *content);

extern void freeheaderlist(struct httpheader *node);

#endif
