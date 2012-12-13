// SmsTraffic.cpp: implementation of the CSmsTraffic class.
//
//////////////////////////////////////////////////////////////////////

#include "../stdafx.h"
#include "SmsTraffic.h"
#include"../Dao/dao.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
HWND  CSmsTraffic::m_Handle = 0;
const int IDLE_SMS_NUMBER = 5;//���ж��Ż���ֵ������������С�ڸ�ֵ��Ϊ����
const int SEND_TIMES_FAILED = 2; // ��������ʧ�ܴ����������жϷ���ʧ�ܣ�
const int SEND_SMS_FAILED = 2;// ��������ʧ�ܶ������������ж�SIMͣ����

CSmsTraffic::CSmsTraffic()
{
	m_nSendIn = 0;
	m_nSendOut = 0;
	m_nRecvIn = 0;
	m_nRecvOut = 0;
    m_sendTimes = 0;
    m_sendFailedSms = 0;

	m_hKillThreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hThreadKilledEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	InitializeCriticalSection(&m_csSend);
	InitializeCriticalSection(&m_csRecv);

	// �������߳�
	AfxBeginThread(SmThread, this, THREAD_PRIORITY_NORMAL);
}

CSmsTraffic::~CSmsTraffic()
{
	SetEvent(m_hKillThreadEvent);			// �����ر����̵߳��ź�
	WaitForSingleObject(m_hThreadKilledEvent, INFINITE);	// �ȴ����̹߳ر�

	DeleteCriticalSection(&m_csSend);
	DeleteCriticalSection(&m_csRecv);

	CloseHandle(m_hKillThreadEvent);
	CloseHandle(m_hThreadKilledEvent);
}

// ��һ������Ϣ���뷢�Ͷ���
void CSmsTraffic::PutSendMessage(SM_PARAM* pparam)
{
	EnterCriticalSection(&m_csSend);
	memcpy(&m_SmSend[m_nSendIn], pparam, sizeof(SM_PARAM));
	m_nSendIn++;
	if (m_nSendIn >= MAX_SM_SEND)  m_nSendIn = 0;
	LeaveCriticalSection(&m_csSend);
}

// �ӷ��Ͷ�����ȡһ������Ϣ
BOOL CSmsTraffic::GetSendMessage(SM_PARAM* pparam)
{
	BOOL fSuccess = FALSE;

	EnterCriticalSection(&m_csSend);

	if (m_nSendOut != m_nSendIn)
	{
		memcpy(pparam, &m_SmSend[m_nSendOut], sizeof(SM_PARAM));

		m_nSendOut++;
		if (m_nSendOut >= MAX_SM_SEND)  m_nSendOut = 0;

		fSuccess = TRUE;
	}

	LeaveCriticalSection(&m_csSend);

	return fSuccess;
}

// ������Ϣ������ն���
void CSmsTraffic::PutRecvMessage(SM_PARAM* pparam, int nCount)
{
	EnterCriticalSection(&m_csRecv);

	for (int i = 0; i < nCount; i++)
	{
		memcpy(&m_SmRecv[m_nRecvIn], pparam, sizeof(SM_PARAM));
	
		m_nRecvIn++;
		if (m_nRecvIn >= MAX_SM_RECV)  m_nRecvIn = 0;

		pparam++;
	}

	LeaveCriticalSection(&m_csRecv);
}

// �ӽ��ն�����ȡһ������Ϣ
BOOL CSmsTraffic::GetRecvMessage(SM_PARAM* pparam)
{
	BOOL fSuccess = FALSE;

	EnterCriticalSection(&m_csRecv);

	if (m_nRecvOut != m_nRecvIn)
	{
		memcpy(pparam, &m_SmRecv[m_nRecvOut], sizeof(SM_PARAM));

		m_nRecvOut++;
		if (m_nRecvOut >= MAX_SM_RECV)  m_nRecvOut = 0;

		fSuccess = TRUE;
	}

	LeaveCriticalSection(&m_csRecv);

	return fSuccess;
}

char* CSmsTraffic::Comport(){
    return this->m_pComport;
}
void CSmsTraffic::set_Comport(char* port)
{
    this->m_pComport = port;
}

int CSmsTraffic::getUnSends()
{
    int result = 0;
    EnterCriticalSection(&m_csSend);
    result = m_nSendIn - m_nSendOut+1;
    LeaveCriticalSection(&m_csSend);
    return result;
}

Status CSmsTraffic::getStatus()
{
    if(m_sendFailedSms>=SEND_SMS_FAILED)
        return Halt;
    else
    {
        int unSendSms = m_nSendIn - m_nSendOut+1;
        if(unSendSms >= IDLE_SMS_NUMBER)
            return Busy;
        else
            return Idle;
    }
    
    
}

void CSmsTraffic::set_Handle(HWND handle)
{
    CSmsTraffic::m_Handle = handle;
}

void CSmsTraffic::info(const char* msg1,const char* msg2,const char* msg3,const char* msg4,const char* msg5,const char* msg6,const char* msg7)
{
    char* pMsg = (char*)malloc(256);
    memset(pMsg,0,256);
    pMsg = strcat(pMsg,msg1);
    if(msg2 != NULL)
        pMsg = strcat(pMsg,msg2);
    if(msg3 != NULL)
        pMsg = strcat(pMsg,msg3);
    if(msg4 != NULL)
        pMsg = strcat(pMsg,msg4);
    if(msg5 != NULL)
        pMsg = strcat(pMsg,msg5);
    if(msg6 != NULL)
        pMsg = strcat(pMsg,msg6);
    if(msg7 != NULL)
        pMsg = strcat(pMsg,msg7);
    SendMessage(m_Handle,WM_INFO_MESSAGE,(UINT)pMsg,0);
}    


UINT CSmsTraffic::SmThread(LPVOID lParam)
{

	CSmsTraffic* p=(CSmsTraffic *)lParam;	// this
    char *pPort = (char *)malloc(20);
    strcpy(pPort,p->m_pComport);
	int nMsg;				// �յ�����Ϣ����
	int nDelete;			// Ŀǰ����ɾ���Ķ���Ϣ���
	SM_BUFF buff;			// ���ն���Ϣ�б�Ļ�����
	SM_PARAM param[256];	// ����/���ն���Ϣ������
	CTime tmOrg, tmNow;		// �ϴκ����ڵ�ʱ�䣬���㳬ʱ��
	enum {
		stBeginRest,				// ��ʼ��Ϣ/��ʱ
		stContinueRest,				// ������Ϣ/��ʱ
		stSendMessageRequest,		// ���Ͷ���Ϣ
		stSendMessageResponse,		// ��ȡ����Ϣ�б�������
		stSendMessageWaitIdle,		// ���Ͳ��ɹ����ȴ�GSM����
		stReadMessageRequest,		// ���Ͷ�ȡ����Ϣ�б������
		stReadMessageResponse,		// ��ȡ����Ϣ�б�������
		stDeleteMessageRequest,		// ɾ������Ϣ
		stDeleteMessageResponse,	// ɾ������Ϣ
		stDeleteMessageWaitIdle,	// ɾ�����ɹ����ȴ�GSM����
		stExitThread				// �˳�
	} nState;				// ������̵�״̬

	// ��ʼ״̬
	nState = stBeginRest;

	// ���ͺͽ��մ����״̬ѭ��
	while (nState != stExitThread)
	{
		switch(nState)
		{
			case stBeginRest:
                info(_TEXT("���Ż���"),pPort,_TEXT(" ׼������..."));
				tmOrg = CTime::GetCurrentTime();
				nState = stContinueRest;
				break;

			case stContinueRest:
				Sleep(300);
				tmNow = CTime::GetCurrentTime();
				if (p->GetSendMessage(&param[0]))
				{
					nState = stSendMessageRequest;	// �д�������Ϣ���Ͳ���Ϣ��
                    info(_TEXT("���Ż���"),pPort,_TEXT(" ��ȡ���ųɹ�..."));
				}
				else if (tmNow - tmOrg >= 5)		// ��������Ϣ���пգ���Ϣ5��
				{
					nState = stReadMessageRequest;	// ת����ȡ����Ϣ״̬
				}
				break;

			case stSendMessageRequest:
				gsmSendMessage(pPort,&param[0]);
				memset(&buff, 0, sizeof(buff));
				tmOrg = CTime::GetCurrentTime();
				nState = stSendMessageResponse;
                p->m_sendTimes++;
				break;

			case stSendMessageResponse:
				Sleep(100);
				tmNow = CTime::GetCurrentTime();
				switch (gsmGetResponse(pPort,&buff))
				{
					case GSM_OK: 
                        setStatus(param[0].index,ssSendSuccess,true);
                        info(_TEXT("���Ż���"),pPort,_TEXT(" ����룺"),param[0].TPA,_TEXT(" ������Ϣ��"),param[0].TP_UD,"�ɹ���");
                        p->m_sendTimes = 0;
                        p->m_sendFailedSms = 0;
						nState = stBeginRest;
						break;
					case GSM_ERR:
						nState = stSendMessageWaitIdle;
                        if(p->m_sendTimes >= SEND_TIMES_FAILED)
                        {
                            setStatus(param[0].index,ssSendFail,true);
                            info(_TEXT("���Ż���"),pPort,_TEXT(" ����룺"),param[0].TPA,_TEXT(" ������Ϣ��"),param[0].TP_UD,"ʧ�ܣ�");
                            p->m_sendTimes = 0;
                            p->m_sendFailedSms++;
                            nState = stBeginRest;
                        }
						break;
					default:
						if (tmNow - tmOrg >= 10)		// 10�볬ʱ
						{
							nState = stSendMessageWaitIdle;
						}
				}
				break;

			case stSendMessageWaitIdle:
				Sleep(500);
				nState = stSendMessageRequest;		
				break;

			case stReadMessageRequest:
				gsmReadMessageList(pPort);
				memset(&buff, 0, sizeof(buff));
				tmOrg = CTime::GetCurrentTime();
				nState = stReadMessageResponse;
				break;

			case stReadMessageResponse:
				Sleep(100);
				tmNow = CTime::GetCurrentTime();
				switch (gsmGetResponse(pPort,&buff))
				{
					case GSM_OK: 
//						TRACE("  GSM_OK %d\n", tmNow - tmOrg);
						nMsg = gsmParseMessageList(param, &buff);
						if (nMsg > 0)
						{
							p->PutRecvMessage(param, nMsg);
                            for(int i=0;i<nMsg;i++)
                            {
                                insertRecvSms(&param[i]);
                                info(_TEXT("���Ż���"),pPort,_TEXT(" ���յ���Ϣ��"),param[0].TP_UD);
                            }
							nDelete = 0;
							nState = stDeleteMessageRequest;
						}
						else
						{
							nState = stBeginRest;
						}
						break;
					case GSM_ERR:
//						TRACE("  GSM_ERR %d\n", tmNow - tmOrg);
                        info(_TEXT("���Ż���"),pPort,_TEXT(" ���մ���"));
						nState = stBeginRest;
						break;
					default:
//						TRACE("  GSM_WAIT %d\n", tmNow - tmOrg);
						if (tmNow - tmOrg >= 15)		// 15�볬ʱ
						{
//							TRACE("  Timeout!\n");
                            //info(_TEXT("���Ż���"),pPort,_TEXT(" ���ճ�ʱ"));
							nState = stBeginRest;
						}
				}
				break;

			case stDeleteMessageRequest:
				if (nDelete < nMsg)
				{
					gsmDeleteMessage(pPort,param[nDelete].index);
                    //info(_TEXT("���Ż���"),pPort,_TEXT("���������ռ���"));
					memset(&buff, 0, sizeof(buff));
					tmOrg = CTime::GetCurrentTime();
					nState = stDeleteMessageResponse;
				}
				else
				{
					nState = stBeginRest;
				}
				break;

			case stDeleteMessageResponse:
				Sleep(100);
				tmNow = CTime::GetCurrentTime();
				switch (gsmGetResponse(pPort,&buff))
				{
					case GSM_OK: 
                        info(_TEXT("���Ż���"),pPort,_TEXT("���ռ���ɾ��1������"));
						nDelete++;
						nState = stDeleteMessageRequest;
						break;
					case GSM_ERR:
                        info(_TEXT("���Ż���"),pPort,_TEXT("ɾ������ʧ��"));
						nState = stDeleteMessageWaitIdle;
						break;
					default:
						if (tmNow - tmOrg >= 5)		// 5�볬ʱ
						{
//							TRACE("  Timeout!\n");
							nState = stBeginRest;
						}
				}
				break;

			case stDeleteMessageWaitIdle:
				Sleep(500);
				nState = stDeleteMessageRequest;		// ֱ��ɾ���ɹ�Ϊֹ
				break;
		}

		// ����Ƿ��йرձ��̵߳��ź�
		DWORD dwEvent = WaitForSingleObject(p->m_hKillThreadEvent, 20);
		if (dwEvent == WAIT_OBJECT_0)  nState = stExitThread;
	}

	// �ø��߳̽�����־
	SetEvent(p->m_hThreadKilledEvent);
    free(pPort);
	return 9999;
}






