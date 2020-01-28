#ifndef MIMETYPE_H
#define MIMETYPE_H

struct MimeType {
	char *name;
	char *extension;
};

typedef struct MimeType MimeType;

char *get_mimetype(char *extension);

#endif