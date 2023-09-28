#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <xdc/runtime/System.h>//to fix the shit TI sucks at

#include "GoodFortune.h"

/* Command Declarations */
cmd aboutCMD = {
    .name = "-about",
    .description = "Get information about the program currently running."
};

cmd helpCMD = {
    .name = "-help",
    .description = "Display available commands & their descriptions."
};

cmd printCMD = {
    .name = "-print",
    .description = "Echo a string back to console."
};

cmd memrCMD = {
    .name = "-memr",
    .description = "Read memory contents and echo to terminal\n\r"
            "\t\t\t0x00000000-0x000FFFFF: FLASH\n\r"
            "\t\t\t0x20000000-0x2003FFFF: SRAM\n\r"
            "\t\t\t0x40000000-0x44054FFF: Peripherals"
};
cmd gpioCMD = {
    .name = "-gpio",
    .description = "Read, write, or toggle GPIO pins: -gpio [pin] [flag] [values]\n\r"
            "\t\t\tgpio 0-3: D1-D4\n\r"
            "\t\t\tgpio 4: PK5\n\r"
            "\t\t\tgpio 5: PD4\n\r"
            "\t\t\tgpio 6: Left Switch\n\r"
            "\t\t\tgpio 7: Right Switch\n\r"
};
cmd errorCMD = {
    .name = "-error",
    .description = "Printing the user faults since startup. -error c to clear."
};

cmd timerCMD = {
    .name = "-timer",
    .description = "Configure the timer period (microseconds)\n\r"
            "\t\t\t-timer [>=100us period]\n\r"
            "\t\t\t-timer 0 to stop"
};

cmd callbackCMD = {
    .name = "-callback",
    .description = "configure callback: -callback [0-3] [times] [payload]\n\r"
            "\t\t\tCallback 0: timer\n\r"
            "\t\t\tCallback 1: LSW\n\r"
            "\t\t\tCallback 2: RSW\n\r"
};

cmd tickerCMD = {
    .name = "-ticker",
    .description = "Conf. tickers 0-15: -ticker [i] [del] [prd] [cnt] [payload]\n\r"
            "\t\t\tTicker every 10ms: del * 10 ms => prd x 10ms\n\r"
            "\t\t\t-ticker c|clear: clear the tickers\n\r"
            "\t\t\ti = index | del = delay | prd = period | cnt = count\n\r"
};
/////////////////////////////////////////////Functions in the system/////////////////////////////////////////////////

// Prints supported commands and details to the screen - Working!!!
void HelpCMD(char* command)
{
    size_t i = 0;
    char* token = GetNxtStr(command, true);

    if (token == NULL)
    {
        const char MSG[] = "\n\rAvailable Commands:\n\r";
        UART_write(glo.DEVICES.uart0, MSG, strlen(MSG));

        // Display all available commands and their descriptions
        for (i = 0; i < SUPPORTEDCMDS; i++) {
            System_snprintf(glo.terminal_out, MAXLEN,
                "%-8s | %s\n\r",
                glo.COMMANDS[i]->name,
                glo.COMMANDS[i]->description);
            UART_write(glo.DEVICES.uart0, glo.terminal_out, strlen(glo.terminal_out));
            memset(glo.terminal_out, 0, sizeof(glo.terminal_out));
        }
        return;
    }
    else
    {
        // Display help for the specified command(s)
        const char MSG[] = "\n\rHelp Menu:\n\r";
        UART_write(glo.DEVICES.uart0, MSG, strlen(MSG));

        // Iterate through commands and add them to the help_buffer if the command name is a substring of the token
        for (i = 0; i < SUPPORTEDCMDS; i++)
        {
            if (strstr(token, glo.COMMANDS[i]->name + 1) != NULL)
            {
                System_snprintf(glo.terminal_out, MAXLEN,
                    "\t%-8s | %s\n\r",
                    glo.COMMANDS[i]->name,
                    glo.COMMANDS[i]->description);
                UART_write(glo.DEVICES.uart0, glo.terminal_out, strlen(glo.terminal_out));
                memset(glo.terminal_out, 0, sizeof(glo.terminal_out));
            }
        }
    }
}




// Print information about the build of the program - Working!
void PrintAbout(char* OUTPUT, size_t buffer_size) {
    //Define local vars
    size_t space_left = buffer_size - 2; // Account for newline and null-terminator
    int chars_made = snprintf(OUTPUT, space_left, "\r\n\tEngineer: %20s"
                            "\n\r\tDate | Time: %13s | %s"
                            "\n\r\tVersion: %9.1f"
                            "\n\r\tAssignment %d: %12s\n\r", "Silas Rodriguez",__DATE__, __TIME__, 0.5, 5,"Tickers");

    // Ensure null-termination
    OUTPUT[buffer_size - space_left - (size_t)chars_made-1] = '\0';
    UART_write(glo.DEVICES.uart0, OUTPUT, strlen(OUTPUT));
    memset(OUTPUT, 0,sizeof(OUTPUT));
}

// Echo characters to console - Working!
void PrintCMD (char *buffer, char result[], size_t len) {
    char * token;
    token = GetNxtStr(buffer, false); //update the token - Protected because Print was here
    //add formatters
    strncat(result, "\n\r", len);
    strcpy(result, token);
    //add formatters
    strncat(result, "\n\r", len);
    //ensure null termination
    result[len-1] = 0;
    UART_write(glo.DEVICES.uart0, result, strlen(result));
    memset(result, 0,sizeof(result));
}

// Read a memory location and print contents of surrounding space   -  Working!!!
void MemrCMD(char *addrHex, char OUTPUT[], size_t bufflen, error* err) {
    addrHex = GetNxtStr(addrHex, true); //update the token after -memr

    uint32_t memaddr;   // actual memory location
    int32_t value;      // value in address
    char *ptr;          // string part of addrHex
    OUTPUT[bufflen - 1] = 0;    // ensure null
    const char MSG[] = "\n\rMEMR:\n\r";
    int32_t space_left = bufflen - strlen(OUTPUT) - strlen(MSG);

    if (addrHex == NULL) {
        memaddr = 0; // default memspace
    }

    memaddr = 0xFFFFFFF0 & strtol(addrHex, &ptr, 16);   // MASK LS bits to print 16 bytes of data
    if (memaddr > 0x100000 && memaddr < 0x20000000) goto MEMERR;    // too high for flash, too low for SRAM
    else if (memaddr > 0x20040000 && memaddr < 40000000) goto MEMERR;  // too high for SRAM too low for peripheral
    else if (memaddr > 0x44055000) goto MEMERR; // above peripherals
    else {
        if (space_left < 3) {
            strncpy(OUTPUT, "\n\r\0", 3);   // newline
            err->count++;
            return;
        }
        strncpy(OUTPUT, MSG, space_left);   // copy the message into the output
        int i;

        // Single loop to add addresses and values
        for (i = 0; i <= 0xF; i+=4) {
            // Add the address to the OUTPUT string if there's enough space
            space_left = bufflen - strlen(OUTPUT);
            if (space_left <= 0) {
                break;  // No more space in the OUTPUT buffer
            }
            snprintf(OUTPUT + strlen(OUTPUT), space_left, "Address 0x%08X: ", memaddr + i);

            // Add the value to the same line
            space_left = bufflen - strlen(OUTPUT);
            if (space_left <= 0) {
                break;  // No more space in the OUTPUT buffer
            }
            // Get memaddr + i location, type cast to 32-bit
            value = *(int32_t *)(memaddr + i);
            snprintf(OUTPUT + strlen(OUTPUT), space_left, "%08X\n\r", value);
        }
        UART_write(glo.DEVICES.uart0, OUTPUT, strlen(OUTPUT));
        memset(OUTPUT, 0 , sizeof(OUTPUT));
        return;

    MEMERR:
        err->count++;
        addrHex[16] = 0;    // ensure null termination of Hex value
        System_snprintf(OUTPUT, bufflen, "Hex address %s out of allowable range. Use -help memr to see range.\n\r", addrHex);
        UART_write(glo.DEVICES.uart0, OUTPUT, strlen(OUTPUT));
        memset(OUTPUT, 0 , sizeof(OUTPUT));
    }
}

//Tokenize the incoming string, assign the callback with index to a new payload - working!
void CallbackCMD(char *input, char *output, size_t outLen)
{
    char service[MAXLEN] = {0};
    // Parse the input string to get callback parameters
    char *token = GetNxtStr(input, true), *endptr;  //  -callback [id] [times] [payload] - > [id] [times] [payload]
    uint32_t i = 0;
    //print all callbacks
    if (token == NULL)
    {
        for(i = 0; i<NUMCALLBACKS; i++){
            System_snprintf(output, outLen, "Callback [%d] | Count: [%d] | Payload: %s\n\r",
                            i, glo.callbacks[i]->count, glo.callbacks[i]->payload);
            UART_write(glo.DEVICES.uart0, output, strlen(output));
        }
        memset(output, 0, sizeof(output));
        return;
    }
    //check for clear condition
    if(!strncmp(token, "clear", strlen("clear")) || *token == 'c')
    {
        for(i = 0; i<NUMCALLBACKS; i++)
        {
            glo.callbacks[i]->count = 0;
            glo.callbacks[i]->HWI_Flag = false;
            memset(glo.callbacks[i]->payload, 0, sizeof(glo.callbacks[i]->payload));
        }
        return;
    }
    // Parse callback index
    int32_t index = *token - '0';   //convert char [id] -> int
    if (index < 0 || index >= NUMCALLBACKS)
    {
        // Invalid callback index, return an error message
        snprintf(output, outLen, "Invalid callback index. Must be between 0 and %zu\n\r", NUMCALLBACKS - 1);
        return;
    }

    // Parse for times
    token = GetNxtStr(token, true); //[id] [times] [payload] -> [count] [payload]
    // No count, just print
    if (token == NULL) {
        System_snprintf(output, outLen, "Callback [%d] | Count: [%d] | Payload: %s\n\r",
                                    index, glo.callbacks[index]->count, glo.callbacks[index]->payload);
        UART_write(glo.DEVICES.uart0, output, strlen(output));
        return;
    }
    // -- To get the times, set an endptr to the next space, and subtract token from it, cpy to a buffer
    endptr = strchr(token, ' ');
    //just set the count if no payload
    if (endptr==NULL)
    {
        glo.callbacks[index]->count = atoi(token);  //return integer of the rest of the token
        return;
    }
    else
    {
        strncpy(service, token,(uint32_t) (endptr - token));    // move the count into a buffer without the space
        glo.callbacks[index]->count = atoi(service);  //return integer of the rest of the token
    }

    // Parse payload - if empty, clear payload
    token = GetNxtStr(token, true); // [count] [payload] -> [payload]
    // no payload
    if (token == NULL)
    {
        memset(glo.callbacks[index]->payload, 0, sizeof(glo.callbacks[index]->payload));
        return;
    }
    char *payload = token;

    // Update the specified callback
    glo.callbacks[index]->HWI_Flag = false;
    System_snprintf(glo.callbacks[index]->payload, sizeof(glo.callbacks[index]->payload), "%s", payload);
    glo.callbacks[index]->payload[sizeof(glo.callbacks[index]->payload) - 1] = 0;   //terminate in case
}

// Reconfigure the timer period or display it to the serial port - Working
void TimerCMD(char *input) {
    // Parse the input to get the desired period value
    char *token = GetNxtStr(input, true);
    if (token == NULL) {
        // If input is null, print the current timer period
        char response[MAXLEN];
        System_snprintf(response, sizeof(response), "Current Timer Period: %d microseconds\n\r", glo.DEVICES.timer0_period);
        UART_write(glo.DEVICES.uart0, response, strlen(response));
        return;
    }

    // Convert the input string to an integer representing the period
    int period_us = atoi(token);

    // Check if the period value is valid
    if (period_us ==0)
    {
        Timer_stop(glo.DEVICES.timer0); //stop the timer
        UART_write(glo.DEVICES.uart0, "Timer Off\n\r", strlen("Timer Off\n\r"));
        glo.DEVICES.timer0_period = 0xFFFFFFFF; //(infinite)
        return;
    }
    else if(period_us < 100)    //from testing, <100 us is perty scurry
    {
        UART_write(glo.DEVICES.uart0, "Invalid timer period value\n\r", sizeof("Invalid timer period value\n\r"));
        return;
    }

    // Configure the existing timer with the new period using Timer_setPeriod
    else
    {
        Timer_stop(glo.DEVICES.timer0); //stop the timer
        if (Timer_setPeriod(glo.DEVICES.timer0, Timer_PERIOD_US, (uint32_t)period_us) != Timer_STATUS_SUCCESS)
        {
            UART_write(glo.DEVICES.uart0, "Failed to configure timer\n\r", sizeof("Failed to configure timer\n\r"));
            return;
        }
        Timer_start(glo.DEVICES.timer0);    //restart the timer
    }

    // Update the current timer period variable
    glo.DEVICES.timer0_period = period_us;

    // Inform the user about the timer configuration
    char response[MAXLEN];
    System_snprintf(response, sizeof(response), "Timer period set to %d microseconds\n\r", period_us);
    UART_write(glo.DEVICES.uart0, response, strlen(response));
}

//Tokenize the incoming string, assign the callback with index to a new payload - working!
void TickerCMD(char *input)
{
    char service[MAXLEN] = {0};
    // Parse the input string to get callback parameters
    char *token = GetNxtStr(input, true), *endptr;  //  -ticker [index] [del] [prd] [cnt] [payload]
    uint32_t i=0, timeArgs=0;                                   //-> [index] [del] [prd] [cnt] [payload]
    // NULL case: print tickers
    if(token == NULL)
    {
        for(i=0; i<NUMTICKERS; i++)
        {
            snprintf(glo.terminal_out, MAXLEN, "TICKER %d: d: %d | p: %d | c: %d | payload: %s\n\r",
                            i, glo.tickers[i].delay, glo.tickers[i].period, glo.tickers[i].count, glo.tickers[i].payload);
            UART_write(glo.DEVICES.uart0, glo.terminal_out, strlen(glo.terminal_out));
            memset(glo.terminal_out, 0, sizeof(glo.terminal_out));
        }
        return;
    }
    // Clear all the tickers
    else if(*token == 'c' || *token=='C')
    {
        for(i=0; i<NUMTICKERS; i++)
        {
            glo.tickers[i].count=0;
            glo.tickers[i].period=0;
            glo.tickers[i].delay=0;
            memset(glo.tickers[i].payload, 0, sizeof(glo.tickers[i].payload));
        }
        return;
    }
    endptr = strchr(token, ' ');            //get the next space
    if (endptr)
        strncpy(service, token, endptr-token);  // copy the index target
    else
        strncpy(service, token, strlen(token));
    i = atoi(service);                      // return an integer of the desired index
    token = GetNxtStr(token, true);         // -> [del] [prd] [cnt] [payload]
    // print just the target index if no other flags
    if(token == NULL)
    {
        System_snprintf(glo.terminal_out, MAXLEN, "TICKER %d: d: %d | p: %d | c: %d | payload: %s\n\r",
                        i, glo.tickers[i].delay, glo.tickers[i].period, glo.tickers[i].count, glo.tickers[i].payload);
        UART_write(glo.DEVICES.uart0, glo.terminal_out, strlen(glo.terminal_out));
        memset(glo.terminal_out, 0, sizeof(glo.terminal_out));
        return;
    }
    else if (*token == 'c' || *token == 'C')
    {
        glo.tickers[i].count=0;
        glo.tickers[i].period=0;
        glo.tickers[i].delay=0;
        memset(glo.tickers[i].payload, 0, sizeof(glo.tickers[i].payload));
        return;
    }
    endptr = strchr(token, ' ');            // get the next space
    if (endptr)
        strncpy(service, token, endptr-token);  // copy the index target
    else
        strncpy(service, token, strlen(token));
    timeArgs = atoi(service);               // return an integer of the desired delay
    glo.tickers[i].delay=timeArgs;          // set the delay, 0 will not run
    token = GetNxtStr(token, true);         // -> [prd] [cnt] [payload]
    if (token == NULL) return;

    endptr = strchr(token, ' ');            // get the next space
    if (endptr)
        strncpy(service, token, endptr-token);  // copy the index target
    else
        strncpy(service, token, strlen(token));
    timeArgs = atoi(service);               // return an integer of the desired period
    glo.tickers[i].period=timeArgs;         // set the period for the ticker event
    token = GetNxtStr(token, true);         // -> [cnt] [payload]
    if(token == NULL) return;

    endptr = strchr(token, ' ');            // get the next space
    if (endptr)
        strncpy(service, token, endptr-token);  // copy the index target
    else
        strncpy(service, token, strlen(token));
    int32_t count = atoi(service);               // return an integer of the desired count
    glo.tickers[i].count=count;         // set the count for the ticker event
    token = GetNxtStr(token, true);         // -> [payload]
    if(token == NULL) return;

    strncpy(glo.tickers[i].payload, token, MAXLEN); // set the payload
}
