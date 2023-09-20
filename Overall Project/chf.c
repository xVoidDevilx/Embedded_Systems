/*
 * chf.c
 *
 *  Created on: Sep 11, 2023
 *      Author: silas
 */
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <xdc/runtime/System.h>//to fix the shit TI sucks at

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>
#include "ti_drivers_config.h"
/* My custom header file */
#include "chf.h"


/*   Global Struct Declaration   */
global glo = {
              .integrity = 0xB00B1E5,   //mature, right?
              .terminal_in = "",
              .terminal_out = ""
};

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
    .description = "Read, write, or toggle GPIO pins: -gpio [pin] [flags] [values]\n\r"
            "\t\t\tgpio 0-3: D1-D4\n\r"
            "\t\t\tgpio 4: PK5\n\r"
            "\t\t\tgpio 5: PD4\n\r"
            "\t\t\tgpio 6: Left Switch\n\r"
            "\t\t\tgpio 7: Right Switch\n\r"
};
cmd errorCMD = {
    .name = "-error",
    .description = "Printing the faults of the user since startup. -error c to clear."
};
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
    .msg = "Attempted bad memory reads",
    .count = 0
};
/* GPIO Pin declarations */
gpio PIN0 = {
             .pin = 0,
             .attachment = "D1"
};
gpio PIN1 = {
             .pin = 1,
             .attachment = "D2"
};
gpio PIN2 = {
             .pin = 2,
             .attachment = "D3"
};
gpio PIN3 = {
             .pin = 3,
             .attachment = "D4"
};
gpio PIN4 = {
             .pin = 4,
             .attachment = "PK5"
};
gpio PIN5 = {
             .pin = 5,
             .attachment = "PD4"
};
gpio PIN6 = {
             .pin = 6,
             .attachment = "SW1"
};
gpio PIN7 = {
             .pin = 7,
             .attachment = "SW2"
};
/*   Function Implementations   */

// config the global struct
void GlobalConfig(global *glo){

    /* Call driver init functions */
    GPIO_init();
    UART_init();

    /* Configure the LED pin */
    GPIO_setConfig(CONFIG_GPIO_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_1, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_2, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_3, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    /* Create a UART with data processing off. */
    UART_Params_init(&(glo->uartParams));
    glo->uartParams.writeDataMode = UART_DATA_BINARY;
    glo->uartParams.readDataMode = UART_DATA_BINARY;
    glo->uartParams.readReturnMode = UART_RETURN_FULL;
    glo->uartParams.baudRate = 115200;

    glo->uart = UART_open(CONFIG_UART_0, &glo->uartParams);

    if (glo->uart == NULL) {
        /* UART_open() failed */
        while (1);
    }
    //Organize Commands
    glo->COMMANDS[0] = &aboutCMD;
    glo->COMMANDS[1] = &helpCMD;
    glo->COMMANDS[2] = &printCMD;
    glo->COMMANDS[3] = &memrCMD;
    glo->COMMANDS[4] = &gpioCMD;
    glo->COMMANDS[5] = &errorCMD;
    //Organize GPIO
    glo->PINS[0] = &PIN0;
    glo->PINS[1] = &PIN1;
    glo->PINS[2] = &PIN2;
    glo->PINS[3] = &PIN3;
    glo->PINS[4] = &PIN4;
    glo->PINS[5] = &PIN5;
    glo->PINS[6] = &PIN6;
    glo->PINS[7] = &PIN7;
    //Organize Errors
    glo->ERRORS[0] = &BFROVF;
    glo->ERRORS[1] = &BADCMD;
    glo->ERRORS[2] = &BADMEM;
    /* Turn on user LED */
    GPIO_write(glo->PINS[0]->pin, CONFIG_GPIO_LED_ON);
    GPIO_write(glo->PINS[1]->pin, CONFIG_GPIO_LED_ON);
    GPIO_write(glo->PINS[2]->pin, CONFIG_GPIO_LED_ON);
    GPIO_write(glo->PINS[3]->pin, CONFIG_GPIO_LED_ON);
}

/* Process inputs, update global buffers, and return the signal to continue the program*/
char ProcInp(char input, size_t bufflen, global *glo){
    glo->terminal_in[--bufflen] = 0;  //ensure null terminator
    int32_t i = (int32_t) strlen(glo->terminal_in); //get the position of the latest character, not the cursor
    i = i>0 ? i:0;  //ensure 0 or more

    // Switch based on input
    switch (input) {
        case '\b':
        case BACKSPACE:
            glo->terminal_in[--i] = 0;
            return 'n';
        case '\n':
        case '\r':
            glo->terminal_in[++i] = 0;    //terminate the buffer here
            return 'y';
        default:
            if (i < bufflen - 3) glo->terminal_in[i] = input;
            else {
                strncpy(glo->terminal_out, "\n\rERROR: BFR OVF\n\r", sizeof(glo->terminal_out));
                memset(glo->terminal_in, 0, sizeof(glo->terminal_in));
                return 'e';
            }
            return 'n';
    }
}

/*
    modifies OUTPUT with formatted string of all the supported commands
*/
void HelpCMD(char* INPUT, global *glo) {
    size_t i=0;
    INPUT = strtok(NULL, " -\n\r"); //update input token - Protected because -help MUST be in the string currently

    // process just -help
    if (INPUT == NULL) {
        //Define local vars
        const char MSG[] = "\n\rAvailable Commands:\n\r";
        int32_t space_left = sizeof(glo->terminal_out) - strlen(MSG) - 1; // Account for newline and null-terminator

        if (space_left <= 1) {
            glo->terminal_out[0] = '\0';
            return;
        }
        strcpy(glo->terminal_out, MSG);
        //itterate through commands and add them to the buffer
        for (i = 0; i < SUPPORTED && space_left > 0; i++) {
            int32_t chars_written = System_snprintf(glo->terminal_out + strlen(glo->terminal_out), space_left,
                                         "\t%-8s | %s\n\r",
                                         glo->COMMANDS[i]->name,
                                         glo->COMMANDS[i]->description);

            space_left = (chars_written >= 0 && chars_written < space_left) ? space_left - chars_written : 0;
        }
        glo->terminal_out[sizeof(glo->terminal_out) - 1] = 0;
    }
    // process certain commands
    else {
        // Define local vars
        const char MSG[] = "\n\rHelp Menu:\n\r";
        int32_t space_left = sizeof(glo->terminal_out) - strlen(MSG) - 3; // Account for newline and null-terminator

        if (space_left <= 1) {
            glo->terminal_out[0] = '\0';
            return;
        }
        strcpy(glo->terminal_out, MSG);
        //itterate through commands and add them to the buffer
        while (INPUT != NULL){
            for (i = 0; i < SUPPORTED && space_left > 0; i++) {
                if (strstr(glo->COMMANDS[i]->name, INPUT)){
                    int32_t chars_written = System_snprintf(glo->terminal_out + strlen(glo->terminal_out), space_left,
                                                 "\t%-8s | %s\n\r",
                                                 glo->COMMANDS[i]->name,
                                                 glo->COMMANDS[i]->description);

                    space_left = (chars_written >= 0 && chars_written < space_left) ? space_left - chars_written : 0;
                }
            }
            INPUT = strtok(NULL, " -\n\r");
        }
        glo->terminal_out[sizeof(glo->terminal_out) - 1] = 0;
    }
}

/*
 *  Print information about the build of the program
 */
void PrintAbout(char* OUTPUT, size_t buffer_size) {
    //Define local vars
    size_t space_left = buffer_size - 2; // Account for newline and null-terminator
    int chars_made = snprintf(OUTPUT, space_left, "\r\n\tEngineer: %20s"
                            "\n\r\tDate | Time: %13s | %s"
                            "\n\r\tVersion: %9.1f"
                            "\n\r\tAssignment %d: %12s\n\r", "Silas Rodriguez",__DATE__, __TIME__, 0.4, 4,"Callback & Timers");

    // Ensure null-termination
    OUTPUT[buffer_size - space_left - (size_t)chars_made-1] = '\0';
}

/* Print command called */
void PrintCMD (char *buffer, char result[], size_t len) {

    buffer = strtok(NULL, " \n\r"); //update the token - Protected because Print was here

    while(buffer != NULL) {
        //exit with max print
        if (strlen(buffer) > len - strlen(result) - 3){ //include \n \r 0
            break;
        }
        //Copy the contents over + move on
        strncat(result, buffer, len);
        strncat(result, " ", len);
        buffer = strtok(NULL, " \r\n");
    }
    //add formatters
    strncat(result, "\n\r", len);
    //ensure null termination
    result[len-1] = 0;
}

/* Memr command called */
void MemrCMD(char *addrHex, char OUTPUT[], size_t bufflen, error* err) {
    addrHex = strtok(NULL, " \n\r"); //update the token after -memr

    uint32_t memaddr;   // actual memory location
    int32_t value;      // value in address
    char *ptr;          // string part of addrHex
    OUTPUT[bufflen - 1] = 0;    // ensure null
    const char MSG[] = "MEMR:\n\r";
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
            System_snprintf(OUTPUT + strlen(OUTPUT), space_left, "Address 0x%08X: ", memaddr + i);

            // Add the value to the same line
            space_left = bufflen - strlen(OUTPUT);
            if (space_left <= 0) {
                break;  // No more space in the OUTPUT buffer
            }
            value = *(int32_t *)(memaddr + i); // Get memaddr + i location, type cast to 32-bit
            System_snprintf(OUTPUT + strlen(OUTPUT), space_left, "%08X\n\r", value);
        }
        return;

    MEMERR:
        err->count++;
        addrHex[16] = 0;    // ensure null termination of Hex value
        System_snprintf(OUTPUT, bufflen, "Hex address %s out of allowable range. Use -help memr to see range.\n\r", addrHex);
    }
}

/* GPIO Command */
void GPIOCMD(char * INPUT, global *glo, error *BADCMD){
    size_t i;
    INPUT = strtok(NULL, " -\n\r"); // This moves the pointer along to the next argument
    char buffer[MAXLEN];
    memset(buffer, 0, sizeof(buffer));

    //Read all pins and display
    if (INPUT == NULL) {
        //Define local vars
        const char MSG[] = "\n\rGPIO:\n\r";
        int32_t space_left = sizeof(glo->terminal_out) - strlen(MSG) - 1; // Account for newline and null-terminator

        if (space_left <= 1) {
            glo->terminal_out[0] = '\0';
            return;
        }
        strcpy(glo->terminal_out, MSG);
        //itterate through commands and add them to the buffer
        for (i = 0; i < GPIO_USED && space_left > 0; i++) {
            int32_t chars_written = System_snprintf(glo->terminal_out + strlen(glo->terminal_out), space_left,
                                         "\tGPIO Pin %i: %s | %i\n\r",
                                         glo->PINS[i]->pin,
                                         glo->PINS[i]->attachment,
                                         GPIO_read(glo->PINS[i]->pin));

            space_left = (chars_written >= 0 && chars_written < space_left) ? space_left - chars_written : 0;
        }
        glo->terminal_out[sizeof(glo->terminal_out) - 1] = 0;
    }
    else {
        // Parse and process the -gpio [pin] [flags] [values] command
        char *endptr; // Pointer to the character that stopped the scan
        long pin = strtol(INPUT, &endptr, 10); // Convert the pin part to a long integer

        if (endptr == INPUT || *endptr != 0) {
            // Conversion failed or there are non-numeric characters
            snprintf(glo->terminal_out, sizeof(glo->terminal_out), "\n\rInvalid GPIO pin number.\n\r");
            goto GFYS;
        }

        if (pin >= 0 && pin < GPIO_USED) {
            INPUT = strtok(NULL, " -\n\r"); // Move to the flags part
            if (INPUT != NULL) {
                switch(INPUT[0]) {
                case 'r':
                    System_snprintf(glo->terminal_out, sizeof(glo->terminal_out),
                             "\n\rGPIO:\n\r\tGPIO Pin %i: %s | %i\n\r",
                            glo->PINS[pin]->pin,
                            glo->PINS[pin]->attachment,
                            GPIO_read(glo->PINS[pin]->pin));
                    break;
                case 'w':
                    INPUT = strtok(NULL, " -\n\r"); // Move to the values part
                    if (INPUT != NULL) {
                        i = strtol(INPUT, &endptr, 10); // Convert the pin part to a long integer
                        if (endptr == INPUT || *endptr != 0) {
                            // Conversion failed or there are non-numeric characters
                            snprintf(glo->terminal_out, sizeof(glo->terminal_out), "\n\rInvalid Write.\n\r");
                            goto GFYS;
                        }
                        if (pin != LSW || pin != RSW)
                        GPIO_write(glo->PINS[pin]->pin, i); //write the value to the pin
                        else goto GFYS;
                    }
                    break;
                case 't':
                        i = GPIO_read(glo->PINS[pin]->pin) ^ 1; //toggle using eor
                        GPIO_write(glo->PINS[pin]->pin, i);
                    break;
                default:
                    snprintf(glo->terminal_out, sizeof(glo->terminal_out), "\n\rInvalid Flag.\n\r");
                    goto GFYS;
                }
            }
            // Pin flag not specified
            else{
                System_snprintf(glo->terminal_out, sizeof(glo->terminal_out),
                                             "\n\rGPIO:\n\r\tGPIO Pin %i: %s | %i\n\r",
                                            glo->PINS[pin]->pin,
                                            glo->PINS[pin]->attachment,
                                            GPIO_read(glo->PINS[pin]->pin));
            }
        }
        else {
            // Invalid pin number
            snprintf(glo->terminal_out, sizeof(glo->terminal_out), "\n\rInvalid GPIO pin number.\n\r");
            goto GFYS;
        }
    }
GFYS:
    BADCMD->count++;
    return;
}
/* Print Errors to the console*/
void PrintErrs(char *INPUT, global *glo){
    size_t i;
    INPUT = strtok(NULL, " -\n\r");
    if (INPUT == NULL){
        UART_write(glo->uart, "User Errors: (Because the programmer isn't wrong)\n\r", sizeof("User Errors: (Because the programmer isn't wrong)\n\r"));
        for (i=0; i<ERRTYPES; i++){
            System_snprintf(glo->terminal_out, sizeof(glo->terminal_out), "Name: %-15s | ID: %i | %-40s | %i\n\r", glo->ERRORS[i]->name, glo->ERRORS[i]->id, glo->ERRORS[i]->msg, glo->ERRORS[i]->count);
            UART_write(glo->uart, glo->terminal_out, strlen(glo->terminal_out));
        }
    }
    if (INPUT[0] == 'c'){
        for (i=0; i<ERRTYPES; i++){
            glo->ERRORS[i]->count=0;
        }
    }

}
