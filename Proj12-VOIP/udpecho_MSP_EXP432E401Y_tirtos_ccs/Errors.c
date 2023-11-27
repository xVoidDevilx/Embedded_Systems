#include "GoodFortune.h"
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <xdc/runtime/System.h>

static const char AIRPLANE[] = "\n\r"
"              ##### | #####\n\r"
"             # _ _ #|# _ _ #\n\r"
"       |       ############\n\r"
"|                  # #\n\r"
"       |     |    #   #      |        |\n\r"
"                 #( O )#    |    |     |\n\r"
"|  ################. .###############  |\n\r"
" ##  _ _|____|     ###     |_ __| _  ##\n\r"
"#  |            43,000 ft           |  #\n\r"
"#  |    |    |    |   |    |    |   |  #\n\r"
" ######################################\n\r"
"                 #     #\n\r"
"                  #####\n\r"
"             OOOOOOO|OOOOOOO\n\r"
"                    U\n\r";



/* Error handlers */
error BFROVF = {
     .id = 0,
     .name = "Buffer Overflow",
     .msg = "Times the input buffer has overflowed",
     .count = 0
};

error BADCMD = {
    .id = 1,
    .name = "Bad Command",
    .msg = "Times an unknown command was entered",
    .count = 0
};

error BADMEM = {
    .id = 2,
    .name = "Bad Memory",
    .msg = "Attempted fatal memory reads",
    .count = 0
};

error BADGPIO = {
    .id = 3,
    .name = "GPIO access ERR",
    .msg = "Times illegal GPIO access occured",
    .count = 0
};

error QUEOVF = {
    .id = 4,
    .name = "Queue Collision",
    .msg = "Times an read/write collisions occurred",
    .count = 0
};

error BADMTH = {
    .id = 5,
    .name = "REG Math ERR",
    .msg = "Illegal -regs operations attempted",
    .count = 0
};

error SCRPTER = {
     .id=6,
     .name="Script Errors",
     .msg="Script space/ifs reference errors",
     .count=0
};

error ADCERR = {
     .id=7,
     .name="ADC Error",
     .msg="Various Audio Stream errors",
     .count=0
};

error VoiceERR = {
     .id=8,
     .name="Voice Error",
     .msg="Various Voice Stream errors",
     .count=0
};

error StreamERR = {
     .id=9,
     .name="Stream Error",
     .msg="Various Stream errors",
     .count=0
};

error NETERR = {
     .id=10,
     .name="Network Error",
     .msg= "Errors involving network",
     .count=0
};

/* Print Errors to the console    -   Working*/
void PrintErrs(char *INPUT){
    size_t i;
    char msgbuf[MAXLEN<<2];
    char *token = GetNxtStr(INPUT, true);
    if (token == NULL){
        UART_write(glo.DEVICES.uart0, AIRPLANE, strlen(AIRPLANE));
        UART_write(glo.DEVICES.uart0, "Why the Plane Crashed:\n\r", strlen("Why the Plane Crashed:\n\r"));
        for (i=0; i<ERRTYPES; i++){
            System_snprintf(msgbuf, MAXLEN, "Name: %-15s | ID: %2i | %-40s | %i\n\r", glo.ERRORS[i]->name, glo.ERRORS[i]->id, glo.ERRORS[i]->msg, glo.ERRORS[i]->count);
            UART_write(glo.DEVICES.uart0, msgbuf, strlen(msgbuf));
        }
    }
    if (token[0] == 'c'){
        for (i=0; i<ERRTYPES; i++)
            glo.ERRORS[i]->count=0;
    }
}

/**
 * Update the value within an error and echo some message back to the user
 */
void ErrorOut(error *ERR, char* details)
{
    ERR->count++;
    char msgbuf[MAXLEN];
    System_snprintf(msgbuf, MAXLEN, "-print \n\r%s (%d): %s\n\r", ERR->name, ERR->id, details);
    AddPayload(msgbuf);
}
