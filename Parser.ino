/*-------------------------------------------------------------------------------
Copyright 2013 Guido Ottaviani
guido@guiott.com
part of "ProtocolTester.ino"
acts as a tester exchanging data with the protocol:
http://www.guiott.com/Rino/CommandDescr/Protocol.htm
*/

char RxCmd = ' ';           // command code
char Test[]="-mary had a little lamb-"; // test string for command 'z'

/*-----------------------------------------------------------------------------*/
void Parser(void)
{//the message packet is complete and verified, decode ready message packet
    RxStatus = 0;
    RxTime = millis();  // packet correctly received, reset RX watchdog

    switch (RxCmd)
    {
       case 'L': // values coming from Low Level Supervisor OUT
            L_LLS();
        
       case 'z': // send back a text string, just for debug
            // Send string 'Test'
            for (int i = 0; i < 25; i++)
            {
                TxBuff[i+RX_HEADER_LEN+1] = Test[i];
            }
            TxData('z', 28);
            Serial.write("\n");
                 
            break;
            
        default:
            RxError(3); //	error: not a known command
            break;
    }
}

