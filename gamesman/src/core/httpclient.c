#include "httpclient.h"
#include <errno.h>

/* FUNCTIONS */

/* TEST FUNCTION ONLY */

/*
int main(int argc, char *argv[])
{
	httpreq *req;
	httpres *res;
	char *body;
	int r, i;
	//char blah[] = "136.152.170.158:8080/gamesman/GamesmanServlet";
	char * url = malloc(strlen(argv[1])+1);
	
	// Read the args    
    if (argc < 2) 
    {
       fprintf(stderr,"usage %s url\n", argv[0]);
       exit(1);
    }
   
    while (1){
	memcpy(url,argv[1],strlen(argv[1])+1);
	printf("trying to resolve %s\n",url);
	req = newrequest(url);
	addheader(req, "type", "MATT");
	addheader(req, "header1", "val1");
	addheader(req, "Header2", "monkey monkey doo doo face");

	printf("%s:%d%s\n", req->hostName, req->portNum, req->path);

	body = "abcdefghijklmnopqrstuvwxyz";	

	res = post(req, body, 27);
	
	printf("%s: %s\n", "Date", getheader(res, "date"));
	printf("%s: %s\n", "Content-Length", getheader(res, "Content-Length"));
	printf("%s: %s\n", "ReturnCode", getheader(res, "ReturnCode"));
	printf("%s: %s\n", "ReturnMessage", getheader(res, "ReturnMessage"));
	
	r = res->bodyLength;
	printf("response: ");
	for (i=0; i<r; i++)
		printf("%d ", res->body[i]);
     
	printf("\ndone\n");	
	freeresponse(res);
    }
	return 0;
}
*/

/**
 * Converts an unsigned long long from host byte order to network byte order
 * depending on the endian-ness of the host system.
 * 
 * n - unsigned long long (usually a POSITION) in host's byte order
 * returns an unsigned long long in network byte order
 */
unsigned long long htonll(unsigned long long n) 
{
	short w = 0x4321;
	if ((*(char *)& w) != 0x21 )
		return n;
	else
		return (((unsigned long long)htonl(n)) << 32) + htonl(n >> 32);
}

/**
 * Returns the value of the specified header as a char array. The returned
 * value may be NULL if no header exists with the specified name.
 *
 * res - httpres struct to read header values from
 * name - name of the header for which we want the value
 * returns the value of the named header
 */
char* getheader(httpres *res, char name[])
{
	char *lname;
	char *lhname;
	char *val = NULL;
	header *currHdr;

	// Lower case the name so we can do case-insensitive comparisons
	if ((lname = malloc(strlen(name)+1)) == NULL)
	{
		fprintf(stderr,"ERROR, could not allocate memory for header name comparision\n");
		exit(1);
	}
	lcstrcpy(lname, name);
	// Loop through the headers until we find one
	currHdr = res->headers;
	while (currHdr != NULL)
	{
		if ((lhname = malloc(strlen(currHdr->name)+1)) == NULL)
		{
			fprintf(stderr,"ERROR, could not allocate memory for header name comparision\n");
			exit(1);
		}
		lcstrcpy(lhname, currHdr->name);
		if (strcmp(lhname, lname) == 0)
		{
			val = currHdr->value;
			free(lhname);
			break;
		}
		free(lhname);
		currHdr = currHdr->next;
	}
	free(lname);
	
	// Return what we've found
	return val;
}

/**
 * Copies the from string to the to string and lower cases it
 * in the process.
 *
 * to - char array to copy into
 * from - char array to copy from
 */
void lcstrcpy(char to[], char from[])
{
	int i;
	int len = strlen(from);
	for (i=0; i<len; i++)
		to[i] = tolower(from[i]);
	to[i] = '\0';
}

/**
 * Frees the specified httpres struct when no longer needed. Not
 * using this function will result in a memory leak.
 * 
 * res - httpres struct to free
 */
void freeresponse(httpres *res)
{
	header *currHdr;
	header *tmpHdr;

	// Free the malloc'd memory
	currHdr = res->headers;
	tmpHdr = NULL;
	while (currHdr != NULL)
	{
		if (tmpHdr != NULL)
			free(tmpHdr);
		tmpHdr = currHdr;
		free(currHdr->name);
		if (currHdr->value != NULL)
			free(currHdr->value);
		currHdr = currHdr->next;
	}
	if (tmpHdr != NULL)
		free(tmpHdr);
	if (res->body != NULL)
		free(res->body);
	free(res); 
	
}

/**
 * POST's the specified httpreq to it's preconfigured url with
 * all preconfigured headers and the body content (if any).
 * Returns a httpres struct representing the HTTP response data.
 *
 * req - httpreq struct to POST to it's url/addr
 * body - content for the body of the HTTP POST, can be NULL
 * bodyLength - length of the body content, can be 0 if body is NULL
 * returns httpres struct representing the HTTP response data
 */
httpres* post(httpreq *req, char body[], int bodyLength)
{
	httpres *res;
	header *currHdr;
	header *tmpHdr;
    char buffer[64];
    int sockFd;
    int n;

	// Add the content-length header
	net_itoa(bodyLength, buffer);
	addheader(req, "Content-Length", buffer);
	
	// Create a socket
	if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr,"ERROR, creating socket: ");
		connecterror(stderr);
		exit(1);
	}

	// Connect to the socket
	if (connect(sockFd, &(req->sock.res), sizeof(struct sockaddr_in)) < 0) 
	{
		fprintf(stderr,"ERROR, opening socket: ");
		connecterror(stderr);
		exit(1);
	}

	// Submit the http request   
	n = write(sockFd, "POST ", 5);
	n = write(sockFd, req->path, strlen(req->path));
	n = write(sockFd, " HTTP/1.1\r\n", 11);
	// Add the headers
	currHdr = req->headers;
	while (currHdr != NULL)
	{
		n = write(sockFd, currHdr->name, strlen(currHdr->name));
		n = write(sockFd, ": ", 2);
		n = write(sockFd, currHdr->value, strlen(currHdr->value));
		n = write(sockFd, "\r\n", 2);
		currHdr = currHdr->next;
	}
	// Add the extra line to separate headers from body
	n = write(sockFd, "\r\n", 2);
	// Add the body (if any)
	if (bodyLength > 0)
		n = write(sockFd, body, bodyLength);

	// Create the response
	if ((res = malloc(sizeof(httpres))) == NULL)
	{
		fprintf(stderr,"ERROR, could not allocate memory for http response\n");
		exit(1);
	}
	res->headers = NULL;
	res->status = NULL;
	res->body = NULL;
	res->bodyLength = 0;
	// Read the response
	readresponse(sockFd, res);
	shutdown(sockFd,2);
	// Free the malloc'd memory
	currHdr = req->headers;
	while (currHdr != NULL)
	{
		free(currHdr->name);
		free(currHdr->value);
		tmpHdr = currHdr;
		currHdr = currHdr->next;
		free(tmpHdr);
	}
	free(req->hostName);
	free(req->path);
	free(req);

	return res;	
}

/**
 * Reads the HTTP response from the specified socket and populates the specified
 * httpres struct accordingly.
 *
 * sockFd - socket file descriptor from which to read
 * res - httpres struct to populate
 */
void readresponse(int sockFd, httpres *res)
{
	header *currHdr;
	header *tmpHdr;
	char *pos;
    char buffer[512];
    char c[1];
    int p;
    int n;

	// Read each line at a time
	tmpHdr = NULL;
	while (1)
	{
		p = 0;
		while ((n = read(sockFd, c, 1)) != EOF)
		{
			if (c[0] == '\r')
				continue;
			else if (c[0] == '\n')
				break;
			else
				buffer[p++] = c[0];
		}
		buffer[p] = '\0';
		
		// Stop reading line-by-line when we encounter a blank line
		if (strlen(buffer) == 0)
			break;
		
		if (res->status == NULL)
		{
			// First line is the HTTP status line
			if ((res->status = malloc(strlen(buffer)+1)) == NULL)
			{
				fprintf(stderr,"ERROR, could not allocate memory for http response status\n");
				exit(1);
			}
			strcpy(res->status, buffer);
			
			// Parse out the status code
			pos = strtok(buffer, " ");
			if (pos != NULL)
			{
				if ((pos = strtok(NULL, " ")) != NULL)
					res->statusCode = atoi(pos);
			}
		}
		else
		{
			// Make a header for this line
			if ((currHdr = malloc(sizeof(header))) == NULL)
			{
				fprintf(stderr,"ERROR, could not allocate memory for http response header\n");
				exit(1);
			}
			currHdr->next = NULL;

			if (pos = strchr(buffer, ':'))
			{
				// Line has a ':' char
				*pos = '\0';
				p = strlen(buffer);
				if ((currHdr->name = malloc(p+1)) == NULL)
				{
					fprintf(stderr,"ERROR, could not allocate memory for header name\n");
					exit(1);
				}
				strcpy(currHdr->name, buffer);

				// Skip spaces and tabs
				*pos = ' ';
				while (p < 512 && (buffer[p] == ' ' || buffer[p] == '\t'))
				{
					p++;
					pos++;
				}

				// Now read to the end
				if ((currHdr->value = malloc(strlen(pos)+1)) == NULL)
				{
					fprintf(stderr,"ERROR, could not allocate memory for header value\n");
					exit(1);
				}			
				strcpy(currHdr->value, pos);
			}
			else
			{
				// Line had no ':' so add the whole line as just a header name, no value
				if ((currHdr->name = malloc(strlen(buffer)+1)) == NULL)
				{
					fprintf(stderr,"ERROR, could not allocate memory for header name\n");
					exit(1);
				}
				strcpy(currHdr->name, buffer);
			}
			
			// Add the header
			if (tmpHdr != NULL)
				tmpHdr->next = currHdr;
			else
				res->headers = currHdr;
			tmpHdr = currHdr;	
		}
	}
		
	// Now read any body content
	if ((pos = getheader(res, "Content-Length")) != NULL)
	{
		p = atoi(pos);
		if ((res->body = malloc(p)) == NULL)
		{
			fprintf(stderr,"ERROR, could not allocate memory for response body\n");
			exit(1);
		}
		
		res->bodyLength = read(sockFd, res->body, p);
	}
}

/**
 * Converts the specified int into a character equivalent and places the
 * value in the specified char array. Assumes the char array has enough
 * space to support the number of converted digits.
 *
 * n - int to convert
 * s - char array that will hold the result of the conversion
 */
void net_itoa(int n, char s[])
{
	int c, i, j;
	if ((c = n) < 0)
		n = -n;
	i =0;
	do
	{
		s[i++] = n % 10 + '0';
	}
	while ((n /= 10) > 0);
	if (c < 0)
		s[i++] = '-';
	s[i] = '\0';
	 
	// Now reverse
	for (i=0, j=strlen(s)-1; i<j; i++, j--)
	{
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

/**
 * Sets the type header to the specified httpreq struct. The specified
 * value is copied (by value) into the struct. Thus, changes to that 
 * character array will not have any further affect.
 *
 * req - httpreq struct to modify
 * value - value for the header t
 */
void settype(httpreq *req, char value[])
{
	addheader(req, "TYPE", value);
}

/**
 * Adds the specified header to the specified httpreq struct. The
 * specified name/value are copied (by value) into the struct. Thus,
 * changes to these character arrays will not have any further affect.
 *
 * req - httpreq struct to modify
 * name - name of the header to add
 * value - value for the header to add
 */
void addheader(httpreq *req, char name[], char value[])
{
	header *hdr;
	header *currHdr;
	
	// Create a header
	if ((hdr = malloc(sizeof(header))) == NULL)
	{
		fprintf(stderr,"ERROR, could not allocate memory for http request header\n");
		exit(1);
	}
	hdr->next = NULL;
	if ((hdr->name = malloc(strlen(name) + 1)) == NULL)
	{
		fprintf(stderr,"ERROR, could not allocate memory for http request header name\n");
		exit(1);
	}
	strcpy(hdr->name, name);
	if ((hdr->value = malloc(strlen(value) + 1)) == NULL)
	{
		fprintf(stderr,"ERROR, could not allocate memory for http request header value\n");
		exit(1);
	}
	strcpy(hdr->value, value);
	
	// Add to the httpreq
	if (req->headers == NULL)
	{
		req->headers = hdr;
	}
	else
	{
		currHdr = req->headers;		
		while (currHdr->next != NULL)
			currHdr = currHdr->next;
		currHdr->next = hdr;
	}	
}

/**
 * Creates a new httpreq to represent the http request to make. Returns
 * a pointer to this struct for later passing to the get/post function.
 * 
 * url - url the http request will use (minus the http:// prefix)
 */
httpreq* newrequest(char url[])
{
	httpreq *req;

	// Create the httpreq
	if ((req = malloc(sizeof(httpreq))) == NULL)
	{
		fprintf(stderr,"ERROR, could not allocate memory for http request\n");
		exit(1);
	}

	req->headers = NULL;

    // Parse the url and add the info to the httpreq
    parse(url, req);
    
    // Lookup the host addr and validate it
	if ((req->serverAddr = gethostbyname(req->hostName)) == NULL)
	{
		fprintf(stderr,"ERROR, no such host: %s\n", req->hostName);
		exit(1);
	}
	
	// Setup the socket address    
	memcpy(&(req->sock.req.sin_addr.s_addr), *(req->serverAddr->h_addr_list), sizeof(struct in_addr));
	req->sock.req.sin_family = AF_INET;
	req->sock.req.sin_port = htons(req->portNum);

	// Add the required headers
	addheader(req, "Host", req->hostName); // Required by HTTP 1.1
	addheader(req, "Connection", "close"); // Don't keep the conn open afterwards
	addheader(req, "User-Agent", "Gamesman/1.0"); // So we can identify ourselves
	addheader(req, "Content-Type", "application/octet-stream"); // Body content will be binary

	// Ready to go
	return req;
}

/**
 * Parses the specified url into the hostName, path, and port
 * and assigns the values into the specified httpreq struct.
 * 
 * url - url to parse
 * req - pointer to httpreq
 */
void parse(char url[], httpreq *req)
{
    char *pos1;
    char *pos2;
    int n;

	// Parse the url   
	if (pos1 = strchr(url, ':'))
	{
		// Url has a port number specified
		// Read just the host name, up to the ':'
		*pos1 = '\0';
		n = strlen(url);		
		if ((req->hostName = malloc(n+1)) == NULL)
		{
			fprintf(stderr,"ERROR, could not allocate memory for hostName\n");
			exit(1);
		}
		strcpy(req->hostName, url);
		// Move past the ':' char
		pos1++;
		if (pos2 = strchr(pos1,'/'))
		{
			// Url has a '/' char
			// Read just the port, up to the '/'
			*pos2 = '\0';
			req->portNum = atoi(pos1);
			// Now read from the '/' to the end
			*pos2 = '/';
			n = strlen(pos2);
			if ((req->path = malloc(n+1)) == NULL)
			{
				fprintf(stderr,"ERROR, could not allocate memory for path\n");
				exit(1);
			}			
			strcpy(req->path, pos2);
		}
		else
		{
			// Just host name and port, but use malloc calls
			// so we can free later as we will need to in 
			// other cases
			req->portNum = atoi(pos1);
			// Default path to '/'
			if ((req->path = malloc(1+1)) == NULL)
			{
				fprintf(stderr,"ERROR, could not allocate memory for path\n");
				exit(1);
			}
			strcpy(req->path, "/");
		}
	}
	else
	{		
		req->portNum = 80;
		if (pos1 = strchr(url, '/'))
		{
			// Url has a '/' char
			// Read just the host name, up to the '/'
			*pos1 = '\0';
			n = strlen(url);
			if ((req->hostName = malloc(n+1)) == NULL)
			{
				fprintf(stderr,"ERROR, could not allocate memory for hostName\n");
				exit(1);
			}
			strcpy(req->hostName, url);
			// Now read from the '/' to the end
			*pos1 = '/';
			n = strlen(pos1);
			if ((req->path = malloc(n+1)) == NULL)
			{
				fprintf(stderr,"ERROR, could not allocate memory for path\n");
				exit(1);
			}			
			strcpy(req->path, pos1);
		}
		else
		{
			// Just the host name, but use malloc calls
			// so we can free later as we will need to in 
			// other cases
			n = strlen(url);
			if ((req->hostName = malloc(n+1)) == NULL)
			{
				fprintf(stderr,"ERROR, could not allocate memory for hostName\n");
				exit(1);
			}
			strcpy(req->hostName, url);
			// Default path to '/'
			if ((req->path = malloc(1+1)) == NULL)
			{
				fprintf(stderr,"ERROR, could not allocate memory for hostName\n");
				exit(1);
			}
			strcpy(req->path, "/");
		}
	}
}


void connecterror(FILE *stream)
{
	if (errno == EOPNOTSUPP)
		fprintf(stream, "Operation not supported on transport endpoint.\n");	
	else if (errno == EPFNOSUPPORT)
		fprintf(stream, "Protocol family not supported.\n");	
	else if (errno == ECONNRESET)
		fprintf(stream, "Connection reset by peer.\n");	
	else if (errno == ENOBUFS)
		fprintf(stream, "No buffer space available.\n");	
	else if (errno == EAFNOSUPPORT)
		fprintf(stream, "Address family not supported by protocol family.\n");	
	else if (errno == EPROTOTYPE)
		fprintf(stream, "Protocol wrong type for socket.\n");	
	else if (errno == ENOTSOCK)
		fprintf(stream, "Socket operation on non-socket.\n");	
	else if (errno == ENOPROTOOPT)
		fprintf(stream, "Protocol not available.\n");	
	else if (errno == ESHUTDOWN)
		fprintf(stream, "Can't send after socket shutdown.\n");	
	else if (errno == ECONNREFUSED)
		fprintf(stream, "Connection refused.\n");	
	else if (errno == EADDRINUSE)
		fprintf(stream, "Address already in use.\n");	
	else if (errno == ECONNABORTED)
		fprintf(stream, "Connection aborted.\n");	
	else if (errno == ENETUNREACH)
		fprintf(stream, "Network is unreachable.\n");	
	else if (errno == ENETDOWN)
		fprintf(stream, "Network interface is not configured.\n");	
	else if (errno == ETIMEDOUT)
		fprintf(stream, "Connection timed out.\n");	
	else if (errno == EHOSTDOWN)
		fprintf(stream, "Host is down.\n");	
	else if (errno == EHOSTUNREACH)
		fprintf(stream, "Host is unreachable.\n");	
	else if (errno == EINPROGRESS)
		fprintf(stream, "Connection already in progress.\n");	
	else if (errno == EALREADY)
		fprintf(stream, "Socket already connected.\n");	
	else if (errno == EDESTADDRREQ)
		fprintf(stream, "Destination address required.\n");	
	else if (errno == EMSGSIZE)
		fprintf(stream, "Message too long.\n");	
	else if (errno == EPROTONOSUPPORT)
		fprintf(stream, "Unknown protocol.\n");	
	else if (errno == ESOCKTNOSUPPORT)
		fprintf(stream, "Socket type not supported.\n");	
	else if (errno == EADDRNOTAVAIL)
		fprintf(stream, "Address not available.\n");	
	else if (errno == ENETRESET)
		fprintf(stream, "Network interface reset.\n");	
	else if (errno == EISCONN)
		fprintf(stream, "Socket is already connected.\n");	
	else if (errno == ENOTCONN)
		fprintf(stream, "Socket is not connected.\n");		
	else if (errno == ENOTSUP)
		fprintf(stream, "Not supported.\n");	
	else if (errno == EMULTIHOP)
		fprintf(stream, "Multihop attempted.\n");				
	else if (errno == EPROTO)
		fprintf(stream, "Protocol error.\n");				
	else if (errno == ENOLINK)
		fprintf(stream, "The link has been severed.\n");										
	else if (errno == EREMOTE)
		fprintf(stream, "The object is remote.\n");										
	else if (errno == ENOSR)
		fprintf(stream, "Out of streams resources.\n");										
	else if (errno == ETIME)
		fprintf(stream, "Timer expired.\n");										
	else if (errno == ENODATA)
		fprintf(stream, "No data (for no delay io).\n");										
	else if (errno == ENOSTR)
		fprintf(stream, "Device not a stream.\n");										
	else if (errno == EIDRM)
		fprintf(stream, "Identifier removed.\n");										
	else if (errno == ENOMSG)
		fprintf(stream, "No message of desired type.\n");										
	else if (errno == EPIPE)
		fprintf(stream, "Broken pipe.\n");										
	else if (errno == EMLINK)
		fprintf(stream, "Too many links.\n");										
	else if (errno == EMFILE)
		fprintf(stream, "Too many open files.\n");										
	else if (errno == ENFILE)
		fprintf(stream, "Too many open files in system.\n");										
	else if (errno == EINVAL)
		fprintf(stream, "Invalid argument.\n");										
	else if (errno == ENODEV)
		fprintf(stream, "No such device.\n");										
	else if (errno == EXDEV)
		fprintf(stream, "Cross-device link.\n");										
	else if (errno == ENOTBLK)
		fprintf(stream, "Block device required.\n");										
	else if (errno == EFAULT)
		fprintf(stream, "Bad address.\n");										
	else if (errno == EACCES)
		fprintf(stream, "Permission denied.\n");										
	else if (errno == EBADF)
		fprintf(stream, "Bad file number.\n");										
	else if (errno == ENXIO)
		fprintf(stream, "No such device or address.\n");										
	else if (errno == EIO)
		fprintf(stream, "I/O error.\n");										
	else if (errno == EINTR)
		fprintf(stream, "Interrupted system call.\n");										
	else if (errno == EPERM)
		fprintf(stream, "Not super-user.\n");										
	else
		fprintf(stream, "Unknown error %d.\n", errno);																					
}

