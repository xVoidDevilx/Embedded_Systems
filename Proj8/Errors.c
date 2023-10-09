#include "GoodFortune.h"
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <xdc/runtime/System.h>//to fix the shit TI sucks at

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


/* Print Errors to the console    -   Working*/
void PrintErrs(char *INPUT){
    size_t i;
    char *token = GetNxtStr(INPUT, true);
    if (token == NULL){
        UART_write(glo.DEVICES.uart0, "\n\rUser Errors: (Because the programmer isn't wrong)\n\r", sizeof("\n\rUser Errors: (Because the programmer isn't wrong)\n\r"));
        for (i=0; i<ERRTYPES; i++){
            System_snprintf(glo.terminal_out, sizeof(glo.terminal_out), "Name: %-15s | ID: %i | %-40s | %i\n\r", glo.ERRORS[i]->name, glo.ERRORS[i]->id, glo.ERRORS[i]->msg, glo.ERRORS[i]->count);
            UART_write(glo.DEVICES.uart0, glo.terminal_out, strlen(glo.terminal_out));
        }
    }
    if (token[0] == 'c'){
        for (i=0; i<ERRTYPES; i++){
            glo.ERRORS[i]->count=0;
        }
    }
}

/**
 * Update the value within an error and echo some message back to the user
 */
void ErrorOut(error ERR, char* details)
{
    ERR.count++;
    System_snprintf(glo.terminal_out, sizeof(glo.terminal_out), "%s: %s\n\r", ERR.name, details);
    glo.terminal_out[sizeof(glo.terminal_out)-1] = 0;   //ensure null termination
    UART_write(glo.DEVICES.uart0, glo.terminal_out, strlen(glo.terminal_out));
    memset(glo.terminal_out, 0, sizeof(glo.terminal_out));
}
