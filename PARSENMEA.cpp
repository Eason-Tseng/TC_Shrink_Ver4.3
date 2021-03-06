//---------------------------------------------------------------------------
#include "PARSENMEA.h"

#include "SMEM.h"
#include "CTIMER.h"

//OT
#include "CSTC.h"
#include "SCREENOperStat.h"

//CCJ++
#include "CHexToInt.h"
//#include "CTC.h"
//#include "convert.h"
//CCJ--
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


//---------------------------------------------------------------------------
PARSENMEA::PARSENMEA(void)
{

}
//---------------------------------------------------------------------------
PARSENMEA::~PARSENMEA(void)
{
}

//---------------------------------------------------------------------------
bool PARSENMEA::ParseNMEABlock(int receiveBlockLength,BYTE *block,MESSAGEOK *messageIn,int *lastPacketIndex,int *maxMessageIndex)
{
    //�ѪR�ʥ]
    try
    {
        int i,j,k;

        i=0;                         //block[i]
        j=0;                         //messageIn[j]
        k=*lastPacketIndex+1;        //messageIn[j].packet[k]

        for (i=0; i<receiveBlockLength; i++)
        {
//      printf("_%C", messageIn[j].packet[k]=block[i]);

            if ( (block[i] == (const BYTE)0x24) )   // 0x24 == $
            {

                messageIn[j].packetLength = k;
                j++;
                k=0;
                messageIn[j].packet[k] = block[i];
                k++;

            }
            else
            {
                if (k < BUFFERSIZE)
                {
                    messageIn[j].packet[k] = block[i];
                    k++;
                }
                else
                {
                    for (int a=0; a<MSGINSIZE; a++)
                    {
                        bzero(messageIn[a].packet, BUFFERSIZE);
                        messageIn[a].cksStatus=false;
                        messageIn[a].success=false;
                        messageIn[a].UDPorRS232=0;
                        messageIn[a].ReadorWrite=0;

                        /* change
                        for (int b=0;b<BUFFERSIZE;b++) {
                          messageIn[a].packet[b]=0;
                          messageIn[a].cksStatus=false;
                          messageIn[a].success=false;
                          messageIn[a].UDPorRS232=0;
                          messageIn[a].ReadorWrite=0;
                        }
                        */
                    }
                    *lastPacketIndex=-1;
                    *maxMessageIndex=0;
                }   //end if (k < BUFFERSIZE)

            }     //end if
        }    //end for

        *maxMessageIndex=j;
        messageIn[j].packetLength=k;

        return true;

    }
    catch(...) {}
}


//---------------------------------------------------------------------------
bool PARSENMEA::vFindGPRMC(int *maxMessageIndex,MESSAGEOK *messageIn)
{
    try
    {
        unsigned char ucSpace;
        unsigned char ucStarLocation;
        unsigned char ucCKS, ucCKS2;

        char cCKSString[4];
        bzero(cCKSString, 4);

        if (*maxMessageIndex>=0)
        {
            for (int i=0; i<=*maxMessageIndex; i++)
            {
                if( messageIn[i].packet[3] == 'R' &&
                        messageIn[i].packet[4] == 'M' &&
                        messageIn[i].packet[5] == 'C' &&
                        messageIn[i].packetLength > 64
                  )
                {
                    ucSpace = 0;
//                   printf("\nHex:");
                    for(int j = 0; j < messageIn[i].packetLength; j++)
                    {
//                     printf("%2X ", messageIn[i].packet[j]);
                        if(messageIn[i].packet[j] == ',')
                        {
                            ++ucSpace;
                        }
                        if(messageIn[i].packet[j] == '*')
                        {
                            ucStarLocation = j;
                        }
                    }
//                   printf("\n");
//                   printf("get RMC space:%d\n", ucSpace);

                    //do checksum
                    ucCKS = 0;
                    for(int j = 1; j < ucStarLocation; j++)
                    {
                        ucCKS ^= messageIn[i].packet[j];
                    }
//                   printf("cal. cks:%X\n", ucCKS);

                    cCKSString[0] = messageIn[i].packet[ucStarLocation+1];
                    cCKSString[1] = messageIn[i].packet[ucStarLocation+2];
                    ucCKS2 = oHexToInt.htoi(cCKSString, 2);
//                   printf("string. cks2:%X\n", ucCKS2);

//                   printf("messageIn[i].packetLength:%d, ucStarLocation:%d\n", messageIn[i].packetLength, ucStarLocation);

                    if( (ucStarLocation + 5) == messageIn[i].packetLength && ucCKS == ucCKS2 && ( ucSpace == 11 || ucSpace == 12))       //cks x 2, 0x0D, 0x0A
                    {
                        messageIn[i].cksStatus = true;
                        messageIn[i].success = true;
                        printf("GPRMC , GNRMC Format OK!\n");
                    }
                }

            }
        }
    }
    catch(...) {}
}

//---------------------------------------------------------------------------
bool PARSENMEA::vAdjTimeByGPS(int *maxMessageIndex, MESSAGEOK *messageIn)
{
    try
    {

        bool bEnableUpdate;

        unsigned char ucTimeLocation = 0;
        unsigned char ucDataLocation = 0;
        unsigned char ucStatusLocation = 0;
        unsigned char ucGetSpace = 0;

        char cTmp[3] = { 0 };
        int siY;
        int siM;
        int siD;
        int siHour;
        int siMin;
        int siSec;

        struct tm time_str;
        struct tm *TSP = &time_str;
        time_t RunSec;

        if (*maxMessageIndex>=0)
        {
            for (int i=0; i <= *maxMessageIndex; i++)
            {
                if (messageIn[i].cksStatus == true)
                {
                    if (messageIn[i].success == true)
                    {
//           printf("go vAdjTimeByGPS!\n");

                        ucTimeLocation = 0;
                        ucDataLocation = 0;
                        ucStatusLocation = 0;
                        ucGetSpace = 0;
                        for(int j = 0; j < messageIn[i].packetLength; j++)
                        {
                            if(messageIn[i].packet[j] == ',')
                            {
                                ++ucGetSpace;
                            }
                            if(ucGetSpace == 1 && ucTimeLocation == 0)    //time
                            {
                                ucTimeLocation = j + 1;
                            }
                            if(ucGetSpace == 2 && ucStatusLocation == 0)    //status
                            {
                                ucStatusLocation = j + 1;
                            }
                            if(ucGetSpace == 9 && ucDataLocation == 0)    //date
                            {
                                ucDataLocation = j + 1;
                            }
                        }
//           printf("ucStatusLocation:%d\n", ucStatusLocation);
//           printf("ucDataLocation:%d\n", ucDataLocation);
//           printf("ucTimeLocation:%d\n", ucTimeLocation);

                        if( messageIn[i].packet[7] != 0x2C && messageIn[i].packet[ucStatusLocation] == 'A' )
                        {

                            bzero(cTmp, 3);
                            cTmp[0] = messageIn[i].packet[ucTimeLocation];
                            ucTimeLocation++;
                            cTmp[1] = messageIn[i].packet[ucTimeLocation];
                            ucTimeLocation++;
                            sscanf(cTmp, "%d", &siHour);
                            time_str.tm_hour = siHour;
//               printf("siHour:%d\n", siHour);

                            bzero(cTmp, 3);
                            cTmp[0] = messageIn[i].packet[ucTimeLocation];
                            ucTimeLocation++;
                            cTmp[1] = messageIn[i].packet[ucTimeLocation];
                            ucTimeLocation++;
                            sscanf(cTmp, "%d", &siMin);
                            time_str.tm_min = siMin;
//               printf("siMin:%d\n", siMin);

                            bzero(cTmp, 3);
                            cTmp[0] = messageIn[i].packet[ucTimeLocation];
                            ucTimeLocation++;
                            cTmp[1] = messageIn[i].packet[ucTimeLocation];
                            ucTimeLocation++;
                            sscanf(cTmp, "%d", &siSec);
                            time_str.tm_sec = siSec;
//               printf("siSec:%d\n", siSec);

                            bzero(cTmp, 3);
                            cTmp[0] = messageIn[i].packet[ucDataLocation];
                            ucDataLocation++;
                            cTmp[1] = messageIn[i].packet[ucDataLocation];
                            ucDataLocation++;
                            sscanf(cTmp, "%d", &siD);
//               printf("siD:%d\n", siD);

                            bzero(cTmp, 3);
                            cTmp[0] = messageIn[i].packet[ucDataLocation];
                            ucDataLocation++;
                            cTmp[1] = messageIn[i].packet[ucDataLocation];
                            ucDataLocation++;
                            sscanf(cTmp, "%d", &siM);
//               printf("siM:%d\n", siM);

                            bzero(cTmp, 3);
                            cTmp[0] = messageIn[i].packet[ucDataLocation];
                            ucDataLocation++;
                            cTmp[1] = messageIn[i].packet[ucDataLocation];
                            ucDataLocation++;
                            sscanf(cTmp, "%d", &siY);
//               printf("siY:%d\n", siY);

                            time_str.tm_year = siY + 100;
                            time_str.tm_mon = siM - 1;
                            time_str.tm_mday = siD;

                            time_str.tm_isdst = -1;
                            RunSec = mktime(&time_str);

                            RunSec = RunSec + 28800;                                         // now RunSec is GPS time

                            if(smem.GetcFace() == cOPERSTAT)                                 //now in show gps page
                            {
                                screenOperStat.vShowGPSTime(RunSec);
                            }

                            TSP = localtime(&RunSec);

                            char date[22] = {0},time[17] = {0};


                            /*OT FIX 20060302 */
                            if(TSP->tm_year > 105 &&
                                    ( (smem.vGetBOOLData(GPS_SYNC) == true ) ||
                                      (TSP->tm_min == 6 && TSP->tm_sec == 0) || (TSP->tm_min == 36 && TSP->tm_sec == 0)
                                    )
                              )
                            {
                                bEnableUpdate = smem.vGetBOOLData(EnableUpdateRTC);

                                if(bEnableUpdate == true)
                                {

                                    stc.TimersRead_BeforeResetCMOSTime();  //OTBUG =1
                                    _intervalTimer.TimersRead_BeforeResetCMOSTime();

                                    smem.vSetTimerMutexRESET(1);
                                    while(smem.vGetTimerMutexCTIMER() == 0 || smem.vGetTimerMutexCSTC() == 0)
                                    {
                                        usleep(100);
                                    }

//                   _intervalTimer.TimersRead_BeforeResetCMOSTime();
//                   stc.TimersRead_BeforeResetCMOSTime();  //OTBUG =1
                                    if(smem.sGPSGetTimeSwitch() == 1)
                                    {
                                        smem.vSetSystemClockTime(TSP->tm_year+1900, TSP->tm_mon+1, TSP->tm_mday, TSP->tm_hour, TSP->tm_min, TSP->tm_sec);
                                        //smem.SetSegmentChange(true);
                                    }
                                    /*move to smem
                                                       sprintf(date,"date -s %#02d%#02d%#02d%#02d%#04d",TSP->tm_mon+1,TSP->tm_mday,TSP->tm_hour,TSP->tm_min,TSP->tm_year+1900);
                                                       printf("\n%s\n",date);
                                                       system(date);

                                                       sprintf(time,"date -s %#02d:%#02d:%#02d",TSP->tm_hour,TSP->tm_min,TSP->tm_sec);
                                                       printf("\n%s\n",time);
                                                       system(time);

                                                       system("hwclock -w");
                                    */
                                    _intervalTimer.TimersReset_AfterResetCMOSTime();  //OTBUG =1
                                    stc.TimersReset_AfterResetCMOSTime();

//                   smem.vSetTimerMutexCTIMER(false);
//                   smem.vSetTimerMutexCSTC(false);
                                    smem.vSetTimerMutexRESET(0);

                                    smem.vSetAdjcount(0);
                                    smem.vSendTimerUpdateToCCJ_5F9E();

                                    system("hwclock -w");
                                }

                                smem.vSetBOOLData(GPS_SYNC, false);

                            }
                            else
                            {
//                 uiAdjTimeCount = 3601;                                         // fix time immd.
                            }

                        }
                        smem.vSaveGPSStatus(true);
                    }
                    else smem.vSaveGPSStatus(false);
                }
            }
        }
    }
    catch(...) {}
}



//---------------------------------------------------------------------------
bool PARSENMEA::EchoToGUI(int *maxMessageIndex,MESSAGEOK *messageIn,char *deviceName)
{
    try
    {
        if (*maxMessageIndex>=0)
        {
            for (int i=0; i<=*maxMessageIndex; i++)
            {
                if (messageIn[i].cksStatus==true)
                {
                    if (messageIn[i].success==true)
                    {

                        //OTADD for packet display to screen
//                   screenCurrentCommPacket.vRefreshCurrentScreenPacket(messageIn[i].packet, messageIn[i].packetLength, "0");

                        char tempBuff[256],buff[2048]="";
                        char portName[200]="[RECEIVE] ";
                        char temp[6]=" --";

                        strcat(portName,deviceName);
                        strcat(portName,temp);

                        strcat(buff,portName);

                        for (int j=0; j<messageIn[i].packetLength; j++)
                        {
                            sprintf(tempBuff,"%C",messageIn[i].packet[j]);
                            strcat(buff,tempBuff);
                        }

                        /*+++OT_No_Printf+++*/
                        printf("%s\n",buff);
                        /*---OT_No_Printf---*/
                    }
                }
            }  //end for (int i=0 ; i < maxMessageIndex ; i++)
        }  //end if (maxMessageIndex >= 0)

        return true;

    }
    catch(...) {}
}


//---------------------------------------------------------------------------
bool PARSENMEA::MoveLastData(int *maxMessageIndex,int *lastPacketIndex,MESSAGEOK *messageIn)
{
    try
    {
        if (messageIn[*maxMessageIndex].cksStatus == false)
        {
            for (int j=0; j<messageIn[*maxMessageIndex].packetLength; j++)
                messageIn[0].packet[j]=messageIn[*maxMessageIndex].packet[j];
            messageIn[0].packetLength=messageIn[*maxMessageIndex].packetLength;
            *lastPacketIndex=messageIn[*maxMessageIndex].packetLength-1;

            for (int a=1; a<MSGINSIZE; a++)
            {
                messageIn[a].cksStatus=false;
                messageIn[a].success=false;
                messageIn[a].UDPorRS232=0;
                messageIn[a].ReadorWrite=0;
//        bzero(messageIn[a].packet, sizeof(messageIn[a].packet));
                bzero(messageIn[a].packet, BUFFERSIZE);
//      for (int b=0;b<BUFFERSIZE;b++) {
//           messageIn[a].packet[b]=0;
//      }
            }

        }
        else

        {
            int length = messageIn[*maxMessageIndex].packetLength;

            {

                for (int a=0; a<MSGINSIZE; a++)
                {
                    messageIn[a].cksStatus=false;
                    messageIn[a].success=false;
                    messageIn[a].UDPorRS232=0;
                    messageIn[a].ReadorWrite=0;
//              bzero(messageIn[a].packet, sizeof(messageIn[a].packet));
                    bzero(messageIn[a].packet, BUFFERSIZE);
//            for (int b=0;b<BUFFERSIZE;b++) {
//                 messageIn[a].packet[b]=0;
//            }
                }

                *lastPacketIndex=-1;
                *maxMessageIndex=0;
            }
        }

        return true;

    }
    catch(...) {}
}
