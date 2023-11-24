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
extern error NETERR;
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
            "\t\t\tgpio 4: PK5 | 0 = Speaker En\n\r"
            "\t\t\tgpio 5: PD4 | 1 = Mic En\n\r"
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

cmd uartCMD = {
     .name= "-uart",
     .description = "Deliver a payload out of uart\n\r"
             "\t\t\t-uart [payload]\n\r"
             "\t\t\tPC5/4 - uart7TX\n\r"
             "\t\t\tPC4/3 - uart7RX\n\r"
};

cmd sinCMD = {
     .name= "-sin",
     .description = "sine wave generator\n\r"
             "\t\t\t-sin (freq)\n\r"
             "\t\t\t-sine (freq)\n\r"
};
cmd clrCMD = {
     .name= "-clear",
     .description = "clear the console\n\r"
};

cmd strmCMD = {
     .name= "-stream",
     .description = "Setup a stream\n\r"
};

cmd audioCMD = {
     .name= "-audio",
     .description = "Play audio out to the speaker from a source\n\r"
};

cmd voiceCMD = {
     .name= "-voice",
     .description = "VOIP\n\r"
};

cmd netCMD = {
     .name= "-netudp",
     .description = "Transmit a UDP packet\n\r"
             "\t\t\t-netudp [ip]:[port] [command]"
};
/*   Global Struct Declaration   */
global glo = {
              .integrity = 0xB00B1E5,   //mature, right?
              .terminal_out = "",
              .UartMsg.index=0,     //init the index to 0
              .PayloadQueue.PayloadReading=0,
              .PayloadQueue.PayloadWriting=0,
              .LUTctrl.sinLUT = SINTABLE,
              .LUTctrl.lutposition = 0.,
              .LUTctrl.deltalut = 0.
};

// config the global struct
void GlobalConfig(global *glo, UART_Handle uart0, UART_Handle uart7,Timer_Handle timer0, Timer_Handle ticker){
    glo->DEVICES.uart0 = uart0;
    glo->DEVICES.uart7 = uart7;

    glo->DEVICES.timer0 = timer0;
    glo->DEVICES.ticker = ticker;
    glo->BIOS = &biosblock;
    //Organize Commands
    unsigned int i = 0;
    glo->COMMANDS[i++] = &aboutCMD;
    glo->COMMANDS[i++] = &helpCMD;
    glo->COMMANDS[i++] = &printCMD;
    glo->COMMANDS[i++] = &memrCMD;
    glo->COMMANDS[i++] = &gpioCMD;
    glo->COMMANDS[i++] = &errorCMD;
    glo->COMMANDS[i++] = &timerCMD;
    glo->COMMANDS[i++] = &callbackCMD;
    glo->COMMANDS[i++] = &tickerCMD;
    glo->COMMANDS[i++] = &regCMD;
    glo->COMMANDS[i++] = &remCMD;
    glo->COMMANDS[i++] = &scriptCMD;
    glo->COMMANDS[i++] = &ifCMD;
    glo->COMMANDS[i++] = &uartCMD;
    glo->COMMANDS[i++] = &sinCMD;
    glo->COMMANDS[i++] = &clrCMD;
    glo->COMMANDS[i++] = &strmCMD;
    glo->COMMANDS[i++] = &audioCMD;
    glo->COMMANDS[i++] = &voiceCMD;
    glo->COMMANDS[i++] = &netCMD;
    i = 0;
    //Organize GPIO
    glo->PINS[i++] = &PIN0;
    glo->PINS[i++] = &PIN1;
    glo->PINS[i++] = &PIN2;
    glo->PINS[i++] = &PIN3;
    glo->PINS[i++] = &PIN4;
    glo->PINS[i++] = &PIN5;
    glo->PINS[i++] = &PIN6;
    glo->PINS[i++] = &PIN7;

    i = 0;
    //Organize Errors
    glo->ERRORS[i++] = &BFROVF;
    glo->ERRORS[i++] = &BADCMD;
    glo->ERRORS[i++] = &BADMEM;
    glo->ERRORS[i++] = &BADGPIO;
    glo->ERRORS[i++] = &QUEOVF;
    glo->ERRORS[i++] = &BADMTH;
    glo->ERRORS[i++] = &SCRPTER;
    glo->ERRORS[i++] = &ADCERR;
    glo->ERRORS[i++] = &VoiceERR;
    glo->ERRORS[i++] = &StreamERR;
    glo->ERRORS[i++] = &NETERR;
    //Organize callbacks
    glo->callbacks[0] = &CB0;
    glo->callbacks[1] = &CB1;
    glo->callbacks[2] = &CB2;

    // Init the registers to a known state
    for(i=0; i<NUMREG; i++)
        glo->REGISTERS[i] = 0;
    // Init the script space:
    for (i=0; i<QUEUELEN; i++)
        memset(glo->scripts[i], 0, sizeof(glo->scripts[i]));

    // Pre init script space for myself
    i = 0;
    strcpy(glo->scripts[i], "-rem this is a sample script");
    strcpy(glo->scripts[++i], "-gpio 0 t");
    strcpy(glo->scripts[++i], "-gpio 1 t");
    strcpy(glo->scripts[++i], "-gpio 2 t");
    strcpy(glo->scripts[++i], "-gpio 3 t");
    i++;
    strcpy(glo->scripts[++i], "-rem this script configs callbacks");
    strcpy(glo->scripts[++i], "-timer 0");
    strcpy(glo->scripts[++i], "-callback 1 -1 -regs 31 ++");
    strcpy(glo->scripts[++i], "-callback 2 -1 -regs 31 --");
    strcpy(glo->scripts[++i], "-callback 0 10 -print Timer popped");
    i++;
    strcpy(glo->scripts[++i], "-rem this configs tickers for binary counter");
    strcpy(glo->scripts[++i], "-ticker 12 20 20 -1 -gpio 0 t");
    strcpy(glo->scripts[++i], "-ticker 13 40 40 -1 -gpio 1 t");
    strcpy(glo->scripts[++i], "-ticker 14 80 80 -1 -gpio 2 t");
    strcpy(glo->scripts[++i], "-ticker 15 160 160 -1 -gpio 3 t");
    i++;
    strcpy(glo->scripts[++i], "-rem this tests conditionals");
    strcpy(glo->scripts[++i], "-if #2 > #3 ? -scripts 0 X:");
    strcpy(glo->scripts[++i], "-if 31 > #0 ? -scripts 0 X:");
    strcpy(glo->scripts[++i], "-if 31 < #0 ? -scripts 1 x:");
    strcpy(glo->scripts[++i], "-if 31 = #0 ? -scripts 12 X:");
    i++;
    strcpy(glo->scripts[++i], "-rem this tests local audio");
    strcpy(glo->scripts[++i], "-timer 0");                          // disable timer
    strcpy(glo->scripts[++i], "-callback 0 c");                     // clear the callback
    strcpy(glo->scripts[++i], "-timer 125");                        // sets the timer to 125 us
    strcpy(glo->scripts[++i], "-sine 250");                         // sets the sine wave @ 500 Hz
    strcpy(glo->scripts[++i], "-callback 0 -1 -sine");              // calls the speaker
    strcpy(glo->scripts[++i], "-gpio 4 t");                         // enable the amp
    strcpy(glo->scripts[++i], "-ticker 0 500 0 0 -gpio 4 t");       // disable the amp
    strcpy(glo->scripts[++i], "-ticker 2 501 0 0 -callback 0 c");   // disable callback
    strcpy(glo->scripts[++i], "-ticker 1 501 0 0 -tickers c");      // clear tickers
    i++;
    /* Turn on user LED */
    GPIO_write(glo->PINS[0]->pin, CONFIG_GPIO_LED_OFF);
    GPIO_write(glo->PINS[1]->pin, CONFIG_GPIO_LED_OFF);
    GPIO_write(glo->PINS[2]->pin, CONFIG_GPIO_LED_OFF);
    GPIO_write(glo->PINS[3]->pin, CONFIG_GPIO_LED_OFF);

    glo->DEVICES.adcbufctrl.conversion.adcChannel = ADCBUF_CHANNEL_0;
    glo->DEVICES.adcbufctrl.conversion.arg = NULL;
    glo->DEVICES.adcbufctrl.conversion.sampleBuffer = glo->TxRx.RX_Ping;
    glo->DEVICES.adcbufctrl.conversion.sampleBufferTwo = glo->TxRx.RX_Pong;
    glo->DEVICES.adcbufctrl.conversion.samplesRequestedCount = DATABLOCKSIZE;
    glo->DEVICES.adcbufctrl.RX_completed = NULL;
    glo->DEVICES.adcbufctrl.TX_completed = NULL;
    glo->DEVICES.adcbufctrl.TX_index = -1;
    glo->DEVICES.adcbufctrl.ping_count = 0;
    glo->DEVICES.adcbufctrl.pong_count = 0;
    glo->DEVICES.adcbufctrl.sample_count = 0;
    glo->DEVICES.adcbufctrl.callback_count = 0;
    glo->DEVICES.adcbufctrl.callback0_count = 0;
    glo->DEVICES.adcbufctrl.missed_255ADCs = 0;
    glo->DEVICES.adcbufctrl.doubled_255ADCs = 0;
    glo->DEVICES.adcbufctrl.delay = 0;
}
