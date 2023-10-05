#include "GoodFortune.h"
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <xdc/runtime/System.h>//to fix the shit TI sucks at

//Extern errors
extern error BFROVF;
extern error BADCMD;
extern error BADMEM;
extern error BADGPIO;
extern error QUEOVF;

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

/* GPIO Command  - working */
void GPIOCMD(char * INPUT){
    size_t i;
    char* token = GetNxtStr(INPUT, true); // This moves the pointer along to the next argument
    char buffer[MAXLEN];
    memset(buffer, 0, sizeof(buffer));

    //Read all pins and display
    if (token == NULL)
    {
        //Define local vars
        const char MSG[] = "\n\rGPIO:\n\r";
        int32_t space_left = sizeof(glo.terminal_out) - strlen(MSG) - 1; // Account for newline and null-terminator

        if (space_left <= 1) {
            glo.terminal_out[0] = '\0';
            return;
        }
        strcpy(glo.terminal_out, MSG);
        //itterate through pins and add them to the buffer
        for (i = 0; i < GPIO_USED && space_left > 0; i++) {
            int32_t chars_written = System_snprintf(glo.terminal_out + strlen(glo.terminal_out), space_left,
                                         "GPIO Pin %i: %s | %i\n\r",
                                         glo.PINS[i]->pin,
                                         glo.PINS[i]->attachment,
                                         GPIO_read(glo.PINS[i]->pin));

            space_left = (chars_written >= 0 && chars_written < space_left) ? space_left - chars_written : 0;
        }
        glo.terminal_out[sizeof(glo.terminal_out) - 1] = 0;
        UART_write(glo.DEVICES.uart0, glo.terminal_out ,strlen(glo.terminal_out));
        memset(glo.terminal_out, 0, sizeof(glo.terminal_out));
        return;
    }
    else {
        // Parse and process the -gpio [pin] [flags] [values] command
        uint32_t pin = *token - '0';    //convert the gpio pin to an integer (char -> int)

        if (pin>=GPIO_USED) {
            // Conversion failed or there are non-numeric characters
            snprintf(glo.terminal_out, sizeof(glo.terminal_out), "\n\rInvalid GPIO pin number.\n\r");
            UART_write(glo.DEVICES.uart0, glo.terminal_out ,strlen(glo.terminal_out));
            goto GFYS;
        }

        if (pin < GPIO_USED) {
            token = GetNxtStr(token, true); // Move to the flags part
            if (token != NULL)
            {
                switch(*token)
                {
                case 'r':
                    System_snprintf(glo.terminal_out, sizeof(glo.terminal_out),
                             "\n\rGPIO:\n\rGPIO Pin %i: %s | %i\n\r",
                            glo.PINS[pin]->pin,
                            glo.PINS[pin]->attachment,
                            GPIO_read(glo.PINS[pin]->pin));

                    UART_write(glo.DEVICES.uart0, glo.terminal_out ,strlen(glo.terminal_out));
                    break;
                case 'w':
                    token = GetNxtStr(token, true); // Move to the values part
                    if (token != NULL) {
                        if (*token<'0' || *token > '9')
                        {
                            goto GFYS;
                        }
                        i = *token - '0'; // Convert the pin part to a long integer

                        if (pin != LSW || pin != RSW)
                            GPIO_write(glo.PINS[pin]->pin, i); //write the value to the pin
                        else goto GFYS;
                    }
                    break;
                case 't':
                        i = GPIO_read(glo.PINS[pin]->pin) ^ 1; //toggle using eor
                        GPIO_write(glo.PINS[pin]->pin, i);
                    break;
                default:
                    UART_write(glo.DEVICES.uart0, "\n\rInvalid Flag.\n\r", strlen("\n\rInvalid Flag.\n\r"));
                    goto GFYS;
                }
                return;
            }
            // Pin flag not specified
            else{
                System_snprintf(glo.terminal_out, sizeof(glo.terminal_out),
                                             "\n\rGPIO:\n\rGPIO Pin %i: %s | %i\n\r",
                                            glo.PINS[pin]->pin,
                                            glo.PINS[pin]->attachment,
                                            GPIO_read(glo.PINS[pin]->pin));
            }
            UART_write(glo.DEVICES.uart0, glo.terminal_out, strlen(glo.terminal_out));
            memset(glo.terminal_out, 0, sizeof(glo.terminal_out));
            return;
        }

        else {
            // Invalid pin number
            UART_write(glo.DEVICES.uart0, "\n\rInvalid GPIO pin number.\n\r", strlen("\n\rInvalid GPIO pin number.\n\r"));
            goto GFYS;
        }
    }

GFYS:
    BADGPIO.count++;
    return;
}
