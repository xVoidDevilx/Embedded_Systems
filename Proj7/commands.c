#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

#include <xdc/runtime/System.h>//to fix the shit TI sucks at

#include "GoodFortune.h"

extern error BADMTH;
extern error BADCMD;

/* Command Declarations */
cmd aboutCMD = {
    .name = "-about",
    .description = "Get information about the program currently running.\n\r"
};

cmd helpCMD = {
    .name = "-help",
    .description = "Display available commands & their descriptions.\n\r"
};

cmd printCMD = {
    .name = "-print",
    .description = "Echo a string back to console.\n\r"
};

cmd memrCMD = {
    .name = "-memr",
    .description = "Read memory contents and echo to terminal\n\r"
            "\t\t\t0x00000000-0x000FFFFF: FLASH\n\r"
            "\t\t\t0x20000000-0x2003FFFF: SRAM\n\r"
            "\t\t\t0x40000000-0x44054FFF: Peripherals\n\r"
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
    .description = "Printing the user faults since startup. -error c to clear.\n\r"
};

cmd timerCMD = {
    .name = "-timer",
    .description = "Configure the timer period (microseconds)\n\r"
            "\t\t\t-timer [>=100us period]\n\r"
            "\t\t\t-timer 0 to stop\n\r"
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

cmd regCMD = {
    .name = "-regs",
    .description = "Perform basic operations with memory. i=dst\n\r"
            "\t\t\t-regs [i] [#lit/src/op] [#lit/src] [op]\n\r"
            "\t\t\t-regs [i] [#lit/src] : copy values into i\n\r"
            "\t\t\tSupported OPs:\n\r"
            "\t\t\t +  : Add |  -   : Sub/Neg\n\r"
            "\t\t\t *  : Mul |  /   : Div\n\r"
            "\t\t\t $  : ExC | [~!] : Not\n\r"
            "\t\t\t ++ : Inc | --   : Dec\n\r"
            "\t\t\t ^  : XOR |  |   : OR\n\r"
            "\t\t\t &  : AND |  =   : CPY\n\r"
            "\t\t\t >  : MAX |  <   : MIN\n\r"
};

cmd remCMD = {
    .name = "-rem",
    .description = "Leaves a comment; Doesn't execute; Doesn't complain\n\r"
};

cmd scriptCMD = {
    .name = "-script",
    .description = "Load a script space with a payload, or execute a script space.\n\r"
            "\t\t\t-script [id] [payload/flag]\n\r"
            "\t\t\tflags: r=read index X|x=execute\n\r"
            "\t\t\tflags: c=clear index C=clear space\n\r"
            "\t\t\tflags: R=read space\n\r"
            "\t\t\tScripts execute until null space\n\r"
            "\t\t\t64 Script spaces are available...\n\r"
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
            System_snprintf(glo.terminal_out, sizeof(glo.terminal_out),
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
                System_snprintf(glo.terminal_out, sizeof(glo.terminal_out),
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
                            "\n\r\tAssignment %d: %12s\n\r", "Silas Rodriguez",__DATE__, __TIME__, 0.7, 7,"Scripts");

    // Ensure null-termination
    OUTPUT[buffer_size - space_left - (size_t)chars_made-1] = '\0';
    UART_write(glo.DEVICES.uart0, OUTPUT, strlen(OUTPUT));
    memset(OUTPUT, 0,sizeof(OUTPUT));
}

// Echo characters to console - Working!
void PrintCMD (char *buffer, char result[], size_t len) {
    char * token;
    token = GetNxtStr(buffer, false); //update the token - Protected because Print was here
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
    {
        strncpy(service, token, endptr-token);  // copy the index target
        service[endptr-token] = 0;
    }
    else
    {
        strncpy(service, token, strlen(token));
        service[strlen(token)] = 0;
    }

    timeArgs = atoi(service);               // return an integer of the desired delay
    glo.tickers[i].delay=timeArgs;          // set the delay, 0 will not run
    token = GetNxtStr(token, true);         // -> [prd] [cnt] [payload]
    if (token == NULL) return;

    endptr = strchr(token, ' ');            // get the next space
    if (endptr)
    {
        strncpy(service, token, endptr-token);  // copy the index target
        service[endptr-token] = 0;
    }
    else
    {
        strncpy(service, token, strlen(token));
        service[strlen(token)] = 0;
    }
    timeArgs = atoi(service);               // return an integer of the desired period
    glo.tickers[i].period=timeArgs;         // set the period for the ticker event
    token = GetNxtStr(token, true);         // -> [cnt] [payload]
    if(token == NULL) return;

    endptr = strchr(token, ' ');            // get the next space
    if (endptr)
    {
        strncpy(service, token, endptr-token);  // copy the index target
        service[endptr-token] = 0;
    }
    else
    {
        strncpy(service, token, strlen(token));
        service[strlen(token)] = 0;
    }
    int32_t count = atoi(service);               // return an integer of the desired count
    glo.tickers[i].count=count;                 // set the count for the ticker event
    token = GetNxtStr(token, true);             // -> [payload]
    if(token == NULL) return;

    strncpy(glo.tickers[i].payload, token, MAXLEN); // set the payload
}

/*
 * Register command that allows you to perform basic arithmetic with integers
 */
void RegCMD(char *input)
{
    char * token = GetNxtStr(input, true); // [i] [litsrc/op] [litsrc] [op]
    size_t i = 0;

    // print register values and exit
    if(token == NULL)
    {
        const char MSG [] = "Registers: \n\r";
        UART_write(glo.DEVICES.uart0, MSG, strlen(MSG));
        for (i=0; i<NUMREG; i+=2)
        {
            snprintf(glo.terminal_out, sizeof(glo.terminal_out), "REG %2d: %11d (0x%08X) | REG %2d: %11d (0x%08X)\n\r", i, glo.REGISTERS[i],
                     glo.REGISTERS[i], i+1, glo.REGISTERS[i+1], glo.REGISTERS[i+1]);
            UART_write(glo.DEVICES.uart0, glo.terminal_out, strlen(glo.terminal_out));
        }
        memset(glo.terminal_out, 0, sizeof(glo.terminal_out));
        return;
    }
    //reset registers
    else if (*token == 'c')
    {
        for (i=0; i<NUMREG; i++)
            glo.REGISTERS[i] = 0;

        UART_write(glo.DEVICES.uart0, "Registers Cleared Sucessfully\n\r", strlen("Registers Cleared Sucessfully\n\r"));
        return;
    }
    // Parse for a dst
    char *endptr = strchr(token, ' ');  // go to next delim
    uint32_t reginx =0;

    // Just the reg index was passed
    if (endptr == NULL)
    {
        reginx = atoi(token);
        // this is to print after later executions
        REGEXIT:
        snprintf(glo.terminal_out, sizeof(glo.terminal_out), "REG %2d: %d (0x%08X)\n\r", reginx, glo.REGISTERS[reginx],
                                     glo.REGISTERS[reginx]);
        UART_write(glo.DEVICES.uart0, glo.terminal_out, strlen(glo.terminal_out));
        memset(glo.terminal_out, 0, sizeof(glo.terminal_out));
        return;
    }

    char clone[MAXLEN];                        // clone string for processing parts of a string
    memset(clone, 0, sizeof(clone));
    strncpy(clone, token, endptr-token);
    reginx = atoi(clone);                       //copy the index being referenced
    token = GetNxtStr(token, true);             // walk to next string space - [#lit/src/op] [litsrc] [op]

    // parse for operation - or another number
    endptr = strchr(token, ' '); // look for the next space
    uint32_t index=0;
    int32_t litsrc0=0, litsrc1=0;
    char *ptr;
    index = atoi(token);    // get the index of the register
    if (endptr == NULL)
    {
        switch (*token)
        {
            //bitwise not (invert dst)
            case '!':
            case '~':
                glo.REGISTERS[reginx] = ~glo.REGISTERS[reginx];
                break;
            //neg or dec
            case '-':
                if (*(++token) == '-')
                {
                    glo.REGISTERS[reginx]--;
                    break;
                }
                glo.REGISTERS[reginx] = -1*glo.REGISTERS[reginx];
                break;
            // cpy a literal into reg index
            case '#':
                token++;    //walk the token ahead by one to pass #
                // check what type of literal being passed: hex or dec
                if (strncmp(token, "0x", 2) ==0 || strncmp(token, "0X", 2) == 0)
                    glo.REGISTERS[reginx] = (int32_t) strtoul(token, &ptr, 16);      // pass the number as base 16
                else
                    glo.REGISTERS[reginx] = (int32_t) strtoul(token, &ptr, 10);      // pass the number as a decimal
                break;

            // copy a register
            default:
                if (index >= NUMREG)
                {
                    BADMTH.count++;
                    UART_write(glo.DEVICES.uart0, "Invalid Register Access\n\r", strlen("Invalid Register Access\n\r"));
                    return;
                }
                glo.REGISTERS[reginx] = glo.REGISTERS[index];   //copy the register directly
                break;

            // increment
            case '+':
                if(*(++token) == '+')
                {
                    glo.REGISTERS[reginx]++;
                    goto REGEXIT;
                }// if this check fails, error out
            //check for operators and fail out
            case '*':
            case '/':
            case '^':
            case '=':
            case '%':
            case '$':
            case '&':
            case '|':
                BADMTH.count++;
                UART_write(glo.DEVICES.uart0, "Invalid Single Reg OP\n\r", strlen("Invalid Single Reg OP\n\r"));
                return;
        }
        goto REGEXIT;   //print updated value
    }
    //Get the litsrc we first expect
    else
        litsrc0 = ExtractLitSrc(token);

    token = GetNxtStr(token, true);            // walk to next string space - [litsrc] / [op]
    // Determine if [litsrc] [op] or error
    if (token != NULL)
    {
        switch(*token)
        {
        //exchange two values
        case '$':
            glo.REGISTERS[index] = glo.REGISTERS[reginx];
            glo.REGISTERS[reginx] = litsrc0;
            goto REGEXIT;
        // copy op
        case '=':
            glo.REGISTERS[reginx] = litsrc0;
            goto REGEXIT;
        //bitwise not
        case '!':
            glo.REGISTERS[reginx] = ~litsrc0;
            goto REGEXIT;

        case '+':
        case '-':
        case '*':
        case '/':
        case '^':
        case '%':
        case '&':
        case '|':
        case '~':
            BADMTH.count++;
            UART_write(glo.DEVICES.uart0, "Invalid Single litsrc OP\n\r", strlen("Invalid Single litsrc OP\n\r"));
            return;
        default:
            break;
        }
    }
    else
    {
        BADMTH.count++;
        const char MSG[] = "Expected Litsrc\n\r";
        UART_write(glo.DEVICES.uart0, MSG, strlen(MSG));
        return;
    }
    // Get the litsrc:
    litsrc1= ExtractLitSrc(token);

    token = GetNxtStr(token, true); // walk to the OP [op]
    if(token == NULL)
    {
        BADMTH.count++;
        const char MSG[] = "Missing Operation\n\r";
        UART_write(glo.DEVICES.uart0, MSG, strlen(MSG));
        return;
    }



    // There was a space, apply the operator:
    switch(*token)
    {
        //add
        case '+':
            glo.REGISTERS[reginx] = litsrc0 + litsrc1;
            break;
        //sub
        case '-':
            glo.REGISTERS[reginx] = litsrc0 - litsrc1;
            break;
        //mul
        case '*':
            glo.REGISTERS[reginx] = litsrc0 * litsrc1;
            break;
        //div
        case '/':
            if (litsrc1 == 0)
            {
                UART_write(glo.DEVICES.uart0, "Cannot Div by 0\n\r", strlen("Cannot Div by 0\n\r"));
                BADMTH.count++;
                return;
            }
            glo.REGISTERS[reginx] = litsrc0 / litsrc1;
            break;
        //xor
        case '^':
            glo.REGISTERS[reginx] = litsrc0 ^ litsrc1;
            break;
        //remainder
        case '%':
            if (litsrc1 == 0)
            {
                UART_write(glo.DEVICES.uart0, "Cannot Div by 0\n\r", strlen("Cannot Div by 0\n\r"));
                BADMTH.count++;
                return;
            }
            glo.REGISTERS[reginx] = litsrc0 % litsrc1;
            break;
        //bitwise and
        case '&':
            glo.REGISTERS[reginx] &= litsrc0 & litsrc1;
            break;
        //bitwise or
        case '|':
            glo.REGISTERS[reginx] = litsrc0 | litsrc1;
            break;
        // max or greater than op
        case '>':
            glo.REGISTERS[reginx] = litsrc0 > litsrc1 ? litsrc0:litsrc1;
            break;
        // min or less than op
        case '<':
            glo.REGISTERS[reginx] = litsrc0 < litsrc1 ? litsrc0:litsrc1;
            break;
        //not a valid operation
        default:
            BADMTH.count++;
            const char MSG[] = "Invalid 2 litsrc Operation\n\r";
            UART_write(glo.DEVICES.uart0, MSG, strlen(MSG));
            return;
    }
    goto REGEXIT;
}

int32_t ExtractLitSrc (char * token)
{
    int32_t litsrc = 0;
    uint32_t index = 0;
    char * ptr;
    // check for literal or src
    switch (*token)
    {
        //provided a literal
        case '#':
            token++;    //walk the token ahead by one to pass #
            // check what type of literal being passed: hex or dec
            if (strncmp(token, "0x", 2) ==0 || strncmp(token, "0X", 2) == 0)
                litsrc = strtol(token, &ptr, 16);      // pass the number as base 16
            else
                litsrc = strtol(token, &ptr, 10);      // pass the number as a decimal
            break;
        // provide a src from regs
        default:
            index = atoi(token);    // get the index of the register
            if (index >= NUMREG)
            {
                BADMTH.count++;
                const char MSG[] = "Invalid Register Access\n\r";
                UART_write(glo.DEVICES.uart0, MSG, strlen(MSG));
                break;
            }
            litsrc = glo.REGISTERS[index];   //copy the register directly
            break;
        // Provided an invalid litsrc
        case '+':
        case '-':
        case '*':
        case '/':
        case '^':
        case '=':
        case '%':
        case '>':
        case '<':
        case '$':
        case '&':
        case '|':
        case '!':
        case '~':
            BADMTH.count++;
            const char MSG[] = "Expected Litsrc\n\r";
            UART_write(glo.DEVICES.uart0, MSG, strlen(MSG));
            break;
    }
    return litsrc;
}

/**
 * This command will process a -scripts payload command
 */
void ScriptCMD(char *input)
{
    // copy -script [id] [payload,flag]       => [id] [payload, flag]
    char * token = GetNxtStr(input, true), *endptr;
    char cpymsg[MAXLEN] = {0};

    uint32_t i=0;

    // print the script space
    if (token == NULL)
    {
        for(i = 0; i<NUMSCRIPTS; i++)
        {
            System_snprintf(glo.terminal_out, sizeof(glo.terminal_out), "Script %d: %s\n\r", i, glo.scripts[i]);
            UART_write(glo.DEVICES.uart0, glo.terminal_out, strlen(glo.terminal_out));
        }
        memset(glo.terminal_out, 0 , sizeof(glo.terminal_out));
        return;
    }
    // given an id
    endptr = strchr(token, ' ');

    // just the ID was given
    if (endptr == NULL)
    {
        // Convert requested script to a number
        i = atoi(token);
    SCRIPTREAD_index:

        if (i >= NUMSCRIPTS)
        {
            BADCMD.count++;
            UART_write(glo.DEVICES.uart0, "ERROR: not supported script space access\n\r", strlen("ERROR: not supported script space access\n\r"));
            return;
        }
        // read the expected script by default
        System_snprintf(glo.terminal_out, sizeof(glo.terminal_out), "Script %d: %s\n\r", i, glo.scripts[i]);
        UART_write(glo.DEVICES.uart0, glo.terminal_out, strlen(glo.terminal_out));
        return;
    }

    // there was a space after id - copy the id, move to space
    strncpy(cpymsg, token, endptr-token);   // this gets the ID
    i = atoi(cpymsg);                       // get the ID of the script
    token = GetNxtStr(token, true);         // walks to the next space: [payload/flags]

    //process payloads or flags:
    switch(*token)
    {
        // execute script space
        case 'X':
        case 'x':
            while(*glo.scripts[i] != 0)
            {
                AddPayload(glo.scripts[i]);
                i++;
            }
            break;
        // read script space
        case 'R':
            while(*glo.scripts[i] != 0)
            {
                System_snprintf(glo.terminal_out, sizeof(glo.terminal_out), "Script %d: %s\n\r", i, glo.scripts[i]);
                UART_write(glo.DEVICES.uart0, glo.terminal_out, strlen(glo.terminal_out));
                i++;
            }
            memset(glo.terminal_out, 0 , sizeof(glo.terminal_out));
            break;
        // read a script index
        case 'r':
            goto SCRIPTREAD_index;
        // clear a script space
        case 'C':
            while(*glo.scripts[i] != 0)
            {
                memset(glo.scripts[i], 0, sizeof(glo.scripts[i]));
                i++;
            }
            break;
        // clear a script index
        case 'c':
            memset(glo.scripts[i], 0, sizeof(glo.scripts[i]));
            break;
        // add payload (no flag)
        case '-':
            strncpy(glo.scripts[i], token, strlen(token));    // copy the payload in
            glo.scripts[i][(MAXLEN>>2)-1] = 0;                //ensure null termination
            break;
        // default error - not a valid payload or flag
        default:
            BADCMD.count++;
            UART_write(glo.DEVICES.uart0, "Error, invalid flag or payload provided. Payloads start with -\n\r",
                       strlen("Error, invalid flag or payload provided. Payloads start with -\n\r"));
            break;
    }
}
