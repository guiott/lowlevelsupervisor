/*-------------------------------------------------------------------------------
Copyright 2013 Guido Ottaviani
guido@guiott.com
part of "ProtocolTester.ino"
acts as a tester exchanging data with the protocol:
http://www.guiott.com/Rino/CommandDescr/Protocol.htm
*/

/*-----------------------------------------------------------------------------*/
void TxData(char TxCmd, int TxCmdLen)
{// Send the requested parameters, with the correct handshake
    byte TxChkSum=0;

    TxBuff[0] = HEADER;
    TxBuff[1] = ID;           // often not used, useful if many device on bus
    TxBuff[2] = TxCmd;        // command
    TxBuff[3] = TxCmdLen - RX_HEADER_LEN;     // lenght of the payload plus CHKSUM
    for (int i=0; i<=TxCmdLen; i++)
    {// compute the checksum while sending
      TxChkSum += (byte)(TxBuff[i]);
      Serial1.write(TxBuff[i]);
    }
   
    Serial1.write(TxChkSum);
}

