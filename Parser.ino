char RxCmd = ' ';           // command code
char Test[]="-mary had a little lamb-"; // test string for command 'z'

/*-----------------------------------------------------------------------------*/
void Parser(void)
{//the message packet is complete and verified, decode ready message packet
    RxStatus = 0;
    // Serial.println(millis()-RxTime); // debug
    RxTime = millis();  // packet correctly received, reset RX watchdog
    // Serial.println(RxTime); // debug
   
    switch (RxCmd)
    {
       case 'L': // values coming from Low Level Supervisor OUT
            L_LLS();
            
            break;
        
       case 'z': // send back a text string, just for debug
            // Send string 'Test'
            for (int i = 0; i < 25; i++)
            {
                TxBuff[i+RX_HEADER_LEN+1] = Test[i];
            }
            TxData('z', 28);
            Serial1.write("\n");
                 
            break;
            
        default:
            RxError(3); //	error: not a known command
            break;
    }
}

