/*
 * HEAD.h
 *
 *  Created on: Aug 27, 2023
 *  Author: Silas Rodriguez
 */

#ifndef HEAD_H_
#define HEAD_H_

/* Standard Headers */
#include <stdio.h>
#include <stddef.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>
/* Driver configuration */
#include "ti_drivers_config.h"

#define MAXLEN 256  // 256 B of data per string.
// -------------------------------------------------------------------\\

//Define command structures
typedef struct CMD {
    const char name[MAXLEN>>2];         // command name | 64 B of space / cmd
    const char description[MAXLEN];  // command help menu text | 256 B of space / cmd
} CMD;
// -------------------------------------------------------------------\\

//Define error struct - future implementations
typedef struct ERR {
    const int32_t code;             // error code:      4 B / Error type
    const char msg[MAXLEN];         // error message: 256 B / Error type
} ERROR;
// -------------------------------------------------------------------\\

//Define global variables | working data - Increases memory efficiency
typedef struct GLOBAL {
    const int32_t Integrity;        // Allocate 4 B for integrity
    char TERMINAL_IN[MAXLEN];       // Allocate 256 B
    char TERMINAL_OUT[MAXLEN<<2];   // Allocate 1024 B
    UART_Handle uart;               // uart handler global
    UART_Params uartParams;         // configure uart
    uint32_t ERRORS;                // The bigger this gets, the more humbled I become
} GLOBAL;
// -------------------------------------------------------------------\\

//Functional Prototypes
size_t ReturnCommandIndex(char *str, const CMD COMMANDS[], size_t num_cmds);
void HelpCMD(char* INPUT, char* OUTPUT, size_t buffer_size, const CMD COMMANDS[], size_t num_cmds);
void PrintAbout(char* OUTPUT, size_t buffer_size);
void PrintERR(char* OUTPUT, size_t buffer_size, ERROR err);
void GlobalConfig(GLOBAL *obj);
void PrintCMD (char buffer[], char result[], size_t len);
void MemrCMD(char *addrHex, char OUTPUT[], size_t bufflen,uint32_t ERRCounter);
// -------------------------------------------------------------------\\

//External variables
extern CMD aboutCMD;
extern CMD helpCMD;
extern CMD printCMD;
extern CMD memrCMD;
extern GLOBAL glo;
extern ERROR BFR_OVF;
#endif /* HEAD_H_ */
