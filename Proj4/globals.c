#include "GoodFortune.h"

/*   Global Struct Declaration   */
global glo = {
              .integrity = 0xB00B1E5,   //mature, right?
              .terminal_out = "",
              .UartMsg.index=0,     //init the index to 0
              .PayloadQueue.PayloadReading=0,
              .PayloadQueue.PayloadWriting=0
};

// config the global struct
void GlobalConfig(global *glo, UART_Handle uart, Timer_Handle timer){
    glo->DEVICES.uart0 = uart;
    glo->DEVICES.timer0 = timer;
    glo->BIOS = &biosblock;
    //Organize Commands
    glo->COMMANDS[0] = &aboutCMD;
    glo->COMMANDS[1] = &helpCMD;
    glo->COMMANDS[2] = &printCMD;
    glo->COMMANDS[3] = &memrCMD;
    glo->COMMANDS[4] = &gpioCMD;
    glo->COMMANDS[5] = &errorCMD;
    glo->COMMANDS[6] = &timerCMD;
    glo->COMMANDS[7] = &callbackCMD;
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
    glo->ERRORS[3] = &BADGPIO;
    glo->ERRORS[4] = &QUEOVF;
    //Organize callbacks
    glo->callbacks[0] = &CB0;
    glo->callbacks[1] = &CB1;
    glo->callbacks[2] = &CB2;
    /* Turn on user LED */
    GPIO_write(glo->PINS[0]->pin, CONFIG_GPIO_LED_ON);
    GPIO_write(glo->PINS[1]->pin, CONFIG_GPIO_LED_ON);
    GPIO_write(glo->PINS[2]->pin, CONFIG_GPIO_LED_ON);
    GPIO_write(glo->PINS[3]->pin, CONFIG_GPIO_LED_ON);
}
