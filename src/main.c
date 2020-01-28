#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/select.h>
#include <arpa/inet.h>

#include "args.h"
#include "clients.h"

#define LISTEN_COUNT 10

int create_server_sock() {
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	int value;
	struct sockaddr_in addr;
	
	if(sock < 0) {
		perror("Failed to create the server socket");
		return -1;
	}
	
	value = 1;
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value)) < 0) {
		perror("Failed to set SO_REUSEADDR");
		return -1;
	}
	
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(g_bind_port);
	if(bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		perror("Failed to bind");
		return -1;
	}
	
	if(listen(sock, LISTEN_COUNT) < 0) {
		perror("Failed to listen");
		return -1;
	}
	
	return sock;
}

void accept_client(int server_sock) {
	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);
	int fd = accept(server_sock, (struct sockaddr*)&addr, &addr_len);
	
	if(fd < 0) {
		perror("Failed to accept");
		return;
	}
	
	new_client(fd, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
}

int run_select_loop(int server_sock) {
	fd_set fds;
	int highest_fd;
	
	while(1) {
		FD_ZERO(&fds);
		
		// we need to listen to the server socket
		FD_SET(server_sock, &fds);
		highest_fd = server_sock;
		
		// and we need to listen to every client fd
		for(Client *client = g_clients; client != NULL; client = client->next) {
			FD_SET(client->fd, &fds);
			if(client->fd > highest_fd) {
				highest_fd = client->fd;
			}
		}
		
		if(select(highest_fd + 1, &fds, NULL, NULL, NULL) < 0) {
			perror("Select failed");
			return -1;
		}
		
		for(int fd = 0; fd < highest_fd + 1; fd++) {
			if(FD_ISSET(fd, &fds)) {
				if(fd == server_sock) {
					accept_client(server_sock);
				} else {
					Client *client = lookup_client(fd);
					
					if(client != NULL) {
						handle_client(client);
					}
				}
			}
		}
	}
	
	return 0;
}

int main(int argc, char **argv) {
	int server_socket;
	
	if(parse_args(argc, argv) < 0) {
		return -1;
	}
	
	server_socket = create_server_sock();
	if(server_socket < 0) {
		return -1;
	}
	
	run_select_loop(server_socket);
	
	return 0;
}
