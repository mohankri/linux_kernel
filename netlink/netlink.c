#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netlink.h>
#include <net/net_namespace.h>

typedef struct netlink_priv {
	struct sock *nl_sk;
} netlink_priv_t;

static netlink_priv_t 	netln;

static void
netlink_recv_msg(struct sk_buff *skb)
{
	//int size;
	int pid;
	struct nlmsghdr *nlh;
	printk("recv message .... \n");
	nlh = (struct nlmsghdr *)skb->data;
	printk("Payload data %s\n", (char *)NLMSG_DATA(nlh));
	pid = nlh->nlmsg_pid;
	return;
}

int
netlink_init_module(void)
{
	struct netlink_kernel_cfg cfg = {
		.input = netlink_recv_msg,	 
	};

	netln.nl_sk = netlink_kernel_create(&init_net, 31, &cfg);
	if (netln.nl_sk == NULL) {
		printk("failed to create netlink...\n");
		return (-1);
	}
	printk("netlink kernel create..\n");
	return (0);
}

void
netlink_exit_module(void)
{
	printk("netlink exit ..\n");
	netlink_kernel_release(netln.nl_sk);
}

module_init(netlink_init_module);
module_exit(netlink_exit_module);
MODULE_AUTHOR("Krishna Mohan <mohankri@gmail.com");
MODULE_LICENSE("GPL");
