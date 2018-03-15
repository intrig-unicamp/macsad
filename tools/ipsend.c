#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <linux/if_vlan.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
//#include <netinet/in.h>       // IPPROTO_RAW, IPPROTO_IP, IPPROTO_TCP, INET_ADDRSTRLEN
#include <netinet/ip.h>
#include <netinet/ether.h>
//#include <ip.h>

#define MY_DEST_MAC0    0x00
#define MY_DEST_MAC1    0x00
#define MY_DEST_MAC2    0x00
#define MY_DEST_MAC3    0x00
#define MY_DEST_MAC4    0x01
#define MY_DEST_MAC5    0x01

#define DEFAULT_IF      "veth1.1"
#define BUF_SIZ         1024

#define IP4_HDRLEN 20         // IPv4 header length
#define TCP_HDRLEN 0         // TCP header length, excludes options data

uint16_t checksum (uint16_t *, int);
uint16_t ip_checksum(void *, size_t);
int main(int argc, char *argv[])
{
        int ip_flags[4];
        int status;
        int sockfd;
        struct ifreq if_Sidx;
        struct ifreq if_Smac;
        struct ifreq if_Didx;
        struct ifreq if_Dmac;
        int tx_len = 0;
        char sendbuf[BUF_SIZ];
        struct ether_header *eh = (struct ether_header *) sendbuf;
 		struct iphdr *iph = (struct iphdr *) (sendbuf + sizeof(struct ether_header));
        struct sockaddr_ll socket_address;
        char ifDName[IFNAMSIZ];
        char ifSName[IFNAMSIZ];
		int pkt_cnt = 1;

        /* Get interface name */
        if (argc == 3)
		{
                strcpy(ifSName, argv[2]);
                strcpy(ifDName, argv[1]);
        }
        else if (argc == 4)
		{
			printf("argc 4\n");
                strcpy(ifSName, argv[1]);
                strcpy(ifDName, argv[2]);
				pkt_cnt = atoi(argv[3]);
        }
		else
		{
			printf("Usage: sendip <src interface name> <dst interface name> <number of packets> \n");
			return 0;
		}

		printf("number of pkt to send %d\n",pkt_cnt);
        /* Open RAW socket to send on */
        if ((sockfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
            perror("socket");
        }

        /* Get the index of the interface to send on */
        memset(&if_Sidx, 0, sizeof(struct ifreq));
        strncpy(if_Sidx.ifr_name, ifSName, IFNAMSIZ-1);
        if (ioctl(sockfd, SIOCGIFINDEX, &if_Sidx) < 0)
            perror("SIOCGIFINDEX");
        /* Get the MAC address of the interface to send on */
        memset(&if_Smac, 0, sizeof(struct ifreq));
        strncpy(if_Smac.ifr_name, ifSName, IFNAMSIZ-1);
        if (ioctl(sockfd, SIOCGIFHWADDR, &if_Smac) < 0)
            perror("SIOCGIFHWADDR");

        /* Get the index of the interface to send on */
        memset(&if_Didx, 0, sizeof(struct ifreq));
        strncpy(if_Didx.ifr_name, ifDName, IFNAMSIZ-1);
        if (ioctl(sockfd, SIOCGIFINDEX, &if_Didx) < 0)
            perror("SIOCGIFINDEX");
        /* Get the MAC address of the interface to send on */
        memset(&if_Dmac, 0, sizeof(struct ifreq));
        strncpy(if_Dmac.ifr_name, ifDName, IFNAMSIZ-1);
        if (ioctl(sockfd, SIOCGIFHWADDR, &if_Dmac) < 0)
            perror("SIOCGIFHWADDR");

		/* Construct the Ethernet header */
        memset(sendbuf, 0, BUF_SIZ);
		/* Ethernet header */
        eh->ether_shost[0] = ((uint8_t *)&if_Smac.ifr_hwaddr.sa_data)[0];
        eh->ether_shost[1] = ((uint8_t *)&if_Smac.ifr_hwaddr.sa_data)[1];
        eh->ether_shost[2] = ((uint8_t *)&if_Smac.ifr_hwaddr.sa_data)[2];
        eh->ether_shost[3] = ((uint8_t *)&if_Smac.ifr_hwaddr.sa_data)[3];
        eh->ether_shost[4] = ((uint8_t *)&if_Smac.ifr_hwaddr.sa_data)[4];
        eh->ether_shost[5] = ((uint8_t *)&if_Smac.ifr_hwaddr.sa_data)[5];
        eh->ether_dhost[0] = ((uint8_t *)&if_Dmac.ifr_hwaddr.sa_data)[0];
        eh->ether_dhost[1] = ((uint8_t *)&if_Dmac.ifr_hwaddr.sa_data)[1];
        eh->ether_dhost[2] = ((uint8_t *)&if_Dmac.ifr_hwaddr.sa_data)[2];
        eh->ether_dhost[3] = ((uint8_t *)&if_Dmac.ifr_hwaddr.sa_data)[3];
        eh->ether_dhost[4] = ((uint8_t *)&if_Dmac.ifr_hwaddr.sa_data)[4];
        eh->ether_dhost[5] = ((uint8_t *)&if_Dmac.ifr_hwaddr.sa_data)[5];

		/* Ethertype field */
        eh->ether_type = htons(ETH_P_IP);
        tx_len += sizeof(struct ether_header);

        /* Packet data */
        sendbuf[tx_len++] = 0xde;
        sendbuf[tx_len++] = 0xad;
        sendbuf[tx_len++] = 0xbe;
        sendbuf[tx_len++] = 0xef;

  // IPv4 header

  // IPv4 header length (4 bits): Number of 32-bit words in header = 5
  iph->ihl = 5; //IP4_HDRLEN / sizeof (uint32_t);

  // Internet Protocol version (4 bits): IPv4
  iph->version = 4;

  // Type of service (8 bits)
  iph->tos = 0;

  // Total length of datagram (16 bits): IP header + TCP header
  iph->tot_len = htons (IP4_HDRLEN + TCP_HDRLEN);

  // ID sequence number (16 bits): unused, since single datagram
  iph->id = htons (0);

  // Flags, and Fragmentation offset (3, 13 bits): 0 since single datagram

  // Zero (1 bit)
  ip_flags[0] = 0;

  // Do not fragment flag (1 bit)
  ip_flags[1] = 0;

  // More fragments following flag (1 bit)
  ip_flags[2] = 0;

  // Fragmentation offset (13 bits)
  ip_flags[3] = 0;

  iph->frag_off = htons ((ip_flags[0] << 15)
                      + (ip_flags[1] << 14)
                      + (ip_flags[2] << 13)
                      +  ip_flags[3]);

  // Time-to-Live (8 bits): default to maximum value
  iph->ttl = 255;

  // Transport layer protocol (8 bits): 6 for TCP
  iph->protocol = 0; //IPPROTO_TCP;

  // Source IPv4 address (32 bits)
  if ((status = inet_pton (AF_INET, "10.0.0.2", &(iph->saddr))) != 1) {
    fprintf (stderr, "inet_pton() failed.\nError message: %s", strerror (status));
    exit (EXIT_FAILURE);
  }

  // Destination IPv4 address (32 bits)
  if ((status = inet_pton (AF_INET, "10.0.0.1", &(iph->daddr))) != 1) {
    fprintf (stderr, "inet_pton() failed.\nError message: %s", strerror (status));
    exit (EXIT_FAILURE);
  }

  // IPv4 header checksum (16 bits): set to 0 when calculating checksum
  iph->check = 0;
  iph->check = ip_checksum ((void *) &iph, IP4_HDRLEN);

  tx_len += sizeof(struct iphdr) + TCP_HDRLEN;

        /* Index of the network device */
        socket_address.sll_ifindex = if_Sidx.ifr_ifindex;
        /* Address length*/
        socket_address.sll_halen = ETH_ALEN;
        /* Destination MAC */
        socket_address.sll_addr[0] = ((uint8_t *)&if_Dmac.ifr_hwaddr.sa_data)[0];
        socket_address.sll_addr[1] = ((uint8_t *)&if_Dmac.ifr_hwaddr.sa_data)[1];
        socket_address.sll_addr[2] = ((uint8_t *)&if_Dmac.ifr_hwaddr.sa_data)[2];
        socket_address.sll_addr[3] = ((uint8_t *)&if_Dmac.ifr_hwaddr.sa_data)[3];
        socket_address.sll_addr[4] = ((uint8_t *)&if_Dmac.ifr_hwaddr.sa_data)[4];
        socket_address.sll_addr[5] = ((uint8_t *)&if_Dmac.ifr_hwaddr.sa_data)[5];

        /* Send packet */
		int ix;
		for (ix = 0; ix < pkt_cnt; ix++)
		{
        if (sendto(sockfd, sendbuf, tx_len, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0)
            printf("Send failed\n");
		}
        return 0;
}

// Computing the internet checksum (RFC 1071).
// Note that the internet checksum does not preclude collisions.
uint16_t
checksum (uint16_t *addr, int len)
{
  int count = len;
  register uint32_t sum = 0;
  uint16_t answer = 0;

  // Sum up 2-byte values until none or only one byte left.
  while (count > 1) {
    sum += *(addr++);
    if (sum & 0x80000000) {
      sum = (sum & 0xFFFF) + (sum >> 16);
    }
    count -= 2;
  }

  // Add left-over byte, if any.
  if (count > 0) {
    sum += *(uint8_t *) addr;
  }

  // Fold 32-bit sum into 16 bits; we lose information by doing this,
  // increasing the chances of a collision.
  // sum = (lower 16 bits) + (upper 16 bits shifted right 16 bits)
  while (sum >> 16) {
    sum = (sum & 0xffff) + (sum >> 16);
  }

  // Checksum is one's compliment of sum.
  answer = ~sum;

  return (answer);
}

uint16_t ip_checksum(void* vdata, size_t length) {
    // Cast the data pointer to one that can be indexed.
    char* data=(char*)vdata;

    // Initialise the accumulator.
    uint32_t acc=0xffff;

    // Handle complete 16-bit blocks.
    size_t i;
    for (i=0;i+1<length;i+=2) {
        uint16_t word;
        memcpy(&word,data+i,2);
        acc+=ntohs(word);
        if (acc>0xffff) {
            acc-=0xffff;
        }
    }

    // Handle any partial block at the end of the data.
    if (length&1) {
        uint16_t word=0;
        memcpy(&word,data+length-1,1);
        acc+=ntohs(word);
        if (acc>0xffff) {
            acc-=0xffff;
        }
    }

    // Return the checksum in network byte order.
    return htons(~acc);
}

