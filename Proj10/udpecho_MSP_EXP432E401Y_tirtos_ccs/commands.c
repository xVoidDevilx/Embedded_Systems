#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <math.h>

#include <xdc/runtime/System.h>//to fix the shit TI sucks at

#include "GoodFortune.h"

//Extern errors
extern error BFROVF;
extern error BADCMD;
extern error BADMEM;
extern error BADGPIO;
extern error QUEOVF;
extern error BADMTH;
extern error SCRPTER;
extern error ADCERR;
extern error VoiceERR;
extern error StreamERR;
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
                            "\n\r\tAssignment %d: %12s\n\r", "Silas Rodriguez",__DATE__, __TIME__, 1.0, 11,"Local Voice");

    // Ensure null-termination
    OUTPUT[buffer_size - space_left - (size_t)chars_made-1] = '\0';
    UART_write(glo.DEVICES.uart0, OUTPUT, strlen(OUTPUT));
    memset(OUTPUT, 0,sizeof(OUTPUT));
}

// Echo characters to console - Working!
void PrintCMD (char *message) {
    char * token;
    unsigned int len0 = 0;
    token = GetNxtStr(message, false); //update the token - Protected because Print was here
    strcat(token, "\n\r");
    len0 = strlen(token);
    UART_write(glo.DEVICES.uart0, token, len0);
}

void clearCMD ()
{
    unsigned int i;
    for (i = 0; i<2048; i++)
        UART_write(glo.DEVICES.uart0, "\n", 1); // write a bunch of newlines to "clear" the console
    UART_write(glo.DEVICES.uart0, "\r", 1);
}

// Read a memory location and print contents of surrounding space   -  Working!!!
void MemrCMD(char *addrHex) {
    addrHex = GetNxtStr(addrHex, true); //update the token after -memr

    uint32_t memaddr;   // actual memory location
    int32_t value;      // value in address
    char *ptr;          // string part of addrHex
    const char MSG[] = "\n\rMEMR:\n\r";
    UART_write(glo.DEVICES.uart0, MSG, strlen(MSG));
    if (addrHex == NULL)
        memaddr = 0; // default memspace

    memaddr = 0xFFFFFFF0 & strtol(addrHex, &ptr, 16);   // MASK LS bits to print 16 bytes of data
    if (memaddr > 0x100000 && memaddr < 0x20000000) goto MEMERR;    // too high for flash, too low for SRAM
    else if (memaddr > 0x20040000 && memaddr < 40000000) goto MEMERR;  // too high for SRAM too low for peripheral
    else if (memaddr > 0x44055000) goto MEMERR; // above peripherals
    else {
        int i;
        // Single loop to add addresses and values
        for (i = 0; i <= 0xF; i+=4) {
            // Get memaddr + i location, type cast to 32-bit
            value = *(int32_t *)(memaddr + i);
            // Add the address to the output string
            snprintf(glo.terminal_out, sizeof(glo.terminal_out), "Address 0x%08X: 0x%08x\n\r", memaddr + i, value);
            // write out
            UART_write(glo.DEVICES.uart0, glo.terminal_out, strlen(glo.terminal_out));
        }
        memset(glo.terminal_out,0, sizeof(glo.terminal_out));
        return;

    MEMERR:
        addrHex[16] = 0;    // ensure null termination of Hex value
        System_snprintf(glo.terminal_out, sizeof(glo.terminal_out), "Hex address %s out of allowable range", addrHex);
        ErrorOut(&BADMEM, glo.terminal_out);
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
        System_snprintf(output, outLen, "Invalid callback index. Must be between 0 and %d\n\r", NUMCALLBACKS - 1);
        ErrorOut(&BADGPIO, output);
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
        System_snprintf(response, sizeof(response), "-print Current Timer Period: %d microseconds\n\r", glo.DEVICES.timer0_period);
        AddPayload(response);
        return;
    }

    // Convert the input string to an integer representing the period
    int period_us = atoi(token);

    // Check if the period value is valid
    if (period_us ==0)
    {
        Timer_stop(glo.DEVICES.timer0); //stop the timer
        glo.DEVICES.timer0_period = 0xFFFFFFFF; //(infinite)
        return;
    }
    else if(period_us < 100)    //from testing, <100 us is perty scurry
    {
        ErrorOut(&BADGPIO, "Invalid timer period value");
        return;
    }

    // Configure the existing timer with the new period using Timer_setPeriod
    else
    {
        Timer_stop(glo.DEVICES.timer0); //stop the timer
        if (Timer_setPeriod(glo.DEVICES.timer0, Timer_PERIOD_US, (uint32_t)period_us) != Timer_STATUS_SUCCESS)
        {
            ErrorOut(&BADGPIO, "Failed to configure timer");
            return;
        }
        Timer_start(glo.DEVICES.timer0);    //restart the timer
    }

    // Update the current timer period variable
    glo.DEVICES.timer0_period = period_us;

    // Inform the user about the timer configuration
    char response[MAXLEN];
    System_snprintf(response, sizeof(response), "-print Timer period set to %d microseconds\n\r", period_us);
    AddPayload(response);
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
                    ErrorOut(&BADMTH, "Invalid Register Access");
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
                ErrorOut(&BADMTH, "Invalid Single Reg OP");
                return;
        }
        goto REGEXIT;   //print updated value
    }
    //Get the litsrc we first expect
    int32_t litsrc0=0, litsrc1=0;
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
            ErrorOut(&BADMTH, "Invalid Single litsrc OP");
            return;
        default:
            break;
        }
    }
    else
    {
        ErrorOut(&BADMTH, "Expected Litsrc");
        return;
    }
    // Get the litsrc:
    litsrc1= ExtractLitSrc(token);

    token = GetNxtStr(token, true); // walk to the OP [op]
    if(token == NULL)
    {
        ErrorOut(&BADMTH, "Missing Operation");
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
                ErrorOut(&BADMTH, "Cannot Div by 0");
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
                ErrorOut(&BADMTH, "Cannot Div by 0");
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
            ErrorOut(&BADMTH, "Invalid 2 litsrc Operation");
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
            ErrorOut(&BADMTH, "Expected Litsrc");
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
            ErrorOut(&SCRPTER, "Invalid script space");
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
    if (i >= NUMSCRIPTS)
    {
        ErrorOut(&SCRPTER, "Invalid script space");
        return;
    }
    token = GetNxtStr(token, true);         // walks to the next space: [payload/flags]

    //process payloads or flags:
    switch(*token)
    {
        // execute script space
        case 'X':
            while(*glo.scripts[i] != 0)
            {
                AddPayload(glo.scripts[i]);
                i++;
            }
            break;
        // execute a script index
        case 'x':
            AddPayload(glo.scripts[i]);
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
            memset(glo.scripts[i], 0, sizeof(glo.scripts[i]));   // ensure rest of string is terminated
            strncpy(glo.scripts[i], token, strlen(token));    // copy the payload in
            glo.scripts[i][sizeof(glo.scripts[i])-1] = 0;     //ensure null termination
            break;
        // default error - not a valid payload or flag
        default:
            ErrorOut(&SCRPTER, "Error, invalid flag or payload provided");
            break;
    }
}

/**
 * Conditional payload processor function/ take an input conditional, and assign a payload based on its result
 */
void condCMD (char * input)
{
    char *token = GetNxtStr(input, true);       // [cond] ? [true]:[false] => [cond] = [litsrc0] <cmp> [litsrc1]

    // If -if is called by itself, print a help for -if for usage
    if (token == NULL)
    {
        HelpCMD("-help if");
        return;
    }
    // extract litsrc 0 , cmp, and litsrc 1
    int32_t litsrc0=0, litsrc1=0;
    char cmp;

    litsrc0 = ExtractLitSrc(token);
    token = GetNxtStr(token, true);   //walk over
    if (token == NULL)
    {
        ErrorOut(&SCRPTER, "Conditional missing cond: [<,=,>]");
        return;
    }
    // check for appropriate comparitor
    switch (*token)
    {
        case '<':
            cmp = '<';
            break;
        case '>':
            cmp = '>';
            break;
        case '=':
            cmp = '=';
            break;
        // error out
        default:
            ErrorOut(&SCRPTER, "Invalid condition: [<,=,>]");
            return;
    }
    token = GetNxtStr(token,true);   //walk over to litsrc 1
    if (token == NULL)
    {
        ErrorOut(&SCRPTER, "Missing second operand");
        return;
    }
    litsrc1 = ExtractLitSrc(token); // get the litsrc otherwise - all operands acquired
    token = GetNxtStr(token, true); // go to ?
    if (*token != '?')
    {
        ErrorOut(&SCRPTER, "Missing ternary op '?'");
        return;
    }
    token = GetNxtStr(token, true);     // walk to payload 1
    if (token == NULL)
    {
        ErrorOut(&SCRPTER, "Missing payloads");
        return;
    }

    char *ptr = strchr(token, ':'); //get semicolon sep
    if (ptr == NULL)
    {
        ErrorOut(&SCRPTER, "Missing payloads delim ':'");
        return;
    }

    char payloadt[MAXLEN]={0}, payloadf[MAXLEN]={0};
    strncpy(payloadt, token, ptr-token);
    payloadt[MAXLEN-1] = 0;         // ensure null termination

    token = ++ptr;                  // move to delim + 1
    token = GetNxtStr(token, true); // move the token along

    // try to check for valid payloads
    if(*ptr=='-')
        strncpy(payloadf, ptr, strlen(ptr));
    else if (*token=='-')
        strncpy(payloadf, token, strlen(token));
    else
        payloadf[0]=0;

    payloadf[MAXLEN-1] = 0; //ensure null termination

    // payloadt/f extracted: can map condtitional statements
    switch(cmp)
    {
    case '<':
        if (litsrc0 < litsrc1)
            AddPayload(payloadt);
        else
            AddPayload(payloadf);
        break;
    case '>':
        if (litsrc0 > litsrc1)
            AddPayload(payloadt);
        else
            AddPayload(payloadf);
        break;
    case '=':
        if (litsrc0 == litsrc1)
            AddPayload(payloadt);
        else
            AddPayload(payloadf);
        break;
    // error out - shouldnt be here, cmp got changed
    default:
        //Log conditional error
        ErrorOut(&SCRPTER, "FATAL: cmp overwritten");
        return;
    }
}

/**
 * @param Message: string containing frequency
 */
void sineCMD(char * message)
{
    double lowerweight, upperweight;
    uint32_t lowerindex, upperindex;
    double answer;
    uint16_t outputPower = 0;

    SPI_Transaction spiTransaction;
    bool transferOK;

    char *token = GetNxtStr(message, true);

    if(glo.DEVICES.timer0_period == 0)
    {
        AddPayload("-print Timer 0 off");
        return;
    }
    if(token && *token != 0 && glo.DEVICES.timer0_period > 0)
    {
        uint32_t freq = atoi(token);
        glo.LUTctrl.deltalut = (double) freq * (double) LUTSIZE * (double) glo.DEVICES.timer0_period / 1000000.;
        if (freq == 0)
        {
            //SineERR.count++;
            AddPayload("-print Missing required digits\n\r");
            return;
        }
    }
    if (glo.LUTctrl.deltalut >= LUTSIZE/2)
    {
        glo.LUTctrl.deltalut = 0;
        AddPayload("-print Nyquist violation");
        TimerCMD("-timer 0");
    }
    if (glo.LUTctrl.deltalut <= 0)
    {
        glo.LUTctrl.deltalut = 0;
        if(glo.DEVICES.timer0_period > 0)
        {
            TimerCMD("-timer 0");
            CallbackCMD("-callback 0 c", glo.terminal_out, sizeof(glo.terminal_out));
        }
        AddPayload("-print Timer 0 off");
        return;
    }
    // there was a number, deltalut re-calculated, do not want SPI
    if (token)
        return;

    lowerindex = (uint32_t) glo.LUTctrl.lutposition;
    upperindex = lowerindex + 1;
    upperweight = glo.LUTctrl.lutposition - (double) lowerindex;
    lowerweight = 1. - upperweight;
    answer = (double) glo.LUTctrl.sinLUT[lowerindex] * lowerweight + (double) glo.LUTctrl.sinLUT[upperindex] * upperweight;
    outputPower = round(answer);

    spiTransaction.count = 1;
    spiTransaction.txBuf = (void *) &outputPower;
    spiTransaction.rxBuf = (void *) NULL;

    transferOK = SPI_transfer(glo.DEVICES.spi3, &spiTransaction);
    if (!transferOK)
        while (1);
    glo.LUTctrl.lutposition +=glo.LUTctrl.deltalut;
    while(glo.LUTctrl.lutposition >= (double) LUTSIZE)
        glo.LUTctrl.lutposition -= (double) LUTSIZE;
}

/**
 * @param payload: string / command to be delivered via uart7
 */
void UartCMD(char * payload)
{
    char *token = GetNxtStr(payload, true);                 // goes to the beginning of the payload to be delivered
    if (token != NULL)
    {
        strcat(token, "\n\r");
        UART_write(glo.DEVICES.uart7, token, strlen(token));
    }
}

void VoiceCMD (char * payload)
{
    int32_t dest_choice, bufflen;
    uint16_t *dest;
    char *loc;

    loc = GetNxtStr(payload, true);
    if (loc == NULL)
        return;
    dest_choice = atoi(loc);

    loc = GetNxtStr(loc, true);
    bufflen = atoi(loc);
    if (bufflen != DATABLOCKSIZE)
        VoiceERR.count++;
    loc = GetNxtStr(loc, false);
    loc++;
    loc++;

    if(dest_choice ==0)
        dest = glo.TxRx.TX_Ping;
    else if (dest_choice == 1)
        dest = glo.TxRx.TX_Pong;

    memcpy(dest, loc, sizeof(uint16_t) * DATABLOCKSIZE);

    if (glo.DEVICES.adcbufctrl.TX_completed == NULL)
    {
        glo.DEVICES.adcbufctrl.TX_completed = dest;
        glo.DEVICES.adcbufctrl.TX_index = 0;
    }
}   // voice end

void StreamCMD(char * message)
{
    char *token;
    uint32_t intent;

    token = GetNxtStr(message, true);
    // token was empty
    if (token == NULL)
    {
        switch(glo.DEVICES.adcbufctrl.converting)
        {
        case 0:
            intent = 1;
            break;
        case 1:
            intent = 2;
            break;
        case 2:
            intent = 0;
            break;
        }
    }
    // token had an intent
    else
    {
        switch (*token)
        {
        case '2':
            intent =2;
            break;
        case '1':
        case 't':
            intent =1;
            break;
        case '0':
        case 'f':
            intent = 0;
            break;
        }
    }
    if (intent == glo.DEVICES.adcbufctrl.converting)
        return;
    if (intent == 0)
    {
        if(glo.DEVICES.adcbufctrl.converting == 2)
        {
            if (ADCBuf_convertCancel(glo.DEVICES.adcBuf)!= ADCBuf_STATUS_SUCCESS)
                ErrorOut(&StreamERR , "ADC cancel failed");
        }
        glo.DEVICES.adcbufctrl.converting=0;
        if(glo.LUTctrl.deltalut > 0.)       // check on this
            sineCMD("-sine 0");
        CallbackCMD("-callback 0 c", glo.terminal_out, sizeof(glo.terminal_out));
        glo.DEVICES.adcbufctrl.RX_completed = NULL;
        glo.DEVICES.adcbufctrl.TX_completed = NULL;
        glo.DEVICES.adcbufctrl.TX_index = -1;
    }
    // Microphone stream
    else if (intent == 1 && glo.DEVICES.adcbufctrl.converting == 0)
    {
        GPIOCMD("-gpio 4 w 0"); // speaker en
        GPIOCMD("-gpio 5 w 1"); // mic en
        //CallbackCMD("-callback 0 -1 -audio");
        TimerCMD("-timer 125");
        glo.DEVICES.adcbufctrl.converting = 1;
        TickerCMD("-tickers 14 100 0 0 -stream 2");
        TickerCMD("-tickers 15 105 0 0 -callback 0 -1 -audio");
    }
    else if (intent == 2 && glo.DEVICES.adcbufctrl.converting == 1)
    {
        if (ADCBuf_convert(glo.DEVICES.adcBuf, &glo.DEVICES.adcbufctrl.conversion, 1)!= ADCBuf_STATUS_SUCCESS)
            ErrorOut(&ADCERR, "ADC Convert Failed");
        else
        {
            glo.DEVICES.adcbufctrl.converting = 2;
            glo.DEVICES.adcbufctrl.RX_completed = NULL;
            glo.DEVICES.adcbufctrl.TX_completed = NULL;
            glo.DEVICES.adcbufctrl.TX_index = -1;
            glo.DEVICES.adcbufctrl.delay = 4;
        }
    }
}   // End of streamCMD

void AudioCMD(char *message)
{
    uint16_t outval;
    SPI_Transaction spiTrans;
    bool transferOK;

    if(glo.DEVICES.adcbufctrl.TX_completed != NULL && glo.DEVICES.adcbufctrl.TX_index >=0
            && glo.DEVICES.adcbufctrl.delay > 0)
    {
        glo.DEVICES.adcbufctrl.delay--;
        return;
    }
    outval = glo.DEVICES.adcbufctrl.TX_completed[glo.DEVICES.adcbufctrl.TX_index++];

    spiTrans.count = 1;
    spiTrans.txBuf = (void *) &outval;
    spiTrans.rxBuf = (void *) NULL;

    transferOK = SPI_transfer(glo.DEVICES.spi3, &spiTrans);
    glo.DEVICES.adcbufctrl.sample_count++;
    if (!transferOK)
        while (1);

    if(glo.DEVICES.adcbufctrl.TX_index >= DATABLOCKSIZE)
    {
        glo.DEVICES.adcbufctrl.TX_index = 0;
        glo.DEVICES.adcbufctrl.TX_completed = glo.DEVICES.adcbufctrl.TX_completed == glo.TxRx.TX_Ping ? glo.TxRx.TX_Pong:glo.TxRx.TX_Ping;
    }
}   // end of audioCMD
