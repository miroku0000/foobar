#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>

struct HTTP_Header {
	struct HTTP_Header *next;
	
	char *name;
	char *value;
};

typedef struct HTTP_Header HTTP_Header;

struct HTTP_Request {
	char *method;
	char *path;
	char *query;
	char *version;
	
	HTTP_Header *headers;
};

typedef struct HTTP_Request HTTP_Request;

/*
 * Checks to see if we have received an entire header.
 */
int http_received_header(char *data, size_t data_len);

/*
 * Parses the data into an HTTP_Request. This does not duplicate the strings and
 * the data buffer will be modified and the caller must maintain data in memory untill
 * this HTTP_Request is freed.
 */
HTTP_Request* http_parse_headers(char *data, size_t data_len);

/*
 * Frees the HTTP_Request structure. Does not free the pointers to the strings.
 */
void free_http_request(HTTP_Request *request);

#endif
