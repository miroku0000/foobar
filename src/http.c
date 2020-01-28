#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "http.h"
#include "log.h"
#include "str_util.h"

int parse_request_line(HTTP_Request *request, char *line);
HTTP_Header* parse_header_line(char *line);
int url_decode(char *string);

int http_received_header(char *data, size_t data_len) {
	return strnstr(data, "\r\n\r\n", data_len) || strnstr(data, "\n\n", data_len);
}

HTTP_Request* http_parse_headers(char *data, size_t data_len) {
	char *data_end = data + data_len;
	char *line;
	HTTP_Request *request = calloc(1, sizeof(HTTP_Request));
	
	// first pull off the request line
	line = get_line(&data, data_end);	// BUG: should reject requests that start with whitespace
	if(line && parse_request_line(request, line) != 0) {
		free_http_request(request);
		return NULL;
	}
	
	// then loop until we have read in all the headers
	while(1) {
		HTTP_Header *header;
		
		line = get_line(&data, data_end);
		if(line == NULL || strlen(line) == 0) {
			break;
		}
		header = parse_header_line(line);
		if(header == NULL) {
			free_http_request(request);
			return NULL;
		}
		header->next = request->headers;
		request->headers = header;
	}
	
	return request;
}

void free_http_request(HTTP_Request *request) {
	HTTP_Header *header = request->headers;
	while(header) {
		HTTP_Header *tmp = header;
		header = header->next;
		
		free(tmp);
	}
}

int parse_request_line(HTTP_Request *request, char *line) {	
	// method is the first token
	request->method = strsep(&line, " \t");
	if(request->method == NULL) {
		LOG_ERROR("Request line does not include a method");
		return -1;
	}
	
	// then we have the path
	request->path = strsep(&line, " \t");
	if(request->path == NULL) {
		LOG_ERROR("Request line does not include a path");
		return -1;
	}
	
	// do the url decoding
	LOG_DEBUG("Request before URL Decoding: %s", request->path);
	if(url_decode(request->path) != 0) {
		LOG_ERROR("Request line contains invalid URL decoding");
		return -1;
	}
	
	// lets see if we have a query...
	request->query = strchr(request->path, '?');
	if(request->query) {
		request->query[0] = '\0';
		request->query++;
	}
	
	// read the version
	request->version = strsep(&line, " \t");
	if(request->version == NULL) {
		LOG_ERROR("Request line does not include a version");
		return -1;
	}
	
	// we should have no more chars here
	if(line != NULL) {
		LOG_ERROR("Request line has junk at the end");
		return -1;
	}
	
	return 0;
}

HTTP_Header* parse_header_line(char *line) {
	char *value = strchr(line, ':');
	HTTP_Header *header;
	
	if(value == NULL) {
		LOG_DEBUG("Invalid header, missing value");
		return NULL;
	}
	
	value[0] = '\0';
	value++;
	
	// skip over any whitespace at the start of the value
	value += strspn(value, " \t");
	
	// trim any whitespace at the end
	char *tmp = strpbrk(value, " \t");
	if(tmp) {
		*tmp = '\0';
	}
	
	header = calloc(1, sizeof(HTTP_Header));
	header->name = line;
	header->value = value;
	
	return header;
}

int url_decode(char *string) {
	char *str = string;
	char *dst = string;
	char tmp[3] = {0};
	
	while(*str) {
		if(*str == '%') {
			str++;
			tmp[0] = *str++;
			if(!ishexnumber(tmp[0])) {
				return -1;
			}
			tmp[1] = *str++;
			if(!ishexnumber(tmp[1])) {
				return -1;
			}
			*dst++ = strtol(tmp, NULL, 16);
		} else {
			*dst++ = *str++;
		}
	}
	*dst = '\0';
	
	return 0;
}