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
             .attachment = "D1 "
};
gpio PIN1 = {
             .pin = 1,
             .attachment = "D2 "
};
gpio PIN2 = {
             .pin = 2,
             .attachment = "D3 "
};
gpio PIN3 = {
             .pin = 3,
             .attachment = "D4 "
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
    char buffer[MAXLEN]= {0};

    //Read all pins and display
    if (token == NULL)
    {
        UART_write(glo.DEVICES.uart0,
                   "\n\rGPIO:\n\r",
                   strlen("\n\rGPIO:\n\r"));
        //itterate through pins and add them to the buffer
        for (i = 0; i < GPIO_USED; i++)
        {
            System_snprintf(buffer, MAXLEN,
                                         "GPIO Pin %i: %s | %i\n\r",
                                         glo.PINS[i]->pin,
                                         glo.PINS[i]->attachment,
                                         GPIO_read(glo.PINS[i]->pin));
            UART_write(glo.DEVICES.uart0, buffer, strlen(buffer));
        }
        return;
    }
    else
    {
        // Parse and process the -gpio [pin] [flags] [values] command
        uint32_t pin = *token - '0';    //convert the gpio pin to an integer (char -> int)

        if (pin>=GPIO_USED)
            goto GFYS;

        if (pin < GPIO_USED)
        {
            token = GetNxtStr(token, true); // Move to the flags part
            if (token != NULL)
            {
                switch(*token)
                {
                case 'r':   // read flag
                    System_snprintf(buffer, MAXLEN,
                             "\n\rGPIO:\n\rGPIO Pin %i: %s | %i\n\r",
                            glo.PINS[pin]->pin,
                            glo.PINS[pin]->attachment,
                            GPIO_read(glo.PINS[pin]->pin));

                    UART_write(glo.DEVICES.uart0, buffer, strlen(buffer));
                    break;
                case 'w':   // write flag
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
                case 't':   // toggle flag
                        i = GPIO_read(glo.PINS[pin]->pin) ^ 1; //toggle using eor
                        GPIO_write(glo.PINS[pin]->pin, i);
                    break;
                default:
                    goto GFYS;
                }
                return;
            }
            // Pin flag not specified - read
            else
            {
                System_snprintf(buffer, MAXLEN,
                                             "\n\rGPIO:\n\rGPIO Pin %i: %s | %i\n\r",
                                            glo.PINS[pin]->pin,
                                            glo.PINS[pin]->attachment,
                                            GPIO_read(glo.PINS[pin]->pin));
            }
            UART_write(glo.DEVICES.uart0, buffer, strlen(buffer));
            return;
        }
        else
            goto GFYS;
    }

GFYS:
    ErrorOut(&BADGPIO, "Invalid GPIO OP");
    return;
}
