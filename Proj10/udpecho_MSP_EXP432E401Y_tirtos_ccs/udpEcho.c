/*
 *    ======== udpEcho.c ========
 *    Contains BSD sockets code.
 */

#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include <pthread.h>
/* BSD support */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>

#include <ti/net/slnetutils.h>

#define UDPPACKETSIZE 1472
#define MAXPORTLEN    6

extern void fdOpenSession();
extern void fdCloseSession();
extern void *TaskSelf();

/*
 *  ======== echoFxn ========
 *  Echoes UDP messages.
 *
 */
void *echoFxn(void *arg0)
{
    int                bytesRcvd;
    int                bytesSent;
    int                status;
    int                server = -1;
    fd_set             readSet;
    struct addrinfo    hints;
    struct addrinfo    *res, *p;
    struct sockaddr_in clientAddr;
    socklen_t          addrlen;
    char               buffer[UDPPACKETSIZE];
    char               portNumber[MAXPORTLEN];

    fdOpenSession(TaskSelf());

//    Display_printf(display, 0, 0, "UDP Echo example started\n");

    sprintf(portNumber, "%d", *(uint16_t *)arg0);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags    = AI_PASSIVE;

    /* Obtain addresses suitable for binding to */
    status = getaddrinfo(NULL, portNumber, &hints, &res);
    if (status != 0) {
//        Display_printf(display, 0, 0, "Error: getaddrinfo() failed: %s\n",
//            gai_strerror(status));
        goto shutdown;
    }

    for (p = res; p != NULL; p = p->ai_next) {
        server = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (server == -1) {
            continue;
        }

        status = bind(server, p->ai_addr, p->ai_addrlen);
        if (status != -1) {
            break;
        }

        close(server);
    }

    if (server == -1) {
        //Display_printf(display, 0, 0, "Error: socket not created.\n");
        goto shutdown;
    } else if (p == NULL) {
        //Display_printf(display, 0, 0, "Error: bind failed.\n");
        goto shutdown;
    } else {
        freeaddrinfo(res);
        res = NULL;
    }

    do {
        /*
         *  readSet and addrlen are value-result arguments, which must be reset
         *  in between each select() and recvfrom() call
         */
        FD_ZERO(&readSet);
        FD_SET(server, &readSet);
        addrlen = sizeof(clientAddr);

        /* Wait forever for the reply */
        status = select(server + 1, &readSet, NULL, NULL, NULL);
        if (status > 0) {
            if (FD_ISSET(server, &readSet)) {
                bytesRcvd = recvfrom(server, buffer, UDPPACKETSIZE, 0,
                        (struct sockaddr *)&clientAddr, &addrlen);

                if (bytesRcvd > 0) {
                    bytesSent = sendto(server, buffer, bytesRcvd, 0,
                            (struct sockaddr *)&clientAddr, addrlen);
                    if (bytesSent < 0 || bytesSent != bytesRcvd) {
//                        Display_printf(display, 0, 0,
//                                "Error: send to failed.\n");
                        goto shutdown;
                    }
                }
            }
        }
    } while (status > 0);

shutdown:
    if (res) {
        freeaddrinfo(res);
    }

    if (server != -1) {
        close(server);
    }

    fdCloseSession(TaskSelf());

    return (NULL);
}
