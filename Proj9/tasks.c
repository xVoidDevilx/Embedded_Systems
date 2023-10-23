//Standard imports
#include <stdint.h>
#include <stddef.h>
/* My custom header file */
#include "GoodFortune.h"

//Extern errors
extern error BFROVF;

static const char WELCOMEMSG[] = "\n\r"
",--.         ,--.,--. ,--. ,---. ,--------.\n\r"
"|  |         |  ||  | |  |'   .-''--.  .--'\n\r"
"|  |    ,--. |  ||  | |  |`.  `-.   |  |\n\r"
"|  |    |  '-'  /'  '-'  '.-'    |  |  |\n\r"
"`--'     `-----'  `-----' `-----'   `--'\n\r"
",------. ,------.  ,---.  ,--.   ,--.,--.   ,--.      ,-.,----.\n\r"
"|  .--. '|  .---' /  O  \\ |  |   |  | \\  `.'  /      /  /'.-.  |\n\r"
"|  '--'.'|  `--, |  .-.  ||  |   |  |  '.    /      /  /   .' <\n\r"
"|  |\\  \\ |  `---.|  | |  ||  '--.|  '--. |  |       \\  \\ /'-'  |\n\r"
"`--' '--'`------'`--' `--'`-----'`-----' `--'        \\  `----'\n\r"
"                                                      `-'\n\r"
"  ,--.           ,-----.    ,--.    ,--.  ,-----.   ,--.,------.,-----. \n\r"
" /    \\,--.  ,--.|  |) /_  /    \\  /    \\ |  |) /_ /   ||  .---'|  .--' \n\r"
"|  ()  | \\  `.'  |  .-.  ||  ()  ||  ()  ||  .-.  `|  ||  `--, '--. `\\ \n\r"
" \\    / /  /.  \\ |  '--' / \\    /  \\    / |  '--' / |  ||  `---..--'  / \n\r"
"  `--' '--'  '--'`------'   `--'    `--'  `------'  `--'`------'`----'\n\r";

// Monitor the UART 0 port, process incoming characters into the Msg Buffer
void TSKUARTRead (void *arg0){
    //init shit
    memset(glo.terminal_out, 0, sizeof(glo.terminal_out));
    char input, proc;
    UART_write(glo.DEVICES.uart0, WELCOMEMSG, sizeof(WELCOMEMSG));
    for (;;) {
        //Echo typing
        UART_read(glo.DEVICES.uart0, &input, 1);
        // manage char from putty
        proc = UartAddByte(input);
        UART_write(glo.DEVICES.uart0, &input, 1);

        switch (proc)
        {
            // Not processing
            case 'n':
                break;
            // Overflow occurred
            case 'e':
                ErrorOut(&BFROVF, "Queue Overflow occured");
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

// Monitor the UART 7 port, process incoming characters into the Msg Buffer
void TSKUART7Read (void *arg0){
    //init shit
    char rxBuf[MAXLEN] = {0};
    int_fast32_t len;
    for (;;) {
        len = UART_read(glo.DEVICES.uart7, rxBuf, sizeof(rxBuf));
        if (len >1 && len<MAXLEN)
        {
            rxBuf[len-1] = 0;
            UART_write(glo.DEVICES.uart0, "UART7>", strlen("UART7>"));
            UART_write(glo.DEVICES.uart0, rxBuf, strlen(rxBuf));
            UART_write(glo.DEVICES.uart0, "\r\n", strlen("\r\n"));
            AddPayload(rxBuf);
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
        }
}
