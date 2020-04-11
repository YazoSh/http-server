#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
	short int port;
	uint32_t inaddr;

	struct hostent *hostname;
	struct servent *service;

	/* Default values */
	port = 0;
	inaddr = INADDR_ANY;
	hostname = NULL;

	/* Process command line options */
	while(--argc)
	{
		if((*++argv)[0] != '-')
			if(!hostname)
			{
				hostname = gethostbyname2(*argv, AF_INET);
				if(!hostname)
				{
					fprintf(stderr, "Error: Couldn\'t resolve hostname \"%s\"\n", *argv);
					exit(1);
				}
				memcpy(&inaddr, (hostname->h_addr_list)[0], sizeof inaddr);
			}
			else
			{
				fprintf(stderr, "Error: Invalid Argument: %s\n", *argv);
				exit(1);
			}
		else
			while(*++argv[0])
				switch(*argv[0])
				{
					case  'p':
						if(--argc && *++argv)
							port = atoi(*argv);
						else
						{
							fprintf(stderr, "Error: Option \"-p\" requires an argument");
							exit(1);
						}

						/* Port checking */
						if(!port)
						{
							fprintf(stderr, "Error: Invalid local port: %s\n", *argv);
							exit(1);
						}
						if(getservbyport(port, "tcp"))
						{
							fprintf(stderr, "Error: port %s already in use\n", *argv);
							exit(1);
						}
						break;
					default:
						fprintf(stderr, "Error: Invalid Argument: -%c\n", *argv[0]);
						exit(1);
				}
	}

	/* Set a port if one is not given in program arguments */
	if(!port)
	{
		port = IPPORT_USERRESERVED;
		while((service = getservbyport(port, "tcp")))
			port++;
	}

	/* Creat and bind a socket */
	int tsock;
	struct sockaddr_in addr;
	struct in_addr in_addr;
	size_t size;

	tsock = socket(AF_INET, SOCK_STREAM, 0);

	in_addr.s_addr = inaddr;
	addr.sin_addr = in_addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	size = sizeof(struct sockaddr_in);
	
	bind(tsock, (struct sockaddr *)&addr, size);

	printf("%d , %d\n", inaddr, port);
}
