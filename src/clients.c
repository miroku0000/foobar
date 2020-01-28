#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <stdarg.h>

#include "clients.h"
#include "log.h"
#include "handlers.h"

/*
 * List of connected clients.
 */
Client *g_clients = NULL;

/*
 * Creates a new client structure with the FD representing the client.
 */
Client *new_client(int fd, char *ip, int port) {
	Client *client = calloc(1, sizeof(Client));
	
	client->fd = fd;
	client->ip = strdup(ip);
	client->port = port;
	
	LOG_INFO("New connection from: %s:%d", client->ip, client->port);
	
	if(g_clients == NULL) {
		g_clients = client;
	} else {
		g_clients->prev = client;
		client->next = g_clients;
		g_clients = client;
	}
	
	return client;
}

/*
 * Removes a client from the client list and frees it.
 */
void free_client(Client *client) {
	LOG_INFO("Connect closed from: %s:%d", client->ip, client->port);
	
	close(client->fd);
	if(client->request) {
		free_http_request(client->request);
		client->request = NULL;
	}
	
	if(client->next) {
		client->next->prev = client->prev;
	}
	if(client->prev) {
		client->prev->next = client->next;
	}
	if(g_clients == client) {
		g_clients = client->next;
	}
	free(client);
}

/*
 * Returns the client with the given fd.
 */
Client *lookup_client(int fd) {
	for(Client *client = g_clients; client != NULL; client = client->next) {
		if(client->fd == fd) {
			return client;
		}
	}
	return NULL;
}

/*
 * Called when the client has data to read.
 */
void handle_client(Client *client) {
	ssize_t read_len = recv(client->fd, &client->data[client->data_len], sizeof(client->data) - client->data_len, 0);
	
	if(read_len <= 0) {
		// Guess we lost this connection, free it up
		free_client(client);
	} else {
		client->data_len += read_len;
		
		// lets see if we have a header
		if(http_received_header(client->data, client->data_len)) {
			client->request = http_parse_headers(client->data, client->data_len);
			if(client->request != NULL) {
				// got a valid request so handle it...
				handle_request(client);
			} else {
				// send an invalid header response
				send_error(client, 400);
			}
			
			// done with this request
			free_client(client);
		} else if(client->data_len >= MAXIMUM_HEADER_SIZE) {
			LOG_INFO("Client excededed maximum header length: %s:%d", client->ip, client->port);
			free_client(client);
		}
		// else we wait for more data...
	}
}

/*
 * Sends the status line to the client.
 */
void send_status(Client *client, int status_code) {
	if(client->request) {
		dprintf(client->fd, "%s %d\r\n", client->request->version, status_code);
	} else {
		dprintf(client->fd, "HTTP/1.0 %d\r\n", status_code);
	}
}

/*
 * Formats a header and sends it to the client.
 */
void send_header_fmt(Client *client, char *name, char *value_format, ...) {
	va_list argp;
	
	va_start(argp, value_format);
	
	dprintf(client->fd, "%s: ", name);
	vdprintf(client->fd, value_format, argp);
	write(client->fd, "\r\n", 2);
	
	va_end(argp);
}

/*
 * Ends the header section. Call this once all headers are sent and before 
 * sending the body of the response.
 */
void send_end_headers(Client *client) {
	write(client->fd, "\r\n", 2);
}

/*
 * Sends an error and closes the header. You should send any more data after this...
 */
void send_error(Client *client, int error) {
	send_status(client, error);
	send_end_headers(client);
}

/*
 * Sends a string that has been HTTP encoded.
 */
void send_html_escape(Client *client, char *string) {
	char *string_end = string + strlen(string);
	
	while(string < string_end) {
		char *next = strpbrk(string, "&<>\"'/'");
		
		if(next == NULL) {
			write(client->fd, string, strlen(string));
			// we sent the rest in one shot, we are done
			break;
		} else {
			char *tmp;
			switch(*next) {
				case '&':
					tmp = "&amp;";
					break;
				case '<':
					tmp = "&lt";
					break;
				case '>':
					tmp = "&gt";
					break;
				case '"':
					tmp = "&quot;";
					break;
				case '\'':
					tmp = "&#x27;";
					break;
				case '/':
					tmp = "&#x2F;";
					break;
				default:
					// we shouldn't get here, just send a dot instead...
					tmp = ".";
			}
			if(next != string) {
				// output up to our special char
				write(client->fd, string, next - string);
			}
			write(client->fd, tmp, strlen(tmp));
			
			string = next + 1;
		}
	}
}
