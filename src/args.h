#ifndef ARGS_H
#define ARGS_H

extern int g_bind_port;
extern char *g_document_root;

int parse_args(int argc, char **argv);

#endif