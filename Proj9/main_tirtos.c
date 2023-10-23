/*
 *  ======== main_tirtos.c ========
 */
#include <stdint.h>

/* POSIX Header files */
#include <pthread.h>

/* RTOS header files */
#include <ti/sysbios/BIOS.h>

#include <ti/drivers/Board.h>

#include "GoodFortune.h"

/*
 * The following (weak) function definition is needed in applications
 * that do *not* use the NDK TCP/IP stack:
 */
void __attribute__((weak)) NDK_hookInit(int32_t id) {}

/*
 *  ======== main ========
 */
int main(void)
{
    Board_init();
    InitializeDrivers();
    BIOS_start();
    return (0);
}
