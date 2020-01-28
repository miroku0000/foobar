#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>

#include "handlers.h"
#include "log.h"
#include "args.h"
#include "mimetype.h"

void handle_get(Client *client);

struct Handler {
	char *method;
	void (*handler)(Client *client);
};

static struct Handler g_handlers[] = {
	{"GET", handle_get},
	{NULL, NULL}
};

void handle_request(Client *client) {
	LOG_INFO("Got %s request for %s from %s:%d", client->request->method, client->request->path, client->ip, client->port);
	
	for(struct Handler *handler = g_handlers; handler->method != NULL; handler++) {
		if(strcasecmp(handler->method, client->request->method) == 0) {
			handler->handler(client);
			return;
		}
	}
	LOG_INFO("Could not find a handler for %s", client->request->method);
}

void send_directory_to_client(Client *client, int dir_fd) {
	DIR *dir = fdopendir(dir_fd);
	
	if(dir == NULL) {
		LOG_ERROR("Failed to open the dir_fd as a directory");
		send_error(client, 404);
	} else {
		struct dirent *entry = readdir(dir);
		
		send_status(client, 200);
		send_header_fmt(client, "Content-Type", "text/html");
		send_end_headers(client);
		
		dprintf(client->fd, "<html>\n<head><title>Directory Listing for %s</title></head>\n", client->request->path);
		dprintf(client->fd, "<body>\n<h2>Directory Listing for %s</h2>", client->request->path);
		dprintf(client->fd, "<hr>\n<ul>");
		
		while(entry) {
			dprintf(client->fd, "<li><a href=\"%s\">%s</a></li>", entry->d_name, entry->d_name);
			
			entry = readdir(dir);
		}
		
		dprintf(client->fd, "</ul>\n</body>\n</html>");
		
		closedir(dir);
	}
}

void send_mimetype_header(Client *client, char *path) {
	char *extension = strrchr(path, '.');
	
	if(extension[0] != '\0') {
		// lets skip over the "."
		extension++;
		
		// lets check to see if the extension has a "/" if it does then the dot is farther up the
		// file path. The file name doesn't have an extension...
		if(strchr(extension, '/') != NULL) {
			// opps have a slash, no file extension
			extension = "\0";
		}
	}
	
	send_header_fmt(client, "Content-Type", "%s", get_mimetype(extension));
}

void send_file_to_client(Client *client, char *path) {
	int file_fd = open(path, O_RDONLY | O_EXLOCK);
	
	if(file_fd < 0) {
		// oops error!
		send_error(client, 404);
	} else {
		struct stat file_info;
		
		if(fstat(file_fd, &file_info) != 0) {
			// for some reason we couldn't stat this...
			LOG_ERROR("Unable to stat path %s", path);
			send_error(client, 404);
		} else {
			if(file_info.st_mode & S_IFREG) {
				char buff[2048];
		
				send_status(client, 200);
				send_header_fmt(client, "Content-Length", "%jd", file_info.st_size);
				send_mimetype_header(client, path);
				send_end_headers(client);
				while(1) {
					int result = read(file_fd, buff, sizeof(buff));
					if(result <= 0) {
						close(file_fd);
						break;
					} else {
						write(client->fd, buff, result);
					}
				}
			} else if (file_info.st_mode & S_IFDIR) {
				send_directory_to_client(client, file_fd);
			} else {
				LOG_ERROR("Cannot serve file type at %s", path);
				send_error(client, 404);
			}
		}
		close(file_fd);
	}
}


void handle_get(Client *client) {
	char path[PATH_MAX];
	char *real_path;
	
	// first create the path
	sprintf(path, "%s/%s", g_document_root, client->request->path);
	
	// create the real path
	real_path = realpath(path, NULL);
	if(real_path == NULL) {
		LOG_ERROR("Unable to create a real path from %s", path);
		send_error(client, 404);
		return;
	}
	
	// see if this is a valid path
	if(strncmp(g_document_root, real_path, strlen(g_document_root)) != 0) {
		LOG_ERROR("Path points out of root: %s", real_path);
		free(real_path);
		send_error(client, 404);
		return;
	}
	
	// now dump the file to the client
	send_file_to_client(client, real_path);
}

