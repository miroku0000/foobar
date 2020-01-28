#include <string.h>

char *get_line(char **data, char *end_ptr) {
	char *curr = *data;
	char *start;
	
	// first skip any whitespace
	for(curr = *data; curr < end_ptr && strchr(" \t", *curr) != NULL; curr++);
	
	// save it for the return
	start = curr;
	
	// then scan until we see a newline
	for(;curr < end_ptr && *curr && *curr != '\n'; curr++);
	
	// replace the \n with a null term
	*curr = '\0';
	
	// check the previous to see if it is a \r
	if(curr[-1] == '\r') {
		curr[-1] = '\0';
	}
	
	// update to our current position
	*data = curr + 1;
	
	return start;
}
