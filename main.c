#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

int main(int argc, char **argv)
{
	uint16_t port;
	uint64_t tmpport;
	uint32_t inaddr;

	struct hostent *hostname;
	struct servent *service;

	char *end;

	/* Default values */
	port = 0;
	tmpport = 0;
	inaddr = INADDR_ANY;
	hostname = NULL;
	end = NULL;

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
							tmpport = strtol(*argv, &end, 0);
						else
						{
							fprintf(stderr, "Error: Option \"-p\" requires an argument\n");
							exit(1);
						}

						/* Port checking */

						/* Checks if iport is more than max port size 
						   or equal to zero 
						   or if its a valid decimal number */
						if(!tmpport || (tmpport > ~(~0<<16)) || *end)
						{
							fprintf(stderr, "Error: Invalid local port: %s\n", *argv);
							exit(1);
						}
						/* Checks if iport is used by an another process */
						if(getservbyport(port, "tcp"))
						{
							fprintf(stderr, "Error: port %s already in use\n", *argv);
							exit(1);
						}
						port = tmpport;
						/* until i figure it out */
						/* to break out of the outer loop */
						*(*argv + 1) = 0;
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
	int ltsock;
	struct sockaddr_in addr;
	struct in_addr in_addr;
	size_t size;

	if((ltsock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fputs("Cant creat socket!\n", stderr);
		exit(1);
	}

	in_addr.s_addr = inaddr;

	addr.sin_addr = in_addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	size = sizeof(struct sockaddr_in);
	
	if(bind(ltsock, (struct sockaddr *)&addr, size) < 0)
	{
		fputs("Cant bind socket to address\n", stderr);
		exit(1);
	}

	/* output ip address and port the server is listening to */
	printf("Starting server\n");
	printf("listening on %s port %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));	

	/* Setup socket to listen for connections */
	listen(ltsock, 10);

	/* Listen for connections and serve clients */
	struct sockaddr_in clientaddr;
	int csize;
	int ctsock;

	csize = sizeof(struct sockaddr_in);

	fd_set sockset;
	FD_ZERO(&sockset);
		
	while(1)
	{
		FD_SET(ltsock, &sockset);
		if(select(FD_SETSIZE, &sockset, NULL, NULL, NULL) < 0)
		{
			fputs("Error: select\n", stderr);
			exit(1);
		}

		for(int sock = 0; sock < FD_SETSIZE; sock++)
		{
			/* Connection requrst on listening socket */
			if(FD_ISSET(sock, &sockset))
			{
				if(sock == ltsock)
				{
					ctsock = accept(sock, (struct sockaddr *)&clientaddr, &csize);	
					if(ctsock < 0)
						fputs("Error: Accepting connection!\n", stderr);
					else
					{
						FD_SET(ctsock, &sockset);
						printf("Server: Connecte from %s:%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
						fflush(stdout);
					}
				}
				/* Serve an already connected socket */
				else
				{
					/* test stuff */
					char buffer[10];
					read(sock, buffer, 10);
					write(1, buffer, 10);
					FD_CLR(sock, &sockset);
					close(sock);
				}
			}
		}
	}
	close(ltsock);
}
