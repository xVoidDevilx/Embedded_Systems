#include "GoodFortune.h"

//Extern cmds
extern cmd aboutCMD;
extern cmd helpCMD;
extern cmd printCMD;
extern cmd memrCMD;
extern cmd errorCMD;
extern cmd gpioCMD;
extern cmd timerCMD;
extern cmd callbackCMD;
extern cmd tickerCMD;
extern cmd regCMD;
//Extern errors
extern error BFROVF;
extern error BADCMD;
extern error BADMEM;
extern error BADGPIO;
extern error QUEOVF;
extern error BADMTH;
//Extern gpio
extern gpio PIN0;
extern gpio PIN1;
extern gpio PIN2;
extern gpio PIN3;
extern gpio PIN4;
extern gpio PIN5;
extern gpio PIN6;
extern gpio PIN7;

//Extern callbacks
extern Callback CB0;
extern Callback CB1;
extern Callback CB2;
//Extern Devices and BIOS
extern devices devs;
extern bios biosblock;

/*   Global Struct Declaration   */
global glo = {
              .integrity = 0xB00B1E5,   //mature, right?
              .terminal_out = "",
              .UartMsg.index=0,     //init the index to 0
              .PayloadQueue.PayloadReading=0,
              .PayloadQueue.PayloadWriting=0
};

// config the global struct
void GlobalConfig(global *glo, UART_Handle uart, Timer_Handle timer0, Timer_Handle ticker){
    glo->DEVICES.uart0 = uart;
    glo->DEVICES.timer0 = timer0;
    glo->DEVICES.ticker = ticker;
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
    glo->COMMANDS[8] = &tickerCMD;
    glo->COMMANDS[9] = &regCMD;
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
    glo->ERRORS[5] = &BADMTH;
    //Organize callbacks
    glo->callbacks[0] = &CB0;
    glo->callbacks[1] = &CB1;
    glo->callbacks[2] = &CB2;
    // Init the registers to a known state
    uint32_t i;
    for(i=0; i<NUMREG; i++){
        glo->REGISTERS[i] = 0;
    }
    /* Turn on user LED */
    GPIO_write(glo->PINS[0]->pin, CONFIG_GPIO_LED_ON);
    GPIO_write(glo->PINS[1]->pin, CONFIG_GPIO_LED_ON);
    GPIO_write(glo->PINS[2]->pin, CONFIG_GPIO_LED_ON);
    GPIO_write(glo->PINS[3]->pin, CONFIG_GPIO_LED_ON);
}
