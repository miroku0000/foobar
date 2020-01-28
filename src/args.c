#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int g_bind_port = 80;
char *g_document_root = NULL;

void print_usage(char *name) {
	printf("Usage: %s -p <port> -r <document_root>\n", name);
}

int parse_args(int argc, char **argv) {
	int ch;
	char *doc_root = NULL;
	
	while((ch = getopt(argc, argv, "p:r:h?")) != -1) {
		switch(ch) {
			case 'p':
				g_bind_port = atoi(optarg);
				break;
			case 'r':
				doc_root = strdup(optarg);
				break;
			default:
				print_usage(argv[0]);
				return -1;
			}
	}
	
	// lets resolve the document root to ensure we can check paths later on
	char *real_root = realpath(doc_root == NULL ? "." : doc_root, NULL);
	
	// we are done with this arg
	if(doc_root) {
		free(doc_root);
	}
	
	// make sure we successfully resolved the real path of the document root
	if(real_root == NULL) {
		perror("Failed to lookup the document root");
		return -1;
	}
	
	g_document_root = real_root;
	
	return 0;
}
