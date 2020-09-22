#ifndef SCREENTFDStatusH
#define SCREENTFDStatusH

#include "SCREEN.h"
#include "var.h"
//---------------------------------------------------------------------------
class SCREENTFDStatus:public SCREEN
{
    public:
        SCREENTFDStatus(void);
        ~SCREENTFDStatus(void);

        void DisplayTFDStatus(void);
        void DoKeyWork(BYTE);                                                   //�ھ�KEY���ȧ@��

        void DisplayTFDStatOK(void);
        void DisplayTFDModuleFailure(void);
        void DisplayTFDTriggerFailure(void);
        void DisplayTFDCCTVFailure(void);
        void DisplayTFDDIOOutput(void);

        void vChangeScreenTFDStatus(bool, unsigned char, unsigned char);

    private:

        unsigned short int dbOperStat;                                          //(0:���\�{���ާ@,1:�����\�{���ާ@,2:�u���\�{���d��)
        unsigned short int requestResult;                                       //0:accept,1:��Ū,2�����\,3:Timeout

        BYTE TFDStatusBitmap[3840];                                                //�ťթ���

        void LoadBitmapFromFile(void);
        void DisplayDBOperStat(void);
        void ClearAll(void);


        void DoKeyF1Work(void);
        void DoKeyF4Work(void);
        void DoKeyEnterWork(void);

        bool bTFDOK;
        bool bTFDMoudleFailure;
        bool bTFDTriggerFailure;
        bool bTFDCCTVFailure;
        bool bTFDDIOOutput;
};
//---------------------------------------------------------------------------
extern SCREENTFDStatus screenTFDStatus;
#endif