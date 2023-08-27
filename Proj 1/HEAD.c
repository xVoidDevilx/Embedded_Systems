/*
 * HEAD.c
 *
 *  Created on: Aug 27, 2023
 *      Author: silas
 *      Purpose: Define commands that will be used on the MSP432 for the Embedded Systems course @ TTU
 */

#include "HEAD.h"
#include <stdio.h>

// Define instances of the Command struct
CMD aboutCMD = {
    .name = "-about",
    .description = "Get information about the program.",
    .output = "\r\n\tEngineer: Silas Rodriguez\r\n\t"
            "Version: 0.1\r\n\t"
            "Date|Time: date | time\r\n"
};

CMD helpCMD = {
    .name = "-help",
    .description = "Display available commands & their descriptions.",
    .output = ""
};
