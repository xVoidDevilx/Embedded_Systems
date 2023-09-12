/*
 * chf.h
 *
 *  Created on: Sep 11, 2023
 *      Author: silas
 */

#ifndef CHF_H_
#define CHF_H_

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>
#include "ti_drivers_config.h"

//MACROS
#define MAXLEN 256 // 256 Char inputs max
#define BACKSPACE 127
#define SUPPORTED 4
//Struct Definitions
typedef struct _cmd {
  const char name[MAXLEN>>2];       // 64 Char name max
  const char description[MAXLEN];   // 256 char description max
} cmd;

typedef struct _err {
    const uint32_t id;              // ID of the error
    const char msg[MAXLEN];         // Error message
    uint32_t count;                 // Times this error has occured
} error;

typedef struct _global {
    const uint32_t integrity;       //Checking build
    char terminal_in[MAXLEN];       // Input buffer
    char terminal_out[MAXLEN<<1];   // 512 Char output max
    UART_Handle uart;               //handler
    UART_Params uartParams;         //params
    cmd COMMANDS[SUPPORTED];        //supported commands
} global;

//Function Prototypes
void GlobalConfig(global *glo); // config the global struct
char ProcInp(char input, size_t bufflen, global *glo);  //take inputs from UART, place into a buffer
size_t ReturnCommandIndex(char *str, global *glo, size_t num_cmds);
void HelpCMD(char* INPUT, global *glo);
void PrintAbout(char* OUTPUT, size_t buffer_size);
void PrintCMD (char buffer[], char result[], size_t len);
void MemrCMD(char *addrHex, char OUTPUT[], size_t bufflen, error* err);
//Extern cmds
extern cmd aboutCMD;
extern cmd helpCMD;
extern cmd printCMD;
extern cmd memrCMD;
//Extern errors
extern error BFROVF;
extern error BADCMD;
extern error BADMEM;
//Global Handler Defs
extern global glo;
#endif /* CHF_H_ */
