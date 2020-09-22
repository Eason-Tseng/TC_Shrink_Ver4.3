//---------------------------------------------------------------------------
#include "PDEVICE.h"
#include <stdio.h>

//---------------------------------------------------------------------------
PDEVICE::PDEVICE(void)
{
}
//---------------------------------------------------------------------------
PDEVICE::~PDEVICE(void)
{
}
//---------------------------------------------------------------------------
bool PDEVICE::DoWorkViaProtocol(MESSAGEOK message)
{
    try
    {
        switch (message.protocol)
        {
        case PROTOCOLUNKNOW:                                                //���M������w
        case PROTOCOL77:                                                    //��q��77�~���q�T��w
        case PROTOCOL87:                                                    //��q��87�~���q�T��w
        case PROTOCOL92:                                                    //��q��92�~���q�T��w
        case PROTOCOLNTCIP:                                                 //��q��NTCIP�q�T��w
            break;
            //�Ĥ@�ؤ���:�g��]��:�F�ͭ��O������L
        case PROTOCOLKEYPAD:                                                //�F�ͭ��O������L�q�T��w
            pDeviceKeypad.DoWorkViaPDevice(message);
            break;
            //�ĤG�ؤ���:�g��]��:RTMS
        case PROTOCOLRTMS:                                                  //RTMS�q�T��w
            pDeviceRTMS.DoWorkViaPDevice(message);
            break;
            //�ĤT�ؤ���:�g��]��:SMART SENSOR
        case PROTOCOLSS:                                                    //SMART SENSOR�q�T��w
            pDeviceSS.DoWorkViaPDevice(message);
            break;
            //�ĥ|�ؤ���:�g��]��:�������O�˼�
        case PROTOCOLOWNWAY:                                                //�������O�˼Ƴq�T��w
            break;
            //�Ĥ��ؤ���:�g��]��:���Ŭ��O�˼�
        case PROTOCOLEOI:                                                   //���Ŭ��O�˼Ƴq�T��w
            break;
            //�Ĥ��ؤ���:�g��]��:�x�_�õaCMS
        case PROTOCOLLISCMS:                                                //�x�_�õaCMS�q�T��w
            break;
            //�ĤC�ؤ���:�g��]��:����O����O
        case PROTOCOLTRAFFICLIGHT:                                                //�F�ͬ���O����q�T��w
            pDeviceTrafficLight.DoWorkViaPDevice(message);
            break;

        case PROTOCOLREVMANUALPAD:
            pDeviceRevManualPad.DoWorkViaPDevice(message);
            break;
        case PROTOCOLPEDESTRIAN:
            pDevicePedPushButton.DoWorkViaPDevice(message);
            break;



        default:
            break;
        }

        return true;

    }
    catch (...) {}
}
//---------------------------------------------------------------------------