/*-------------------------------------------------------------------------------
Copyright 2013 Guido Ottaviani
guido@guiott.com
exchange data with the protocol:
http://www.guiott.com/Rino/CommandDescr/Protocol.htm

The comunication part of the  program is structured as a three layer system.
The first layer uses an aynchronous task (event, ISR or whatever) to get bytes from 
the serial port. These bytes are queued in a circular buffer and the serial task takes 
care of updating write pointer to the next free room.
When there are new bytes in buffer the second layer task, scheduled in the main loop, 
gets them and starts the analisys in a Finite State Machine checking the correctenss 
of the message packet.
When the packet is over and verified it passes to the third layer parser that performs
the scheduled actions according to the commnad code.
*/

byte linBuff[MAX_BUFF]; // Linear buffer 
byte hPwrOff;           // Power Off command from HLS to LLS
byte Light[2];          // headlights intensity
byte Obst[11];          // Obstacles distance 7 averaged + 4 RAW
byte lPwrOff;           // Power Off command from LLS to HLS

/*-----------------------------------------------------------------------------*/     
void L_LLS(void)
{/*  values to and from Low Level Supervisor LLS <-> HLS
     right now it's simulated by the same serial port but actually it will come from the LLS serial
     0      Header	 @
     1      Id		 0	ASCII	(not used here, just for compatibility)
     2      Cmd		 G 	ASCII
     3      CmdLen	 Num of bytes (bin) following (checksum included)
     4      BatV[0]      Left and Right battery voltage level
     5      BatV[1]
     6      Temp[0]      Left and Right skulls temperature
     7      Temp[1]
                         obstacle distance @ x degrees 
                         (http://www.guiott.com/QuadSonar/HR/LinoSonar.png)
     8      Obst[0]      LL Lef Left -52°
     9      Obst[1]      LC Left Center -27°
     10     Obst[2]      CL Center Left -12°
     11     Obst[3]      CC Center Center 0°
     12     Obst[4]      CR Center Right 12°
     13     Obst[5]      RC Right Center 27°
     14     Obst[6]      RR Right Right 52°
     15     lPwrOff      Switch Off from LLS to HLS -> 0 = PowerOn, 1 = PowerOff
     
     16     Obst[7]      LL Lef Left -40°
     17     Obst[8]      CL Center Left -15°
     18     Obst[9]      CR Center Right 15°
     19     Obst[10]     RR Right Right 40°
     */
     
     int Indx = RX_HEADER_LEN;  // Head length, number of characters in buffer before valid data
     
     linearize();
     Light[0] = linBuff[0];
     Light[1] = linBuff[1];
     hPwrOff = linBuff[2];
     
     if((hPwrOff == 1) && (ShutdownFlag == 0)) 
     {
       Serial.println("---shutdown ordered by HLS---");
       Shutdown(100);
     }

     analogWrite(Light_L,Light[0]);
     analogWrite(Light_R,Light[1]);
 
     
     TxBuff[++Indx]=BatteryLevel(Batt1_Vin_Val)*20; // Battery level
     TxBuff[++Indx]=BatteryLevel(Batt2_Vin_Val)*20;
     TxBuff[++Indx]=Temp1_Val/10;                   // Temperature
     TxBuff[++Indx]=Temp2_Val/10;
     TxBuff[++Indx]=Obst[0];                        // Obstacles
     TxBuff[++Indx]=Obst[1]; 
     TxBuff[++Indx]=Obst[2]; 
     TxBuff[++Indx]=Obst[3]; 
     TxBuff[++Indx]=Obst[4]; 
     TxBuff[++Indx]=Obst[5]; 
     TxBuff[++Indx]=Obst[6]; 
     TxBuff[++Indx]=lPwrOff;                        // Power Off
     TxBuff[++Indx]=Obst[7];                        // Obstacles RAW data
     TxBuff[++Indx]=Obst[8]; 
     TxBuff[++Indx]=Obst[9]; 
     TxBuff[++Indx]=Obst[10]; 
     
     TxData('L', Indx+1);
}

/*-----------------------------------------------------------------------------*/
void linearize(void)
{// bring the circular queue payload data in a linear buffer for a simpler parsing
  int ptr = RxPtrData;
  for (int i=0; i<RxCmdLen-1; i++)
  {
    if (++ptr >= MAX_BUFF) ptr = 0;
    linBuff[i] = RxBuff[ptr];
  }
}

