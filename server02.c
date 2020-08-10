#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>

#define SERV_PORT 6666

struct ac_info 
{
	struct sockaddr_in clitaddr;
	int confd;
};

void * do_job(void * arg)
{
	struct ac_info * ac = (struct ac_info *)arg;
	int ret;	
	char buf[1024];
	char str[128];

	while(1)
	{
		ret = read(ac->confd, buf, 1024); 
		if (ret == 0)
		{
			printf("The client %d closed\n", ac->confd);
			break;
		}
		printf("Connected to client at ip: %s, port: %d\n",
				inet_ntop(AF_INET, &(ac->clitaddr.sin_addr.s_addr), str, sizeof(str)),
				ntohs(ac->clitaddr.sin_port));
		for(int i = 0; i < ret; i++)
			buf[i] = tolower(buf[i]);
		write(STDOUT_FILENO, buf, ret);
		write(ac->confd, buf, ret);
	}
	close(ac->confd);
	return (void*)0;
}

int main(void)
{
	struct sockaddr_in servaddr, clitaddr;
	socklen_t clit_addr_len = sizeof(clitaddr);
	int lfd, cfd, i = 0;
	pthread_t th1;

	struct ac_info ac[128];
	
	lfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	bind(lfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
	listen(lfd, 128);

	printf("Accepting client connect\n");

	while(1)
	{
		cfd = accept(lfd, (struct sockaddr*)&clitaddr, &clit_addr_len);
		ac[i].confd = cfd;
		ac[i].clitaddr = clitaddr;

		pthread_create(&th1, NULL, do_job, (void *)&ac[i]);
		pthread_detach(th1);
		i++;
	}
	return 0;
}





