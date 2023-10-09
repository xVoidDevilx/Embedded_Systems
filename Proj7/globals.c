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
extern cmd remCMD;
extern cmd scriptCMD;
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
    glo->COMMANDS[10] = &remCMD;
    glo->COMMANDS[11] = &scriptCMD;
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
    for(i=0; i<NUMREG; i++)
        glo->REGISTERS[i] = 0;
    // Init the script space:
    for (i=0; i<QUEUELEN; i++)
        memset(glo->scripts[i], 0, sizeof(glo->scripts[i]));

    // Pre init script space for myself
    strncpy(glo->scripts[0], "-rem this is a sample script", strlen("-rem this is a sample script"));
    strncpy(glo->scripts[1], "-gpio 0 t", strlen("-gpio 0 t"));
    strncpy(glo->scripts[2], "-gpio 1 t", strlen("-gpio 1 t"));
    strncpy(glo->scripts[3], "-gpio 2 t", strlen("-gpio 2 t"));
    strncpy(glo->scripts[4], "-gpio 3 t", strlen("-gpio 3 t"));

    strncpy(glo->scripts[6], "-rem this script configs callbacks", strlen("-rem this script configs callbacks"));
    strncpy(glo->scripts[7], "-timer 0", strlen("-timer 0"));
    strncpy(glo->scripts[8], "-callback 1 -1 -regs 31 ++", strlen("-callback 1 -1 -regs 31 ++"));
    strncpy(glo->scripts[9], "-callback 2 -1 -regs 31 --", strlen("-callback 1 -1 -regs 31 --"));
    strncpy(glo->scripts[10], "-callback 0 10 -print Timer popped", strlen("-callback 0 8 -print Timer popped"));

    strncpy(glo->scripts[12], "-rem this configs tickers", strlen("-rem this configs tickers"));
    strncpy(glo->scripts[13], "-ticker 12 10 10 -1 -gpio 0 t", strlen("-ticker 12 10 10 -1 -gpio 0 t"));
    strncpy(glo->scripts[14], "-ticker 13 20 20 -1 -gpio 1 t", strlen("-ticker 13 20 20 -1 -gpio 1 t"));
    strncpy(glo->scripts[15], "-ticker 14 100 100 -1 -gpio 2 t", strlen("-ticker 14 100 100 -1 -gpio 2 t"));
    strncpy(glo->scripts[16], "-ticker 15 50 50 -1 -gpio 3 t", strlen("-ticker 15 50 50 -1 -gpio 3 t"));

    /* Turn on user LED */
    GPIO_write(glo->PINS[0]->pin, CONFIG_GPIO_LED_ON);
    GPIO_write(glo->PINS[1]->pin, CONFIG_GPIO_LED_ON);
    GPIO_write(glo->PINS[2]->pin, CONFIG_GPIO_LED_ON);
    GPIO_write(glo->PINS[3]->pin, CONFIG_GPIO_LED_ON);
}
