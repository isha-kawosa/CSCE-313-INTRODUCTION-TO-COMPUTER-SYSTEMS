#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <stdlib.h>
#include <netdb.h>

/* IP Header */
struct ipheader {
    unsigned char iph_ihl : 4, iph_ver : 4;           // IP Header length & Version.
    unsigned char iph_tos;                            // Type of service
    unsigned short int iph_len;                       // IP Packet length (Both data and header)
    unsigned short int iph_ident;                     // Identification
    unsigned short int iph_flag : 3, iph_offset : 13; // Flags and Fragmentation offset
    unsigned char iph_ttl;                            // Time to Live
    unsigned char iph_protocol;                       // Type of the upper-level protocol
    unsigned short int iph_chksum;                    // IP datagram checksum
    struct in_addr iph_sourceip;                      // IP Source address (In network byte order)
    struct in_addr iph_destip;                        // IP Destination address (In network byte order)
};

/* ICMP Header */
struct icmpheader {
    unsigned char icmp_type;        // ICMP message type
    unsigned char icmp_code;        // Error code
    unsigned short int icmp_chksum; // Checksum for ICMP Header and data
    unsigned short int icmp_id;     // Used in echo request/reply to identify request
    unsigned short int icmp_seq;    // Identifies the sequence of echo messages,
                                    // if more than one is sent.
};

#define ICMP_ECHO_REPLY     0
#define ICMP_ECHO_REQUEST   8
#define ICMP_TIME_EXCEEDED  11
#define MAX_HOPS            30
#define MAX_RETRY           3
#define PACKET_LEN          1500

unsigned short checksum(void* b, int len){    
    unsigned short* buf = (unsigned short*)b; 
    unsigned int sum = 0; 
    unsigned short result;

    for (sum = 0; len > 1; len -= 2) 
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char*)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

void traceroute(char* dest) {
    // raw sockets require root priviliges: no change necessary
    if (getuid() != 0) {
        perror("requires root privilige");
        exit(-1);
    }

    // open socket: no change necessary
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(-1);
    }

    // dns resolve and get ip for destination: no change necessary
    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));
    addr.sin_family = AF_INET;
    hostent* getip = gethostbyname(dest);
    if (getip == NULL) {
        perror("failed gethostbyname");
        exit(-1);
    }
    memcpy((char*)(&addr.sin_addr), getip->h_addr, getip->h_length);

    printf("traceroute to %s (%s), %d hops max, %ld bytes packets\n", dest, inet_ntoa(addr.sin_addr), MAX_HOPS, sizeof(ipheader) + sizeof(icmpheader));
    
    char send_buf[PACKET_LEN], recv_buf[PACKET_LEN];
    /** TODO: 1
     * Prepare packet
     * a. outgoing packets only contain the icmpheader with type = ICMP_ECHO_REQUEST, code = 0
     * b. ID in the icmpheader should be set to current process id to identify received ICMP packets
     * c. checksum can be set to 0 for this test
     * d. write/copy the header to the send_buf  
     * 
     * HINT:
     * - icmpheader* icmp = (icmpheader*)send_buf;
     * - set header fields with required values: icmp->field = value;
     * */
    struct icmpheader* icmp = (struct icmpheader*)(send_buf);
    struct sockaddr_in recv_addr;
    socklen_t addr_len = sizeof(recv_addr);
    struct timeval tv;
    fd_set rfd;
    int pid = getpid();
    for (int ttl = 1; ttl <= MAX_HOPS; ttl++) {
        printf("%2d ", ttl);
        /** TODO: 2
         * set the seq in icmpheader to ttl
         * 
         * HINT:
         * similar to TODO 1 HINT, just set the seq
         */
        memset(send_buf, 0, PACKET_LEN);
        icmp->icmp_type = ICMP_ECHO_REQUEST;
        icmp->icmp_code = 0;
        icmp->icmp_chksum = 0;
        icmp->icmp_id = pid;
        icmp->icmp_seq = ttl;
        icmp->icmp_chksum = checksum(icmp, sizeof(struct icmpheader));

        // set ttl to outgoing packets: no need to change
        if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, (const char*)&ttl, sizeof(ttl)) < 0) {
            perror("setsockopt failed");
            exit(-1);
        }

        int retry = 0;
        while (retry < MAX_RETRY) {
            /** TODO: 3
             * send packet using sendto(...)
             * 
             * HINT:
             * - check man page of sendto(...)
             * - ensure we send one icmpheader in the packet
             * 
             */
            if (sendto(sockfd, send_buf, sizeof(struct icmpheader), 0, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
                perror("sendto failed");
                exit(-1);
            }

           
            // wait to check if there is data available to receive; need to retry if timeout: no need to change
            tv.tv_sec = 1;
            tv.tv_usec = 0;
            FD_ZERO(&rfd);
            FD_SET(sockfd, &rfd);
            int ret = select(sockfd + 1, &rfd, NULL, NULL, &tv);

            /** TOOD: 4
             * Check if data available to read or timeout
             * a. if ret == 0: timeout --> retry upto MAX_RETRY
             * b. if ret > 0: data available, use recvfrom(...) to read data to recv_buf and process --> see TODO 5 below
             */
            if (ret == 0) {
                // TODO 4.a
                printf("* ");
                retry++;
            }
            else if (ret > 0) {
                if(recvfrom(sockfd, recv_buf, PACKET_LEN, 0,(struct sockaddr*)&recv_addr, &addr_len)<0){
                    perror("recvfrom failed");
                    continue;
                }

                struct ipheader* recv_ip =(struct ipheader*)recv_buf;
                struct icmpheader* recv_icmp =(struct icmpheader*)(recv_buf + sizeof(struct ipheader));

                if(recv_icmp->icmp_type == ICMP_TIME_EXCEEDED){
                    struct ipheader* orig_ip = (struct ipheader*)(recv_buf + sizeof(struct ipheader) + sizeof(struct icmpheader));
                    struct icmpheader* orig_icmp = (struct icmpheader*)(recv_buf + 2 * sizeof(struct ipheader) + sizeof(struct icmpheader));

                    if(orig_icmp->icmp_seq == ttl && orig_icmp->icmp_id == pid){
                        printf("%s\n", inet_ntoa(recv_ip->iph_sourceip));
                        break;
                    }
                } 
                else if(recv_icmp->icmp_type == ICMP_ECHO_REPLY && recv_icmp->icmp_id == pid){
                    printf("%s\n", inet_ntoa(recv_ip->iph_sourceip));
                    close(sockfd);
                    return;
                }
            }
            else {
                perror("select failed");
                exit(-1);
            }
            fflush(stdout);

            /** TODO: 6
             * Check if timed out for MAX_RETRY times; increment ttl to move on to processing next hop
             */
            if (retry == MAX_RETRY) {
                printf("\n");
            }
        }
    }
    close(sockfd);
}

int main(int argc, char** argv) {

    if (argc < 2) {
        printf("Usage: traceroute <destination hostname>\n");
        exit(-1);
    }
    
    char* dest = argv[1];
    traceroute(dest);

    return 0;
}