#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#define MAXLINE 8192
#define SERV_PORT 8000

struct s_info
{
	struct sockaddr_in cliaddr;
	int connfd;
};

void* do_work(void* arg)
{
	int n, i;
	struct s_info * ts = (struct s_info *)arg;
	char buf[MAXLINE];
	char str[INET_ADDRSTRLEN];

	while(1)
	{
		n = read(ts->connfd, buf, MAXLINE);
		if(n == 0)
		{
			printf("The client %d closed...\n", ts->connfd);
			break;
		}
		printf("Received from %s at Port %d\n",
				inet_ntop(AF_INET, &(ts->cliaddr.sin_addr.s_addr), str, sizeof(str)),
				ntohs(ts->cliaddr.sin_port));
		for(i = 0; i < n; i++)
			buf[i] = toupper(buf[i]);

		write(STDOUT_FILENO, buf, n);
		write(ts->connfd, buf, n);
	}
	close(ts->connfd);

	return (void * )0;
}

int main(void)
{
	struct sockaddr_in servaddr, cliaddr;
	socklen_t cliaddr_len = sizeof(cliaddr);
	int lfd, cfd;
	pthread_t tid;

	struct s_info ts[256];
	int i = 0;

	lfd = socket(AF_INET, SOCK_STREAM, 0);

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(lfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
	listen(lfd, 128);

	printf("Accepting client connect...\n");

	while(1)
	{
		cfd = accept(lfd, (struct sockaddr*)&cliaddr, &cliaddr_len);
		ts[i].cliaddr = cliaddr;
		ts[i].connfd = cfd;

		pthread_create(&tid, NULL, do_work, (void *)&ts[i]);
		pthread_detach(tid);
		i++;
	}
	return 0;
} 

