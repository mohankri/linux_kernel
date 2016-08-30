#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define	NETLINK_USER	31
#define	MAX_PAYLOAD	1024

/* Not sure why when making this local variable it fails with No Buffer Space Available */

struct nlmsghdr *nlh = NULL;

void
main()
{
	struct iovec iov;
	struct sockaddr_nl src_addr, dest_addr;
	struct msghdr msg;
	/* Comment out b'cos of No Buffer Space Available */
	//struct nlmsghdr *nlh = NULL;
	int sockfd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
	if (sockfd < 0) {
		return;
	}
	memset(&src_addr, 0, sizeof(src_addr));

	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid();
	//src_addr.nl_groups = 0;

	bind(sockfd, (struct sockaddr *)&src_addr, sizeof(src_addr));


	memset(&dest_addr, 0, sizeof(dest_addr));
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0;
	dest_addr.nl_groups = 0;
	//dest_addr.nlmsg_flags = 0;	

	nlh = (struct nlmsghdr *) malloc(NLMSG_SPACE(MAX_PAYLOAD));
	memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_flags = 0;

	strcpy(NLMSG_DATA(nlh), "KM");
	printf("data %s :", NLMSG_DATA(nlh));	

	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	printf("send msg %ld \n", sendmsg(sockfd, &msg, 0));
	perror("failed:");	
	close(sockfd);
}

