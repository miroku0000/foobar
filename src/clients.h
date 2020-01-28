#ifndef CLIENTS_H
#define CLIENTS_H

#include "http.h"

#define MAXIMUM_HEADER_SIZE	(16 * 1024)

struct Client {
	struct Client *next;
	struct Client *prev;
	
	int fd;
	char *ip;
	int port;
	
	char data[MAXIMUM_HEADER_SIZE];
	size_t data_len;
	
	HTTP_Request *request;
};

typedef struct Client Client;

/*
 * List of connected clients.
 */
extern Client *g_clients;

/*
 * Creates a new client structure with the FD representing the client.
 */
Client *new_client(int fd, char *ip, int port);

/*
 * Removes a client from the client list and frees it.
 */
void free_client(Client *client);

/*
 * Returns the client with the given fd.
 */
Client *lookup_client(int fd);

/*
 * Called when the client has data to read.
 */
void handle_client(Client *client);

/*
 * Sends the status line to the client.
 */
void send_status(Client *client, int status_code);

/*
 * Formats a header and sends it to the client.
 */
void send_header_fmt(Client *client, char *name, char *value_format, ...);

/*
 * Ends the header section. Call this once all headers are sent and before 
 * sending the body of the response.
 */
void send_end_headers(Client *client);

/*
 * Sends an error and closes the header. You should send any more data after this...
 */
void send_error(Client *client, int error);

/*
 * Sends a string that has been HTML escaped.
 */
void send_html_escape(Client *client, char *string);

#endif
