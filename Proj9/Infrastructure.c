#include "GoodFortune.h"

void InitializeDrivers (void){
    UART_Handle uart0, uart7;
    UART_Params uartParams;

    Timer_Handle timer0, ticker;
    Timer_Params TimerParams;

//    int_fast16_t status;

    Board_init();
    GPIO_init();
    UART_init();
    Timer_init();

    /* Configure the GPIO pins */
    GPIO_setConfig(CONFIG_GPIO_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_1, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_2, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_3, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_4, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_5, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(LSW, GPIO_CFG_IN_PU);
    GPIO_setConfig(RSW, GPIO_CFG_IN_PU);
    GPIO_setCallback(LSW, &LSWCallback);
    GPIO_setCallback(RSW, &RSWCallback);
    //Enable interrupts on the switches
    GPIO_enableInt(LSW);
    GPIO_enableInt(RSW);

    /* Create a UART with data processing off. */
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.baudRate = 115200;

    uart0 = UART_open(CONFIG_UART_0, &uartParams);

    if (uart0 == NULL) {
        /* UART_open() failed */
        while (1);
    }

    // Configure uart 7
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_TEXT;
    uartParams.readReturnMode = UART_RETURN_NEWLINE;
    uartParams.baudRate = 115200;
    uartParams.readEcho = UART_ECHO_OFF;

    uart7 = UART_open(CONFIG_UART_1, &uartParams);

    if (uart7 == NULL) {
        /* UART_open() failed */
        while (1);
    }

    //Configure Timer 0
    Timer_Params_init(&TimerParams);
    TimerParams.period = INIT_PERIOD;
    TimerParams.periodUnits = Timer_PERIOD_US;
    TimerParams.timerCallback = Timer0Callback;
    TimerParams.timerMode = Timer_CONTINUOUS_CALLBACK;
    glo.DEVICES.timer0_period = INIT_PERIOD;
    timer0 = Timer_open(CONFIG_TIMER_0, &TimerParams);

    if(timer0 == NULL) while(1); /*Timer failed to open*/
    if(Timer_start(timer0) == Timer_STATUS_ERROR) while(1); //Timer start Failed

    Timer_Params_init(&TimerParams);
    TimerParams.period = 10000;     //default 10 ms
    TimerParams.periodUnits = Timer_PERIOD_US;
    TimerParams.timerCallback = Timer1Callback;
    TimerParams.timerMode = Timer_CONTINUOUS_CALLBACK;
    ticker = Timer_open(CONFIG_TIMER_1, &TimerParams);

    if(ticker == NULL) while(1); /*ticker failed to open*/
    if(Timer_start(ticker) == Timer_STATUS_ERROR) while(1); //ticker start Failed

    //BIOS config
    InitBios();
    //Global Configurator
    GlobalConfig(&glo, uart0, uart7, timer0, ticker);
}
