#include "GoodFortune.h"
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/* Process inputs, update global queue, and return the signal to continue the program*/
char UartAddByte(char input){

    uint32_t gatekey;   //used to lock the resource while writing
    int32_t index = glo.UartMsg.index; //tracking the message len
    // Switch based on input
    gatekey = GateSwi_enter(glo.BIOS->MSGWriteGate);
    switch (input) {
        case '\b':
        case BACKSPACE:
            if (index > 0)
            {
                glo.UartMsg.msgBuf[--index] = 0;
                glo.UartMsg.index--;        //updates the current len of the string
            }
            GateSwi_leave(glo.BIOS->MSGWriteGate, gatekey);
            return 'n';
        case '\n':
        case '\r':
            glo.UartMsg.msgBuf[++index] = 0;    //terminate the buffer here
            glo.UartMsg.index = 0;              // reset the buffer for the next guy
            GateSwi_leave(glo.BIOS->MSGWriteGate, gatekey);
            return 'y';
        default:
            if (index < MAXLEN - 3)
            {
                glo.UartMsg.msgBuf[index] = input;
                glo.UartMsg.index++;            //increase msg len
                GateSwi_leave(glo.BIOS->MSGWriteGate, gatekey);
                return 'n';
            }
            else
            {
                memset(glo.UartMsg.msgBuf, 0, MAXLEN);
                glo.UartMsg.index=0;    //reset when overflow
                GateSwi_leave(glo.BIOS->MSGWriteGate, gatekey);
                return 'e';
            }
    }
}

// Command parser to map commands to function calls. - Call only in tasks
void ParseBuf(char payload[], uint32_t lenPayload)
{
   /*Process the buffer*/
   char message[MAXLEN] = {0};
   // Make a safe copy of the payload into payload
   strncpy(message, payload, sizeof(message) - 1); // Leave room for null-terminator

    // CLI CMD mapping
    if (message != NULL)
    {
        //about cmd
        if (!strncmp(message, "-about", strlen("-about"))) {
            PrintAbout(glo.terminal_out, sizeof(glo.terminal_out));
            UART_write(glo.DEVICES.uart0, glo.terminal_out, strlen(glo.terminal_out));
        }
        //help cmd
        else if (!strncmp(message, "-help", strlen("-help"))) {
            HelpCMD(message);
        }
        //print cmd
        else if (!strncmp(message, "-print", strlen("-print"))) {
            PrintCMD(message, glo.terminal_out, sizeof(glo.terminal_out));
        }
        //memr cmd
        else if (!strncmp(message, "-memr", strlen("-memr"))) {
            MemrCMD(message, glo.terminal_out, sizeof(glo.terminal_out), &BADMEM);
        }
        //gpio cmd
        else if(!strncmp(message, "-gpio", strlen("-gpio"))){
            GPIOCMD(message);
        }
        //errors cmd
        else if(!strncmp(message, "-error", strlen("-error")))
            PrintErrs(message);
        //callback cmd
        else if(!strncmp(message, "-callback", strlen("-callback")))
            CallbackCMD(message, glo.terminal_out, sizeof(glo.terminal_out));
        else if(!strncmp(message, "-timer", strlen("-timer")))
            TimerCMD(message);
        else
        {
            if (strlen(message))
            {
                BADCMD.count++;  //increase the errors occurred in operations
                UART_write(glo.DEVICES.uart0, "Command not recognized\n\r", sizeof("Command not recognized\n\r"));
            }
        }
    }
    memset(message, 0, MAXLEN); // clears the message, leaves payload intact
}

// Process the string coming in by walking to the next non whitespace.
char *GetNxtStr(char * input, bool AllWhites)
{
    char *token;
    if(input == NULL) return NULL;
    token = strchr(input, ' '); //get place of first space
    if (token == NULL) return NULL; //no more string
    //remove white spaces
    if(AllWhites)
        while(*token == ' ') token++;
    else
        token++;    //move to next spot (for print)
    if(!*token)
        return NULL;
    return token;   //return the rest of the string to be handled
}

/*
 * Add a payload to the payload queue - |Should be safe for staging|
 */
int32_t AddPayload(char *payload)
{
    int32_t nextPayload=0;    //to track the nextQueue Insertion
    int32_t index=0;          //Current payload in queue
    uint32_t gateKey;       //block resource access

    if(!payload || *payload == 0)
        return -1;

    index = glo.PayloadQueue.PayloadWriting;
    gateKey = GateSwi_enter(glo.BIOS->PayloadWriteGate);

    nextPayload = index+1;
    // Circular queue, so if payload exceeds, roll over
    if (nextPayload >= QUEUELEN)
        nextPayload = 0;
    // If the writing has caught up to the reading, write the error and drop the payload
    if (nextPayload == glo.PayloadQueue.PayloadReading)
    {
        QUEOVF.count++;
        glo.PayloadQueue.DroppedMessage = true; //signal that a issue occured with the message
    }
    else
    {
        // Clean insertion of the payload into the queue
        strcpy(glo.PayloadQueue.payloads[index], payload);
        glo.PayloadQueue.PayloadWriting = nextPayload;  //stage next insertion
    }
    GateSwi_leave(glo.BIOS->PayloadWriteGate, gateKey);
    Semaphore_post(glo.BIOS->PayloadSem);   // send a signal that resource (queue) is ready to be serviced
    return index;
}
