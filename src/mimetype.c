#include <stdlib.h>
#include <string.h>

#include "mimetype.h"


static MimeType g_mimetypes[] = {
	{"text/plain", "txt"},
	{"text/plain", "c"},
	{"text/plain", "h"},
	{"text/html", "htm"},
	{"text/css", "css"},
	{"image/png", "png"},
	{"image/jpeg", "jpg"},
	{"image/gif", "gif"},
	{"application/javascript", "js"},
	{"application/xml", "xml"},
	{"application/zip", "zip"},
	{"application/pdf", "pdf"},
	{NULL, NULL}
};

static char *g_default_mimetype = "application/octet-stream";

char *get_mimetype(char *extension) {
	for(MimeType *type = &g_mimetypes[0]; type->name; type++) {
		if(strcasecmp(type->extension, extension) == 0) {
			return type->name;
		}
	}
	return g_default_mimetype;
}
