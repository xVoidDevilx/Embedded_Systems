#include "GoodFortune.h"

//Extern errors
extern error BFROVF;
extern error BADCMD;
extern error BADMEM;
extern error BADGPIO;
extern error QUEOVF;
extern error BADMTH;
extern error SCRPTER;
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
    .description = "Load script spaces with cmds, or execute a script space\n\r"
            "\t\t\t-script [id] [payload/flag]\n\r"
            "\t\t\tflags: r=read index x=exe index\n\r"
            "\t\t\tflags: R=read space X=exe space\n\r"
            "\t\t\tflags: c=clear index C=clear space\n\r"
            "\t\t\t64 Script spaces are available...\n\r"
};

cmd ifCMD = {
     .name= "-if",
     .description = "Check a condition and load payloads from condition\n\r"
             "\t\t\t-if [cond] ? [payloadt]:[payloadf]\n\r"
             "\t\t\t[cond] = [<,>,=]\n\r"
};

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
    glo->COMMANDS[12] = &ifCMD;
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
    glo->ERRORS[6] = &SCRPTER;
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
