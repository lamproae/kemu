#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kprobes.h>
#include <linux/kallsyms.h>
#include <net/ip.h>
#include <net/arp.h>
#include <linux/inet.h>

static unsigned int counter = 0;

static int packet_filter(struct sk_buff *skb)
{
    struct iphdr *iph;
    u32 sip, dip;
    u32 protocol;
    iph = ip_hdr(skb);
    sip = iph->saddr;
    dip = iph->daddr;
    protocol = iph->protocol;

    if (protocol == IPPROTO_UDP || protocol == IPPROTO_TCP || protocol == IPPROTO_RAW)
        return 1;

    return 0;
}

static long netif_rx_handler(struct sk_buff *skb)
{
    printk(KERN_EMERG "[%s]\n", __func__);
    jprobe_return();
    return 0;
}

static struct jprobe jprobe_netif_rx =
{
    .entry = netif_rx_handler,
    .kp = {
        .symbol_name = "netif_rx",
    },
};

static long dev_queue_xmit_handler(struct sk_buff *skb)
{
    if (!packet_filter(skb)) {
        printk(KERN_EMERG "[%s]\n", __func__);
    }
    jprobe_return();
    return 0;
}

static struct jprobe jprobe_dev_queue_xmit =
{
    .entry = dev_queue_xmit_handler,
    .kp = {
        .symbol_name = "dev_queue_xmit",
    },
};

int ip_rcv_handler(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev)
{
    struct iphdr *iph;
    u32 sip, dip;
    u32 protocol;
    iph = ip_hdr(skb);
    sip = iph->saddr;
    dip = iph->daddr;
    protocol = iph->protocol;

    if (!packet_filter(skb)) {
        printk("Recived packet from: %pI4 to %pI4 on interface: %s protocol: 0x%x\n", &sip, &dip, dev->name, protocol);
    }
    jprobe_return();
}

static struct jprobe jprobe_ip_rcv =
{
    .entry = ip_rcv_handler,
    .kp = {
        .symbol_name = "ip_rcv",
    },
};

int arp_rcv_handler(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev)
{
    struct arphdr *arp;
    unsigned char *arp_ptr;
    __be32 sip, tip;

    printk(KERN_EMERG "[%s]\n", __func__);
    arp = arp_hdr(skb);
    arp_ptr = (unsigned char *)(arp + 1);
    arp_ptr += dev->addr_len;
    memcpy(&sip, arp_ptr, 4);
    arp_ptr += 4;
    arp_ptr += dev->addr_len;
    memcpy(&tip, arp_ptr, 4);
    if (arp->ar_op == htons(ARPOP_REPLY))
    {
        printk(KERN_EMERG "Received ARP Reply from %pI4 for %pI4!\n", &sip, &tip);
    }
    else if (arp->ar_op == htons(ARPOP_REQUEST))
    {
        printk(KERN_EMERG "Received ARP Request from %pI4 for %pI4!\n", &sip, &tip);
    }

    jprobe_return();
}

static struct jprobe jprobe_arp_rcv =
{
    .entry = arp_rcv_handler,
    .kp = {
        .symbol_name = "arp_rcv",
    },
};

void arp_send_handler(int type, int ptype, __be32 dest_ip,
        struct net_device *dev, __be32 src_ip,
        const unsigned char *dest_hw, const unsigned char *src_hw,
        const unsigned char *target_hw)
{
    printk(KERN_EMERG "[%s]\n", __func__);
    if (type == htons(ARPOP_REPLY))
    {
        printk(KERN_EMERG "Send ARP Replay to %pI4 for %pI4 from interface %s\n", &dest_ip, &src_ip, dev->name);
    }
    else if (type == htons(ARPOP_REQUEST))
    {
        printk(KERN_EMERG "Send ARP Request for %pI4 from interface %s\n", &dest_ip, dev->name);
    }
    jprobe_return();
}

static struct jprobe jprobe_arp_send =
{
    .entry = arp_send_handler,
    .kp = {
        .symbol_name = "arp_send",
    },
};

static void arp_solicit_handler(struct neighbour *neigh, struct sk_buff *skb)
{
    printk(KERN_EMERG "[%s]\n", __func__);
    jprobe_return();
}

static struct jprobe jprobe_arp_solicit =
{
    .entry = arp_solicit_handler,
    .kp = {
        .symbol_name = "arp_solicit",
    },
};

int neigh_update_handler(struct neighbor *neigh, const u8 *lladdr, u8 new, u32 flags)
{
    printk(KERN_EMERG "[%s]\n", __func__);
    jprobe_return();
}

static struct jprobe jprobe_neigh_update =
{
    .entry = neigh_update_handler,
    .kp = {
        .symbol_name = "neigh_update",
    },
};

int __neigh_event_send_handler(struct neighbour *neigh, struct sk_buff *skb)
{
    printk(KERN_EMERG "[%s]\n", __func__);
    jprobe_return();
}

static struct jprobe jprobe_neigh_event_send =
{
    .entry = __neigh_event_send_handler,
    .kp = {
        .symbol_name = "__neigh_event_send",
    },
};

static struct neighbour *neigh_alloc_handler(struct neigh_table *tbl, struct net_device *dev)
{
    printk(KERN_EMERG "[%s]\n", __func__);
    jprobe_return();
}

static struct jprobe jprobe_neigh_alloc =
{
    .entry = neigh_alloc_handler,
    .kp = {
        .symbol_name = "neigh_alloc",
    },
};

struct neighbour *__neigh_create_handler(struct neigh_table *tbl, const void *pkey,
        struct net_device *dev, bool want_ref)
{
    printk(KERN_EMERG "[%s]\n", __func__);
    jprobe_return();
}

static struct jprobe jprobe_neigh_create =
{
    .entry = __neigh_create_handler,
    .kp = {
        .symbol_name = "__neigh_create",
    },
};

static void neigh_periodic_work_handler(struct work_struct *work)
{
    printk(KERN_EMERG "[%s]\n", __func__);
    jprobe_return();
}

static struct jprobe jprobe_neigh_periodic_work =
{
    .entry = neigh_periodic_work_handler,
    .kp = {
        .symbol_name = "neigh_periodic_work",
    },
};

struct neighbour *neigh_event_ns_handler(struct neigh_table *tbl,
        u8 *lladdr, void *saddr,
        struct net_device *dev)
{
    printk(KERN_EMERG "[%s]\n", __func__);
    jprobe_return();
}

static struct jprobe jprobe_neigh_event_ns =
{
    .entry = neigh_event_ns_handler,
    .kp = {
        .symbol_name = "neigh_event_ns",
    },
};

struct neighbour *neigh_lookup_handler(struct neigh_table *tbl, const void *pkey,
        struct net_device *dev)
{
    printk(KERN_EMERG "[%s]\n", __func__);
    jprobe_return();
}

static struct jprobe jprobe_neigh_lookup =
{
    .entry = neigh_lookup_handler,
    .kp = {
        .symbol_name = "neigh_lookup",
    },
};

static void neigh_timer_handler_handler(unsigned long arg)
{
    printk(KERN_EMERG "[%s]\n", __func__);
    jprobe_return();
}

static struct jprobe jprobe_neigh_timer_handler =
{
    .entry = neigh_timer_handler_handler,
    .kp = {
        .symbol_name = "neigh_timer_handler",
    },
};

int netif_receive_skb_handler(struct sk_buff *skb)
{
    if (!packet_filter(skb))
    {
        printk(KERN_EMERG "[%s]\n", __func__);
    }
    jprobe_return();
}

static struct jprobe jprobe_netif_receive_skb =
{
    .entry = netif_receive_skb_handler,
    .kp = {
        .symbol_name = "netif_receive_skb",
    },
};

int ip_route_input_noref_handler(struct sk_buff *skb, __be32 daddr, __be32 saddr,
        u8 tos, struct net_device *dev)
{

    if (!packet_filter(skb))
    {
        printk(KERN_EMERG "[%s]\n", __func__);
    }
    jprobe_return();
}

static struct jprobe jprobe_ip_route_input_noref =
{
    .entry = ip_route_input_noref_handler,
    .kp = {
        .symbol_name = "ip_route_input_noref",
    },
};

static int ip_route_input_slow_handler(struct sk_buff *skb, __be32 daddr, __be32 saddr,
        u8 tos, struct net_device *dev)
{
    printk(KERN_EMERG "[%s]\n", __func__);
    jprobe_return();
}

static struct jprobe jprobe_ip_route_input_slow =
{
    .entry = ip_route_input_slow_handler,
    .kp = {
        .symbol_name = "ip_route_input_slow",
    },
};

static int ip_mkroute_input_handler(struct sk_buff *skb,
        struct fib_result *res,
        const struct flowi4 *fl4,
        struct in_device *in_dev,
        __be32 daddr, __be32 saddr, u32 tos)
{
    if (!packet_filter(skb))
    {
        printk(KERN_EMERG "[%s]\n", __func__);
    }
    jprobe_return();
}

static struct jprobe jprobe_ip_mkroute_input =
{
    .entry = ip_mkroute_input_handler,
    .kp = {
        .symbol_name = "ip_mkroute_input",
    },
};

static int __mkroute_input_handler(struct sk_buff *skb,
        const struct fib_result *res,
        struct in_device *in_dev,
        __be32 daddr, __be32 saddr, u32 tos)
{
    if (!packet_filter(skb))
    {
        printk(KERN_EMERG "[%s]\n", __func__);
    }
    jprobe_return();
}

static struct jprobe mkroute_input =
{
    .entry = __mkroute_input_handler,
    .kp = {
        .symbol_name = "__mkroute_input",
    },
};

static struct fib_nh_exception *find_exception_handler(struct fib_nh *nh, __be32 daddr)
{
    printk(KERN_EMERG "[%s]\n", __func__);
    jprobe_return();
}

static struct jprobe jprobe_find_exception =
{
    .entry = find_exception_handler,
    .kp = {
        .symbol_name = "find_exception",
    },
};

struct rtable *rt_dst_alloc_handler(struct net_device *dev,
        unsigned int flags, u16 type,
        bool nopolicy, bool noxfrm, bool will_cache)
{
    printk(KERN_EMERG "[%s]\n", __func__);
    jprobe_return();
}

static struct jprobe jprobe_rt_dst_alloc =
{
    .entry = rt_dst_alloc_handler,
    .kp = {
        .symbol_name = "rt_dst_alloc",
    },
};

static void rt_set_nexthop_handler(struct rtable *rt, __be32 daddr,
        const struct fib_result *res,
        struct fib_nh_exception *fnhe,
        struct fib_info *fi, u16 type, u32 itag)
{
    printk(KERN_EMERG "[%s]\n", __func__);
    jprobe_return();
}

static struct jprobe jprobe_rt_set_nexthop_handler =
{
    .entry = rt_set_nexthop_handler,
    .kp = {
        .symbol_name = "rt_set_nexthop",
    },
};

static bool rt_bind_exception_handler(struct rtable *rt, struct fib_nh_exception *fnhe,
        __be32 daddr)
{
    printk(KERN_EMERG "[%s]\n", __func__);
    jprobe_return();
}

static struct jprobe jprobe_rt_bind_exception =
{
    .entry = rt_bind_exception_handler,
    .kp = {
        .symbol_name = "rt_bind_exception",
    },
};

int ip_output_handler(struct net *net, struct sock *sk, struct sk_buff *skb)
{
    if (!packet_filter(skb)) {
        printk(KERN_EMERG "[%s]\n", __func__);
    }
    jprobe_return();
}

static struct jprobe jprobe_ip_output =
{
    .entry = ip_output_handler,
    .kp = {
        .symbol_name = "ip_output",
    },
};

struct rtable *ip_route_output_flow_handler(struct net *net, struct flowi4 *flp4,
        const struct sock *sk)
{
    printk(KERN_EMERG "[%s]\n", __func__);
    jprobe_return();
}

static struct jprobe jprobe_ip_route_output_flow =
{
    .entry = ip_route_output_flow_handler,
    .kp = {
        .symbol_name = "ip_route_output_flow",
    },
};

struct rtable *__ip_route_output_key_hash_handler(struct net *net, struct flowi4 *fl4,
        int mp_hash)
{
    printk(KERN_EMERG "[%s]\n", __func__);
    jprobe_return();
}

static struct jprobe jprobe_ip_route_output_key_hash =
{
    .entry = __ip_route_output_key_hash_handler,
    .kp = {
        .symbol_name = "ip_route_output_key_hash",
    },
};

int fib_table_lookup_handler(struct fib_table *tb, const struct flowi4 *flp,
        struct fib_result *res, int fib_flags)
{
    printk(KERN_EMERG "[%s]\n", __func__);
    jprobe_return();
}

static struct jprobe jprobe_fib_table_lookup_handler =
{
    .entry = fib_table_lookup_handler,
    .kp = {
        .symbol_name = "fib_table_lookup",
    },
};

static struct rtable *__mkroute_output_handler(const struct fib_result *res,
        const struct flowi4 *fl4, int orig_oif,
        struct net_device *dev_out,
        unsigned int flags)
{
    printk(KERN_EMERG "[%s]\n", __func__);
    jprobe_return();
}

static struct jprobe jprobe_mkroute_output =
{
    .entry = __mkroute_output_handler,
    .kp = {
        .symbol_name = "__mkroute_output",
    },
};

static int ip_finish_output_handler(struct net *net, struct sock *sk, struct sk_buff *skb)
{
    if (!packet_filter(skb)) {
        printk(KERN_EMERG "[%s]\n", __func__);
    }
    jprobe_return();
}

static struct jprobe jprobe_ip_finish_output =
{
    .entry = ip_finish_output_handler,
    .kp = {
        .symbol_name = "ip_finish_output",
    },
};

static int ip_finish_output2_handler(struct net *net, struct sock *sk, struct sk_buff *skb)
{
    if (!packet_filter(skb)) {
        printk(KERN_EMERG "[%s]\n", __func__);
    }
    jprobe_return();
}

static struct jprobe jprobe_ip_finish_output2 =
{
    .entry = ip_finish_output2_handler,
    .kp = {
        .symbol_name = "ip_finish_output2",
    },
};

static int arp_constructor_handler(struct neighbour *neigh)
{
    printk(KERN_EMERG "[%s]\n", __func__);
    jprobe_return();
}

static struct jprobe jprobe_arp_constructor =
{
    .entry = arp_constructor_handler,
    .kp = {
        .symbol_name = "arp_constructor",
    },
};

int neigh_resolve_output_handler(struct neighbour *neigh, struct sk_buff *skb)
{
    printk(KERN_EMERG "[%s]\n", __func__);
    jprobe_return();
}

static struct jprobe jprobe_neigh_resolve_output_ =
{
    .entry = neigh_resolve_output_handler,
    .kp = {
        .symbol_name = "neigh_resolve_output",
    },
};

static int __init jp_init(void)
{
    int ret = -1;
    printk("Test jp module init\n");

    ret = register_jprobe(&jprobe_netif_rx);
    ret = register_jprobe(&jprobe_ip_rcv);
    ret = register_jprobe(&jprobe_dev_queue_xmit);
    ret = register_jprobe(&jprobe_arp_rcv);
    ret = register_jprobe(&jprobe_arp_send);
    ret = register_jprobe(&jprobe_arp_solicit);
    ret = register_jprobe(&jprobe_arp_constructor);
    ret = register_jprobe(&jprobe_ip_output);
    ret = register_jprobe(&jprobe_ip_route_input_slow);
    ret = register_jprobe(&jprobe_ip_route_output_flow);
    ret = register_jprobe(&jprobe_ip_route_input_noref);
    ret = register_jprobe(&jprobe_ip_route_output_key_hash);
    ret = register_jprobe(&jprobe_ip_mkroute_input);
    ret = register_jprobe(&jprobe_rt_dst_alloc);
    ret = register_jprobe(&jprobe_ip_finish_output);
    ret = register_jprobe(&jprobe_ip_finish_output2);
    ret = register_jprobe(&jprobe_netif_receive_skb);
    if (ret < 0)
    {
        printk (KERN_EMERG "Register jprobe failed, return: %x\n", ret);
        return -1;
    }

#if 0
    printk(KERN_EMERG "Planted jprobe at: %p, handler addr: %p\n",
            jprobe_do_fork.kp.addr, jprobe_do_fork.entry);
#endif

    return 0;
}

static void __exit jp_exit(void)
{
    unregister_jprobe(&jprobe_netif_rx);
    unregister_jprobe(&jprobe_ip_rcv);
    unregister_jprobe(&jprobe_dev_queue_xmit);
    unregister_jprobe(&jprobe_arp_rcv);
    unregister_jprobe(&jprobe_arp_send);
    unregister_jprobe(&jprobe_arp_solicit);
    unregister_jprobe(&jprobe_arp_constructor);
    unregister_jprobe(&jprobe_ip_output);
    unregister_jprobe(&jprobe_ip_route_input_slow);
    unregister_jprobe(&jprobe_ip_route_output_flow);
    unregister_jprobe(&jprobe_ip_route_input_noref);
    unregister_jprobe(&jprobe_ip_route_output_key_hash);
    unregister_jprobe(&jprobe_ip_mkroute_input);
    unregister_jprobe(&jprobe_rt_dst_alloc);
    unregister_jprobe(&jprobe_ip_finish_output);
    unregister_jprobe(&jprobe_ip_finish_output2);
    unregister_jprobe(&jprobe_netif_receive_skb);
    printk("Test jp module removed\n");
}

module_init(jp_init);
module_exit(jp_exit);

MODULE_AUTHOR("kkkmmu");
MODULE_DESCRIPTION("Jprobe_Mudule");
MODULE_LICENSE("GPL");

