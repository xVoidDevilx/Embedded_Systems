/*
 * callbacks.c
 *
 *  Created on: Sep 21, 2023
 *      Author: silas
 */

#include "GoodFortune.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//timer 0 callback
Callback CB0 = {
                .HWI_Flag = false,
                .count=0,
                .payload=0
};
//LSW callback
Callback CB1 = {
                .HWI_Flag = false,
                .count=0,
                .payload=0
};
//RSW callback
Callback CB2 = {
                .HWI_Flag = false,
                .count=0,
                .payload=0
};
extern error ADCERR;

void SWITimer0(UArg arg0, UArg arg1){
    //run set times
    int32_t i = 0;
    uint32_t gatekey;
    bool shouldIexit = true;
    gatekey = GateSwi_enter(glo.BIOS->CallbackGate);
    if(glo.callbacks[i]->count!=0)
    {
        if(glo.callbacks[i]->count>0)
            glo.callbacks[i]->count--;  //decrement the counter
        if(!glo.callbacks[i]->HWI_Flag)
        {
            shouldIexit = false;
            GateSwi_leave(glo.BIOS->CallbackGate, gatekey);
            AddPayload(glo.callbacks[i]->payload);
        }
    }
    if (shouldIexit)
        GateSwi_leave(glo.BIOS->CallbackGate, gatekey);
}

//Enter this callback when the timer pops - HWI
void Timer0Callback(Timer_Handle TimerHandle, int_fast16_t status)
{
    int32_t i = 0;
    glo.callbacks[i]->HWI_Flag = false;

    if(glo.callbacks[i]->count != 0){
        if (!strcmp(glo.callbacks[i]->payload, "-audio")){
            AudioCMD(glo.callbacks[i]->payload);
            glo.callbacks[i]->HWI_Flag = true;
            return;
        }
        if (!strncmp(glo.callbacks[i]->payload, "-sin", strlen("-sin"))){
            glo.callbacks[i]->HWI_Flag = true;
            sineCMD(glo.callbacks[i]->payload);
            return;
        }
    }
    Swi_post(glo.BIOS->Timer0SWI);
}

//Enter this callback for the ticker
void Timer1Callback(Timer_Handle TimerHandle, int_fast16_t status)
{
    Swi_post(glo.BIOS->TickerSWI);
}
// This is the SWI for the ticker
void SWITimer1(UArg arg0, UArg arg1){
   //run set times
   int32_t i = 0;
   uint32_t gatekey;
   gatekey = GateSwi_enter(glo.BIOS->CallbackGate);
   for (i=0; i<NUMTICKERS; i++)
   {
       // Tick until payload activation
       if(glo.tickers[i].delay > 0)
       {
           glo.tickers[i].delay--;
           //activation
           if(glo.tickers[i].delay<=0)
           {
               // Run if the ticker is scheduled to run
               if (glo.tickers[i].count !=0)
               {
                   glo.tickers[i].delay = glo.tickers[i].period;    //reload the ticker next activation
                   if(glo.tickers[i].count > 0)
                       glo.tickers[i].count--;  // decrease the reloads
               }
               AddPayload(glo.tickers[i].payload);      // deliver the payload to be serviced when delay is 0
           }
       }
   }
   GateSwi_leave(glo.BIOS->CallbackGate, gatekey);
}

//SWI LSW - Send queue - post sem, etc
void SWILSW(UArg arg0, UArg arg1)
{
    int32_t i = 1;
    uint32_t gatekey;
    gatekey = GateSwi_enter(glo.BIOS->CallbackGate);
    if(glo.callbacks[i]->count!=0){
        if(glo.callbacks[i]->count>0)
            glo.callbacks[i]->count--;  //decrement the counter
        AddPayload(glo.callbacks[i]->payload);
    }
   GateSwi_leave(glo.BIOS->CallbackGate, gatekey);
}

//Enter this callback when LSW is pressed - HWI
void LSWCallback(uint_least8_t index){
    Swi_post(glo.BIOS->LSWSWI);
    return;
}

//SWI RSW
void SWIRSW(UArg arg0, UArg arg1)
{
    int32_t i = 2;
    uint32_t gatekey;
    gatekey = GateSwi_enter(glo.BIOS->CallbackGate);
    if(glo.callbacks[i]->count!=0){
        if(glo.callbacks[i]->count>0)
            glo.callbacks[i]->count--;  //decrement the counter
        AddPayload(glo.callbacks[i]->payload);
    }
   GateSwi_leave(glo.BIOS->CallbackGate, gatekey);
}

//Enter this callback when Rsw is pressed
void RSWCallback(uint_least8_t index){
    Swi_post(glo.BIOS->RSWSWI);
    return;
}

void ADC_BUFFER_CALLBACK_FXN(ADCBuf_Handle handle,
                             ADCBuf_Conversion *conversion,
                             void *completedADCBuffer,
                             uint32_t completedChannel,
                             int_fast16_t status)
{
    // Generate an error for mem corruption
    if(completedADCBuffer != glo.TxRx.RX_Ping && completedADCBuffer != glo.TxRx.RX_Pong)
    {
        Swi_post(glo.BIOS->ADCSWI);
        return;
    }
    glo.DEVICES.adcbufctrl.RX_completed = completedADCBuffer;
    glo.DEVICES.adcbufctrl.callback_count++;
    Semaphore_post(glo.BIOS->ADCBufSem);
}

void SWIADC(UArg argo, UArg arg1)
{
    ADCERR.count++;
    AddPayload("-print stream reading failed\n\r");
}
