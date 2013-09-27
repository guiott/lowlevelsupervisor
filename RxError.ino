/*-------------------------------------------------------------------------------
Copyright 2013 Guido Ottaviani
guido@guiott.com
part of "ProtocolTester.ino"
acts as a tester exchanging data with the protocol:
http://www.guiott.com/Rino/CommandDescr/Protocol.htm
*/

String RxErrorText = "";
char  RxErrorExpected = ' ';
char RxErrorFound = ' ';

/*-----------------------------------------------------------------------------*/
void RxError(int Err)
{  
  RxPtrOut=0;				// flush circular queue
  RxPtrIn=0;
  RxPtrEnd=0;
  RxStatus=0; 
  ChkSum=0;
  
  switch (Err)
  {
    case 1: // Timeout
      RxErrorText= "Timeout";
      RxErrorExpected = ' ';
      RxErrorFound = ' ';
    break;
    
    case 2: // no header found, out of command sequence
      RxErrorText= "No Header";
      RxErrorExpected = '@';
      RxErrorFound =(char)(RxBuff[0]);
    break;
    
    case 3: // unknown command
      RxErrorText= "Wrong CMD";
      RxErrorExpected = 'Cmd';
      RxErrorFound = (char)(RxBuff[2]);
    break;
      
    case 4:
      RxErrorText= "Wrong LEN";
      RxErrorExpected = 'Len';
      RxErrorFound = (char)(RxBuff[3]-1);
    break;

    case 5:
      RxErrorText= "ChkSum err";
      RxErrorExpected = (char)(ChkSum);
      RxErrorFound = (char)(+RxBuff[RxPtrOut]);
    break;

    case 6:
      RxErrorText= "Queue overflow";
      RxErrorExpected = ' ';
      RxErrorFound = ' ';
    break;
    
   case 7:
      RxErrorText= "Unknown FSM status";
      RxErrorExpected = ' ';
      RxErrorFound = ' ';
    break;
    
    default:
      RxErrorText= "--RX OK--";
      RxErrorExpected = ' ';
      RxErrorFound =  ' ';
    break;
  }
  
#ifdef DEBUG_MODE
    Serial.print("Err: ");
    Serial.print(Err); 
    Serial.print("  Chk: ");
    Serial.print(ChkSum,DEC);
    Serial.println("  "+RxErrorText+"  expected: "+RxErrorExpected+"  found: "+ RxErrorFound);
#endif

    RxPtrOut=0;  // flush circular queue
    RxPtrIn=0;
    RxPtrEnd=0;
    RxStatus=0; 
    ChkSum=0;
    Err = 0;
}

