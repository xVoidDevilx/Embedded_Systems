#include "GoodFortune.h"

//Semaphores & Swi
extern Semaphore_Handle PayLoadSem;
extern Semaphore_Handle ADCBufSem;
extern Semaphore_Handle UDPOutSem;
extern Swi_Handle swi0;
extern Swi_Handle swi1;
extern Swi_Handle swi2;
extern Swi_Handle swi3;
extern Swi_Handle swi4;
extern GateSwi_Handle gateSwi0;
extern GateSwi_Handle gateSwi1;
extern GateSwi_Handle gateSwi2;
extern GateSwi_Handle gateSwi3;
extern GateSwi_Handle gateSwi4;

bios biosblock;
void InitBios(void){
    biosblock.Timer0SWI = swi0;
    biosblock.LSWSWI = swi1;
    biosblock.RSWSWI = swi2;
    biosblock.TickerSWI = swi3;
    biosblock.ADCSWI = swi4;
    biosblock.PayloadSem = PayLoadSem;
    biosblock.ADCBufSem = ADCBufSem;
    biosblock.UDPOutSem = UDPOutSem;
    biosblock.CallbackGate = gateSwi0;      // gate for callbacks
    biosblock.PayloadWriteGate = gateSwi1;  // gate for payloads
    biosblock.MSGWriteGate = gateSwi2;      // gate for addbyte
    biosblock.UDPOutWriteGate = gateSwi3;
    biosblock.RegsGate = gateSwi4;
}
