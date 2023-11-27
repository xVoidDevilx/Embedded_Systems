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

char *UDPParse(char *buff, struct sockaddr_in *clientAddr); // UDP payload parser
void NetToSpeaker();                           // take what's in the network buffer and play to the speaker

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

// extract an ip, port, and payload from an incoming stream of chars
char *UDPParse(char *buff, struct sockaddr_in *clientAddr)
{
    char *token;
    char *colon;
    int32_t AddrByte;
    uint32_t PortWord;

    token = buff;
    if(!token) return token;

    // verify byte 1
    if(isdigit(*token) == 0)
    {
        ErrorOut(&NETERR, "Message missing required digits");
        return NULL;
    }
    AddrByte = atoi(token);
    clientAddr->sin_addr.s_addr = AddrByte;

    //byte 2
    token = strchr(token, '.');
    if(!token) return token;
    token++;
    if(*token == 0) return NULL;
    if(isdigit(*token)==0) return NULL;
    AddrByte = atoi(token);
    clientAddr->sin_addr.s_addr |= AddrByte << 8;

    // byte 3
    token = strchr(token, '.');
    if(!token) return token;
    token++;
    if(*token == 0) return NULL;
    if(isdigit(*token)==0) return NULL;
    AddrByte = atoi(token);
    clientAddr->sin_addr.s_addr |= AddrByte << 16;

    // byte 4
    token = strchr(token, '.');
    if(!token) return token;
    token++;
    if(*token == 0) return NULL;
    if(isdigit(*token)==0) return NULL;
    AddrByte = atoi(token);
    clientAddr->sin_addr.s_addr |= AddrByte << 24;

    // get the port
    colon = strchr(token, ':');
    if(!colon)
    {
        PortWord = DEFAULT_NET_PORT;
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

    // extract the payload, if any
    if(token)
        token = strchr(token, '-');
    return token;
}

void Dial(char *message)
{
    char *token;
    struct sockaddr_in clientAddr;
    char txbuf[MAXLEN];
    char netbuf[100];

    // extract the IP address we intend to call
    token = GetNxtStr(message, true);

    if(!token)  // no IP provided, close the connections (hangup both calls, if any)
    {
        if(glo.REGISTERS[SHADOW_DIAL_0_IP] != 0)
        {
            // tell connected to hangup the call
            sprintf(netbuf, "-netudp %d.%d.%d.%d:%d",
                    (uint8_t)(glo.REGISTERS[SHADOW_DIAL_0_IP] >> 24)&0xFF, (uint8_t)(glo.REGISTERS[SHADOW_DIAL_0_IP] >> 16)&0xFF,
                    (uint8_t)(glo.REGISTERS[SHADOW_DIAL_0_IP] >> 8)&0xFF,(uint8_t)(glo.REGISTERS[SHADOW_DIAL_0_IP])&0xFF,
                       glo.REGISTERS[SHADOW_DIAL_0_PORT]);
            sprintf(txbuf, "%s -stream 0", netbuf);
            AddPayload(txbuf);
            sprintf(txbuf, "%s -regs %d #0", netbuf, SHADOW_DIAL_0_IP);  // no correction needed - reset
            AddPayload(txbuf);
        }
        // clear the dialed registers
        AddPayload("-stream 0");    // hangup my call
        glo.REGISTERS[SHADOW_DIAL_0_IP] = glo.REGISTERS[SHADOW_DIAL_1_IP] = 0;
        glo.REGISTERS[SHADOW_DIAL_0_PORT] = DEFAULT_NET_PORT;
        return;
    }

    // view the registers we called
    if(*token == 'r')
        goto PRINT_ADDR;

    //sets up our intended client, port, + extracts payload into token
    clientAddr.sin_addr.s_addr = 0;
    clientAddr.sin_port = DEFAULT_NET_PORT;
    token = UDPParse(token, &clientAddr);

    // extract the dialed ip - has to be flipped
    glo.REGISTERS[SHADOW_DIAL_0_IP]  = (clientAddr.sin_addr.s_addr & 0xFF) << 24;
    glo.REGISTERS[SHADOW_DIAL_0_IP] |= ((clientAddr.sin_addr.s_addr >> 8) & 0xFF) << 16;
    glo.REGISTERS[SHADOW_DIAL_0_IP] |= ((clientAddr.sin_addr.s_addr >> 16) & 0xFF) << 8;
    glo.REGISTERS[SHADOW_DIAL_0_IP] |= (clientAddr.sin_addr.s_addr >> 24) & 0xFF;

    // setup the shadow dial port
    glo.REGISTERS[SHADOW_DIAL_0_PORT] = (clientAddr.sin_port & 0xFF)    << 8;
    glo.REGISTERS[SHADOW_DIAL_0_PORT] |= (clientAddr.sin_port & 0xFF00) >> 8;

    // setup the network call
    sprintf(netbuf, "-netudp %d.%d.%d.%d:%d",
            (uint8_t)(glo.REGISTERS[SHADOW_DIAL_0_IP]>>24)&0xFF, (uint8_t)(glo.REGISTERS[SHADOW_DIAL_0_IP]>>16)&0xFF,
            (uint8_t)(glo.REGISTERS[SHADOW_DIAL_0_IP]>>8)&0xFF, (uint8_t)(glo.REGISTERS[SHADOW_DIAL_0_IP])&0xFF,
            glo.REGISTERS[SHADOW_DIAL_0_PORT]);

    // Tell contact who I am
    sprintf(txbuf, "%s -regs %d #%d", netbuf, SHADOW_DIAL_0_IP, glo.REGISTERS[IP_SHADOW_REG]);   // setup proxy dial reg with my ip
    AddPayload(txbuf);
    sprintf(txbuf, "%s -regs %d #%d", netbuf, SHADOW_DIAL_0_PORT, glo.REGISTERS[PORT_SHADOW_REG]);   // setup proxy dial reg with my port
    AddPayload(txbuf);
    sprintf(txbuf, "%s -stream 1", netbuf);   // setup proxy dial reg with my port
    AddPayload(txbuf);
    AddPayload("-stream 1");

PRINT_ADDR:
    sprintf(txbuf, "-regs %d", SHADOW_DIAL_0_IP);
    AddPayload(txbuf);
    sprintf(txbuf, "-regs %d", SHADOW_DIAL_0_PORT);
    AddPayload(txbuf);
}
