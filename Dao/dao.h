// dao.h: interface for the CDao class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DAO_H__8DB86AB4_06A5_4944_A4C0_647D1184DBCB__INCLUDED_)
#define AFX_DAO_H__8DB86AB4_06A5_4944_A4C0_647D1184DBCB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "../ocilib/include/ocilib.h" 
#include "../Sms/SmsTraffic.h"

typedef enum CODEC{cdcWestern=0,cdcBIN=4,cdcGB=8};
typedef enum SendStatus{ssWaitToSend=0,ssSending,ssSendSuccess,ssSendFail};
typedef struct
{
    int ID;
    CString TeleCode;
    CODEC CodeC;
    CString Msg;
}RecvSMS,*PRecvSMS;

typedef struct
{
    int ID;
    CString TeleCode;
    CODEC CodeC;
    CString Msg;
    SendStatus Status;  
}SendSMS,*PSendSMS;


//Interface Methods
void err_handler(OCI_Error *err);//�쳣����

PSendSMS* getSendSms(int &smsNumber);//��ȡ�����Ͷ���

BOOL setStatus(int id,SendStatus status,BOOL needClean,OCI_Connection *cn=NULL);//���¶���״̬

BOOL insertRecvSms(SM_PARAM* pparam);


#endif // !defined(AFX_DAO_H__8DB86AB4_06A5_4944_A4C0_647D1184DBCB__INCLUDED_)
