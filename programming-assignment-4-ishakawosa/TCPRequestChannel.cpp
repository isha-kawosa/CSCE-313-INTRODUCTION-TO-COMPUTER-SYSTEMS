#include "TCPRequestChannel.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using namespace std;


TCPRequestChannel::TCPRequestChannel (const std::string _ip_address, const std::string _port_no) {
    //if server
    // create socket on specified port
    //   specify domain, type and protocol
    // bind the sock to addr set-ups listening
    // mark socket as listening

    //if client
    // create socket on specified port
    //   specify domain, type and protocol
    // connect socket to the IP addr of the server

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sockfd<0){
        perror("cannot create socket");
        exit(1);
    }

    //server
    if(_ip_address.empty()){
        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(stoi(_port_no));
        bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
        listen(sockfd, SOMAXCONN);
    }

    //client
    else{ 
        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(stoi(_port_no));
        inet_pton(AF_INET, _ip_address.c_str(), &(serv_addr.sin_addr));
        connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
    }

}

TCPRequestChannel::TCPRequestChannel (int _sockfd) {
    //
    sockfd = _sockfd;
}

TCPRequestChannel::~TCPRequestChannel () {
    // close the sockfd
    close(sockfd);
}

int TCPRequestChannel::accept_conn () {
    struct sockaddr_storage client_addr;
    socklen_t addr_len= sizeof(client_addr);
    return accept(sockfd, (struct sockaddr*)&client_addr,&addr_len);
    //implementing accept(...) -retval the sockfd of client
}
//read/write, recv/send
int TCPRequestChannel::cread (void* msgbuf, int msgsize) {
    return recv(sockfd, msgbuf, msgsize, 0);
}

int TCPRequestChannel::cwrite (void* msgbuf, int msgsize) {
    return send (sockfd, msgbuf, msgsize, 0);
}