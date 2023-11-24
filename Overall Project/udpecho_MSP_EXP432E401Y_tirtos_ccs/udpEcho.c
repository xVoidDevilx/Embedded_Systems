/*
 *    ======== udpEcho.c ========
 *    Contains BSD sockets code.
 */

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "GoodFortune.h"
#include <pthread.h>
#include <ctype.h>

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
extern error NETERR;

char *UDPParse(char *buff, struct sockaddr_in *clientAddr);

/*
 *  Receives UDP packets.
 *
 */
void *UDPrxFxn(void *arg0)
{
    int                bytesRcvd;
    int                status;
    int                server = -1;
    fd_set             readSet;
    struct addrinfo    hints;
    struct addrinfo    *res, *p;
    struct sockaddr_in clientAddr;
    socklen_t          addrlen;
    char               buffer[UDPPACKETSIZE + 1];   // adding 0 @ the end of str
    char               portNumber[MAXPORTLEN];
    char               txbuf[MAXLEN];

    fdOpenSession(TaskSelf());

    AddPayload("-print UDP RX started");

    sprintf(portNumber, "%d", *(uint16_t *)arg0);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags    = AI_PASSIVE;

    /* Obtain addresses suitable for binding to */
    status = getaddrinfo(NULL, portNumber, &hints, &res);
    if (status != 0) {
        sprintf(txbuf, "Error: getaddrinfo() failed: %s\n",
            gai_strerror(status));
        ErrorOut(&NETERR, txbuf);
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
        ErrorOut(&NETERR, "Error: socket not created.\n");
        goto shutdown;
    } else if (p == NULL) {
        ErrorOut(&NETERR, "Error: bind failed.\n");
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

                if (bytesRcvd > 0)
                {

                    buffer[bytesRcvd] = 0; // one extra byte in buffer so this is safe for strlen
                    //special case for HWI priority to handle a voice call
                    if (!strncmp("-voice", buffer, strlen("-voice")))
                        VoiceCMD(buffer);
                    else
                    {
                        sprintf(txbuf, "UDP %d.%d.%d.%d:%d > ",
                                  (uint8_t)(clientAddr.sin_addr.s_addr) &0xFF, (uint8_t)(clientAddr.sin_addr.s_addr>>8) &0xFF,
                                  (uint8_t)(clientAddr.sin_addr.s_addr>>16) &0xFF, (uint8_t)(clientAddr.sin_addr.s_addr>>24) &0xFF,
                                  clientAddr.sin_port);
                        UART_write(glo.DEVICES.uart0,
                                   txbuf,
                                   strlen(txbuf));
                        UART_write(glo.DEVICES.uart0,
                                   buffer,
                                   strlen(buffer));
                        UART_write(glo.DEVICES.uart0,"\r\n",2);
                        AddPayload(buffer);
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

/*
 *  Transmits UDP packets.
 *
 */
void *UDPtxFxn(void *arg0)
{
    int                bytesSent;
    int                status;
    int                server = -1;
    fd_set             readSet;
    fd_set             writeSet;
    struct addrinfo    hints;
    struct addrinfo    *res, *p;
    struct sockaddr_in clientAddr;
    socklen_t          addrlen;
    char               portNumber[MAXPORTLEN];
    char               txbuf[MAXLEN];
    char               *token;
    int32_t            payloadnext;
    int                bytesRequested;

    fdOpenSession(TaskSelf());
    AddPayload("-print UDP TX started");

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags    = AI_PASSIVE;

    /* Obtain addresses suitable for binding to */
    status = getaddrinfo(NULL, portNumber, &hints, &res);
    if (status != 0) {
        sprintf(txbuf, "Error: getaddrinfo() failed: %s\n",
            gai_strerror(status));
        ErrorOut(&NETERR, txbuf);
        goto shutdown;
    }

    for (p = res; p != NULL; p = p->ai_next)
    {
        server = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (server == -1)
            continue;
    }

    if (server == -1)
    {
        ErrorOut(&NETERR, "Error: socket not created.\n");
        goto shutdown;
    }
    else // server is valid
    {
        freeaddrinfo(res);
        res = NULL;
    }

    for(;;)
    {

        /*
         *  readSet and addrlen are value-result arguments, which must be reset
         *  in between each select() and recvfrom() call
         */
        FD_ZERO(&readSet);
        FD_SET(server, &readSet);
        addrlen = sizeof(clientAddr);

        /* Wait forever for the reply */
        Semaphore_pend(glo.BIOS->UDPOutSem, BIOS_WAIT_FOREVER);

        status = select(server + 1, NULL, &writeSet, NULL, NULL);

        if (status <= 0)
        {
            ErrorOut(&NETERR, "Server Failed");
            continue;
        }

        token = UDPParse(glo.netOutQueue.payloads[glo.netOutQueue.PayloadReading], &clientAddr);

        if (!token)
            continue;

        bytesRequested = strlen(token) + 1;
        bytesRequested += glo.netOutQueue.BinaryCount[glo.netOutQueue.PayloadReading];

        bytesSent = sendto(server, token, bytesRequested, 0,
                (struct sockaddr *)&clientAddr, addrlen);

        if (bytesSent < 0 || bytesSent != bytesRequested)
            ErrorOut(&NETERR, "SendTo failed");

        payloadnext = glo.netOutQueue.PayloadReading + 1;

        if(payloadnext >= QUEUELEN)
            payloadnext = 0;

        glo.netOutQueue.PayloadReading = payloadnext;
    }

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

char *UDPParse(char *buff, struct sockaddr_in *clientAddr)
{
    char *token;
    char *colon;
    int32_t AddrByte;
    uint32_t PortWord;

    token = buff;
    if(!token) return token;
    if(isdigit(*token) == 0)
    {
        ErrorOut(&NETERR, "Message missing required digits");
        return NULL;
    }
    AddrByte = atoi(token);
    clientAddr->sin_addr.s_addr = AddrByte;

    token = strchr(token, '.');
    if(!token) return token;
    token++;
    if(*token == 0) return NULL;
    if(isdigit(*token)==0) return NULL;
    AddrByte = atoi(token);
    clientAddr->sin_addr.s_addr |= AddrByte << 8;

    token = strchr(token, '.');
    if(!token) return token;
    token++;
    if(*token == 0) return NULL;
    if(isdigit(*token)==0) return NULL;
    AddrByte = atoi(token);
    clientAddr->sin_addr.s_addr |= AddrByte << 16;

    token = strchr(token, '.');
    if(!token) return token;
    token++;
    if(*token == 0) return NULL;
    if(isdigit(*token)==0) return NULL;
    AddrByte = atoi(token);
    clientAddr->sin_addr.s_addr |= AddrByte << 24;

    colon = strchr(token, ':');
    if(!colon)
    {
        PortWord = UDPPORT;
        token = strchr(token, ' ');
    }
    else
    {
        token = colon;
        token++;
        if(*token == 0) return NULL;
        if(isdigit(*token) == 0) return NULL;
        PortWord = atoi(token);
    }
    clientAddr->sin_port = (PortWord & 0xFF00) >> 8;
    clientAddr->sin_port |= (PortWord & 0xFF) << 8;
    clientAddr->sin_family = AF_INET;

    if(token)
        token = strchr(token, '-');
    return token;
}
