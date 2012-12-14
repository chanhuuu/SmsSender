// SmsTraffic.h: interface for the CSmsTraffic class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SMSTRAFFIC_H__3A4D81DE_C363_42D6_8A47_3BA017BFBF56__INCLUDED_)
#define AFX_SMSTRAFFIC_H__3A4D81DE_C363_42D6_8A47_3BA017BFBF56__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Sms.h"
#include "../Comm/Comm.h"

#define MAX_SM_SEND		128		// ���Ͷ��г���
#define MAX_SM_RECV		128		// ���ն��г���
#define WM_INFO_MESSAGE (WM_USER+100)

enum Status
{
    Idle,
    Busy,
    Halt
};



class CSmsTraffic  
{
public:
	CSmsTraffic();
	virtual ~CSmsTraffic();

	int m_nSendIn;		// ���Ͷ��е�����ָ��
	int m_nSendOut;		// ���Ͷ��е����ָ��

	int m_nRecvIn;		// ���ն��е�����ָ��
	int m_nRecvOut;		// ���ն��е����ָ��

	SM_PARAM m_SmSend[MAX_SM_SEND];		// ���Ͷ���Ϣ����
	SM_PARAM m_SmRecv[MAX_SM_SEND];		// ���ն���Ϣ����

	CRITICAL_SECTION m_csSend;		// �뷢����ص��ٽ��
	CRITICAL_SECTION m_csRecv;		// �������ص��ٽ��

	HANDLE m_hKillThreadEvent;		// ֪ͨ���̹߳رյ��¼�
	HANDLE m_hThreadKilledEvent;	// ���߳�����رյ��¼�

	void PutSendMessage(SM_PARAM* pSmParam);	// ������Ϣ���뷢�Ͷ���
	BOOL GetSendMessage(SM_PARAM* pSmParam);	// �ӷ��Ͷ�����ȡһ������Ϣ
	void PutRecvMessage(SM_PARAM* pSmParam, int nCount);	// ������Ϣ������ն���
	BOOL GetRecvMessage(SM_PARAM* pSmParam);	// �ӽ��ն�����ȡһ������Ϣ

    int getUnSends();//��ȡ�����Ͷ�����

	static UINT SmThread(LPVOID lpParam);	// ����Ϣ�շ��������߳�

    int m_sendTimes;//���ŷ��ʹ���,�����ж�ͣ��״̬
    int m_readFailedTimes;//�ն���ʧ�ܴ���
    int m_sendFailedSms;//�������Ͷ���ʧ������
public:
    char* Comport();
    void set_Comport(char* port);
    void set_Handle(HWND  handle);
    Status getStatus();
private:
    char* m_pComport;
    static HWND  m_Handle;//��ʾ��Ϣ�Ĵ�����
    static void info(const char* msg1,const char* msg2=NULL,const char* msg3=NULL,const char* msg4=NULL,const char* msg5=NULL,const char* msg6=NULL,const char* msg7=NULL);


};


#endif // !defined(AFX_SMSTRAFFIC_H__3A4D81DE_C363_42D6_8A47_3BA017BFBF56__INCLUDED_)
