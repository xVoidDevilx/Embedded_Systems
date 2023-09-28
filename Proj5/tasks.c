//Standard imports
#include <stdint.h>
#include <stddef.h>
#include <string.h>
/* My custom header file */
#include "GoodFortune.h"

// Monitor the UART 0 port, process incoming characters into the Msg Buffer
void TSKUARTRead (void *arg0){
    //init shit
    memset(glo.terminal_out, 0, sizeof(glo.terminal_out));

    char input, proc;
    const char  echoPrompt[] = "MSP432 Ready:\r\n";

    InitializeDrivers();

    UART_write(glo.DEVICES.uart0, echoPrompt, strlen(echoPrompt));

    for (;;) {
        //Echo typing
        UART_read(glo.DEVICES.uart0, &input, 1);
        UART_write(glo.DEVICES.uart0, &input, 1);
        // manage char from putty
        proc = UartAddByte(input);

        switch (proc)
        {
            // Not processing
            case 'n':
                break;
            // Overflow occurred
            case 'e':
                BFROVF.count++;
                UART_write(glo.DEVICES.uart0, "\n\rBuffer Overflow\n\r", strlen("\n\rBuffer Overflow\n\r"));
                memset(glo.terminal_out, 0, sizeof(glo.terminal_out));
                break;
            case 'y':
                UART_write(glo.DEVICES.uart0, "\n\r", 2);
                //Semaphore_post(glo.BIOS->UART0ReadSem); //signal that there is shit in the queue needing serviced
                AddPayload(glo.UartMsg.msgBuf);
                // Clear buffers
                memset(glo.UartMsg.msgBuf, 0, MAXLEN);
                memset(glo.terminal_out, 0, sizeof(glo.terminal_out));
                break;
        }
    }
}

/*
 * This task is responsible for reading from the queue and processing Payloads
 */
void TSKPayload(void * arg0)
{
    int32_t index, nextPayload; //monitor the circular queue
    char *payload;              //payload to execute
    uint32_t gatekey;           //lock access to resources

        for(;;) //Ever
        {
            Semaphore_pend(glo.BIOS->PayloadSem, BIOS_WAIT_FOREVER);    //waits for external signals to run
            index = glo.PayloadQueue.PayloadReading;                    //Read the earliest payload
            payload = glo.PayloadQueue.payloads[index];                 //get the payload

            ParseBuf(payload, strlen(payload));                         // run the payload

            gatekey = GateSwi_enter(glo.BIOS->PayloadWriteGate);        // lock this resource
            nextPayload = index+1;                                      //track the next payload
            //circular operation
            if(nextPayload >= QUEUELEN)
                nextPayload = 0;
            //increment the payload to be processed
            glo.PayloadQueue.PayloadReading=nextPayload;

            GateSwi_leave(glo.BIOS->PayloadWriteGate, gatekey); //release the resources
            //Semaphore_post(glo.BIOS->UART0ReadSem);             // Let uartTSK know it is ok to monitor UART
        }
}

/* Pends on a semaphore, then pushes into payloads from queue*/
void TSKHandleMsQ (void *arg0)
{
//    for(;;)
//    {
//        Semaphore_pend(glo.BIOS->UART0ReadSem, BIOS_WAIT_FOREVER);
//        AddPayload(glo.MsgQ.Messages[--glo.MsgQ.MsgReading].msgBuf);
//        Semaphore_post(glo.BIOS->PayloadSem);   //let the next task know that there is a payload
//    }
}
