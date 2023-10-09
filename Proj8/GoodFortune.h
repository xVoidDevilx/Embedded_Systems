#ifndef GOODFORTUNE_H_
#define GOODFORTUNE_H_

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/TIMER.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Swi.h>
#include <ti/sysbios/hal/Hwi.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/gates/GateSwi.h>

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ti_drivers_config.h"
#include <xdc/std.h>

// MACROS
#define MAXLEN 256       // 256 Char Payloads max
#define QUEUELEN 64      // How many commands can float in the queue
#define NUMSCRIPTS 64    // script space length
#define BACKSPACE 127    // Putty Backspace ASCII
#define NUMCALLBACKS 3
#define NUMTICKERS 16    // support for 16 tickers
#define NUMREG 32        // Number of supported registers

//LOOP MACROS
#define SUPPORTEDCMDS 13  // Total Commands Being supported by the system right now
#define GPIO_USED 8       // Total GPIO expected for GPIO command
#define ERRTYPES 7        // Supported Error Types in the System

//TIMER
#define INIT_PERIOD 2500000 //2.5s

//Struct Definitions
typedef struct _cmd {
  const char name[MAXLEN>>3];       // 32 Char name max
  const char description[MAXLEN<<1];   // 512 char description max
} cmd;

typedef struct _err {
    const uint32_t id;              // ID of the error
    const char name[MAXLEN>>2];     // Pretty name
    const char msg[MAXLEN];         // Error message
    uint32_t count;                 // Times this error has occurred
} error;

typedef struct _gpio {
    const uint32_t pin;              // Pin #
    const char attachment[MAXLEN];   // What the pin is attached to
} gpio;

typedef struct _ticker {
    uint32_t delay;
    uint32_t period;
    int32_t count;
    char payload[MAXLEN];
}TICKER;

//typedef struct _pwm {
//    int32_t index;
//    int32_t delay;
//    int32_t period;
//    int32_t duty;
//    bool running;
//}PWM;

typedef struct _callback {
    int32_t count;
    bool HWI_Flag;
    char payload[MAXLEN];
}Callback;

typedef struct _message {
    int32_t index;              //current position in the buffer
    //int32_t MsgCount;         // This messages current index
    char msgBuf[MAXLEN];        // the buffer for this message
}Message;

typedef struct _messageQueue {
    int32_t MsgReading;     //where they come out
    int32_t MsgWriting;     //where im writing
    Message Messages[QUEUELEN]; //Messages
}MQ;

typedef struct _payloadQueue {
    int32_t PayloadReading;             //Which Payload is being read
    int32_t PayloadWriting;             //which payload is being written
    //int32_t BinaryCount[QUEUELEN];    //?
    char payloads[QUEUELEN][MAXLEN];    //Payloads to be processed
}PQ;

typedef struct _bios {
    Semaphore_Handle UART0ReadSem;
    Semaphore_Handle PayloadSem;
    //Semaphore_Handle ADCbufSem;
    //Semaphore_Handle UDPOutSem;
    //Semaphore_Handle UDPLaunchSem;
    //Semaphore_Handle UDPInSem;
    //Swi_Handle ComparatorSWI;
    Swi_Handle Timer0SWI;
    Swi_Handle TickerSWI;
    Swi_Handle LSWSWI;
    Swi_Handle RSWSWI;
    //Swi_Handle ADCSWI;
    GateSwi_Handle CallbackGate;
    //GateSwi_Handle TickerGate;
    GateSwi_Handle PayloadWriteGate;
    GateSwi_Handle MSGReadGate;
    GateSwi_Handle MSGWriteGate;
    GateSwi_Handle RegGate;
    //GateSwi_Handle IfQWriteGate;
    //GateSwi_Handle UDPOutWriteGate;
} bios;

typedef struct _dev
{
    //ADC_Handle adc[ADCCOUNT];
    UART_Handle uart0;
    //UART_Handle uart7;
    Timer_Handle timer0;
    uint32_t timer0_period;
    Timer_Handle ticker;
    //ADCBuf_Handle adcBuf;
    //ADCBufControl adcbufctrl;
    //SPI_Handle spi3;
    //Comparator_Handle comp0;
    //PWM_Handle PWM[PWMCOUNT];
} devices;


typedef struct _global
{
    const uint32_t integrity;       // Checking build

    char terminal_out[MAXLEN<<2];       // 1024 Char output max
    char scripts[NUMSCRIPTS][MAXLEN];  // Give a script space to quickly add payloads to the queue
    devices DEVICES;
    bios* BIOS;                     // Structure of BIOS handles

    Callback *callbacks[NUMCALLBACKS];
    PQ PayloadQueue;
    Message UartMsg;
    TICKER tickers[NUMTICKERS];      // array of tickers and commands
    int32_t REGISTERS[NUMREG];       // array of registers
    cmd *COMMANDS[SUPPORTEDCMDS];    // Supported commands
    gpio *PINS[GPIO_USED];           // A list of gpio pins
    error *ERRORS[ERRTYPES];         // Error types in the system

} global;

//Function Prototypes
void InitializeDrivers (void);                                          // Initialize TI stuff & call GlobalConfig
void GlobalConfig(global *glo, UART_Handle uart, Timer_Handle timer0, Timer_Handle ticker);  // Config the global struct
char UartAddByte(char input);                                           // Take inputs from UART, place into a buffer
void ParseBuf(char payload[], uint32_t lenPayload);                     // Parser to map strings to commands
void HelpCMD(char* INPUT);                                              // Print available commands
void PrintAbout(char* OUTPUT, size_t buffer_size);                      // Print system information
void PrintCMD (char buffer[], char result[], size_t len);               // Echo to console some string
void MemrCMD(char *addrHex); // Access a mem location and view hex data
void GPIOCMD(char* INPUT);                                              // Take an input, process its meaning, and handle GPIO
void PrintErrs(char* INPUT);                                            // Print errors involved with the system
void ErrorOut(error ERR, char* details);                                // Error out during a function call
void InitBios(void);                                                    // Init Bios devices (semaphores, gates, etc)
void CallbackCMD(char *input, char *output, size_t outLen);             // HWI, SWI, callbacks structs
void TimerCMD(char *input);                                             // Reconfigure timer properties
char *GetNxtStr(char * input, bool AllWhites);                          // Walk to next string location between white spaces
int32_t AddPayload(char *payload);                                      // Add payload to a payload queue
void TickerCMD(char *input);                                            // Add a payload to the tickers
void RegCMD(char *input);                                               // Math operations with registers
int32_t ExtractLitSrc (char * token);                                   // Helper function
void ScriptCMD(char *input);                                            // Script payload funciton
void condCMD(char * input);                                             // Function to handle conditional payloads
//Callback Prototypes
void Timer0Callback(Timer_Handle TimerHandle, int_fast16_t status);     // Callback at timer interrupt
void Timer1Callback(Timer_Handle TimerHandle, int_fast16_t status);     // Callback at timer interrupt
void LSWCallback(uint_least8_t index);                                  // Callback from Left Switch Interrupt
void RSWCallback(uint_least8_t index);                                  // Callback from Right Switch Interrupt

//Extern Global Handler
extern global glo;

#endif /* GOODFORTUNE_H_ */
