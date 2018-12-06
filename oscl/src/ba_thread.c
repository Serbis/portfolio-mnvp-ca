#include "../include/ba_therad.h"
#include "../../libs/miniakka/includes/standard_messages.h"
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ActorMessage* new_r_BaThread_Error(char *reason, uint16_t stackSize) {
    char *body = pmalloc(sizeof(reason) + 1);
    strcpy(body, reason);

    return new_ActorMessage(r_BaThread_Error, body, stackSize, del_default);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ActorMessage* new_r_BaThread_Connected(int socket, uint16_t stackSize) {
    int *body = pmalloc(sizeof(int));
    *body = socket;

    return new_ActorMessage(r_BaThread_Connected, body, stackSize, del_default);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void BaThread_run(void *args) {
    BaThreadArgs * tArgs = (BaThreadArgs*) args;

    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's host information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    char buffer[sizeof(int) * 2 + 1];
    sprintf(buffer, "%d", tArgs->port);

    if ((rv = getaddrinfo(tArgs->address, buffer, &hints, &servinfo)) != 0) {
        tArgs->creator->tell(tArgs->creator, new_r_BaThread_Error("getaddrinfo", 64), NULL, tArgs->system);
        //fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            //tArgs->creator->tell(tArgs->creator, new_r_BaThread_Error("server: socket", 64), NULL);
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            //tArgs->creator->tell(tArgs->creator, new_r_BaThread_Error("setsockopt", 64), NULL);
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            //tArgs->creator->tell(tArgs->creator, new_r_BaThread_Error("server: bind", 64), NULL);
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        tArgs->creator->tell(tArgs->creator, new_r_BaThread_Error("server: failed to bind", 64), NULL, tArgs->system);
        return;
    }

    if (listen(sockfd, 10) == -1) {
        tArgs->creator->tell(tArgs->creator, new_r_BaThread_Error("listen", 64), NULL, tArgs->system);
        return;
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        tArgs->creator->tell(tArgs->creator, new_r_BaThread_Error("sigaction", 64), NULL, tArgs->system);
        return;
    }

    //printf("server: waiting for connections...\n");

    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
        tArgs->creator->tell(tArgs->creator, new_r_BaThread_Error("accept", 64), NULL, tArgs->system);
        return;
    }

    inet_ntop(their_addr.ss_family,
              get_in_addr((struct sockaddr *)&their_addr),
              s, sizeof s);
    //printf("server: got connection from %s\n", s);
    tArgs->creator->tell(tArgs->creator, new_r_BaThread_Connected(new_fd, 64), NULL, tArgs->system);
    close(sockfd);
    pfree(tArgs->address);
    pfree(tArgs->creator);
    pfree(tArgs);
}