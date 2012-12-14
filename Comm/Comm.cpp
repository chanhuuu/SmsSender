#include "../stdafx.h"
#include "Comm.h"

typedef struct{
    char* pPort;
    HANDLE hComm;
} ComPort,*PComPort;

int defaltIndex = -1;

ComPort ComPool[COMPOOL_SIZE];

BOOL IsOpened(const char* pPort){
    BOOL result = false;
    for(int i=0;i<COMPOOL_SIZE;i++)
    {
        if(ComPool[i].pPort == NULL)
            return false;
        if(strcmp(ComPool[i].pPort,pPort)==0)
            result = true;
    }
    return result;
}

HANDLE HandleOfPort(const char* pPort,int &index = defaltIndex){
    HANDLE result = NULL;
    index = -1;
    for(int i=0;i<COMPOOL_SIZE;i++)
    {
        if(ComPool[i].pPort!=NULL)
        {
            if(strcmp(ComPool[i].pPort,pPort)==0)
            {
                result = ComPool[i].hComm;
                index = i;
                break;
            }
        }
        
    }
    return result;
}

int searchValidIndex(const char* pPort){
    int result =-1;
    for(int i=0;i<COMPOOL_SIZE;i++)
    {
        if(ComPool[i].hComm == NULL)
        {
            result = i;
            break;
        }
    }
    return result;
}

// �򿪴���
// ����: pPort - �������ƻ��豸·��������"COM1"��"\\.\COM1"���ַ�ʽ�������ú���
//       nBaudRate - ������
//       nParity - ��żУ��
//       nByteSize - �����ֽڿ��
//       nStopBits - ֹͣλ
BOOL OpenComm(const char* pPort, int nBaudRate, int nParity, int nByteSize, int nStopBits)
{
    if(IsOpened(pPort))
        return true;

	DCB dcb;		// ���ڿ��ƿ�
    
	COMMTIMEOUTS timeouts = {	// ���ڳ�ʱ���Ʋ���
		100,				// ���ַ������ʱʱ��: 100 ms
		1,					// ������ʱÿ�ַ���ʱ��: 1 ms (n���ַ��ܹ�Ϊn ms)
		500,				// ������(�����)����ʱʱ��: 500 ms
		1,					// д����ʱÿ�ַ���ʱ��: 1 ms (n���ַ��ܹ�Ϊn ms)
		100};				// ������(�����)д��ʱʱ��: 100 ms


	HANDLE hComm = CreateFile(pPort,	// �������ƻ��豸·��
			GENERIC_READ | GENERIC_WRITE,	// ��д��ʽ
			0,				// ����ʽ����ռ
			NULL,			// Ĭ�ϵİ�ȫ������
			OPEN_EXISTING,	// ������ʽ
			0,				// ���������ļ�����
			NULL);			// �������ģ���ļ�
	
	if(hComm == INVALID_HANDLE_VALUE) return FALSE;		// �򿪴���ʧ��

	GetCommState(hComm, &dcb);		// ȡDCB

	dcb.BaudRate = nBaudRate;
	dcb.ByteSize = nByteSize;
	dcb.Parity = nParity;
	dcb.StopBits = nStopBits;
    dcb.fBinary = TRUE;
    dcb.fDtrControl =RTS_CONTROL_ENABLE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;
    dcb.fOutxCtsFlow = FALSE;

	SetCommState(hComm, &dcb);		// ����DCB

	SetupComm(hComm, 4096, 1024);	// �������������������С

	SetCommTimeouts(hComm, &timeouts);	// ���ó�ʱ

    int index  =searchValidIndex(pPort);
    if((index>=0)&&(index<COMPOOL_SIZE))
    {
        PComPort pComPort = new ComPort();
        pComPort->hComm = hComm;
        pComPort->pPort = (char*)malloc(8*sizeof(char*));
        strcpy(pComPort->pPort,pPort);
        //pComPort->pPort = (char *)pPort;
        ComPool[index] = *pComPort;
    }
	return TRUE;
}

// �رմ���
BOOL CloseComm(const char* pPort)
{
    BOOL result =false;
    int index = -1;
    HANDLE hComm = HandleOfPort(pPort,index);
    if(hComm != NULL)
    {
	    result =  CloseHandle(hComm);
        if(result)
            ComPool[index].hComm = NULL;
    }
    return result;
}

// д����
// ����: pData - ��д�����ݻ�����ָ��
//       nLength - ��д�����ݳ���
// ����: ʵ��д������ݳ���
int WriteComm(const char* pPort,void* pData, int nLength)
{
	DWORD dwNumWrite =0 ;	// ���ڷ��������ݳ���
    HANDLE hComm = HandleOfPort(pPort);
    if(hComm != NULL)
    {
        PurgeComm(hComm,PURGE_RXCLEAR);
	    WriteFile(hComm, pData, (DWORD)nLength, &dwNumWrite, NULL);
    }
	return (int)dwNumWrite;
}

// ������
// ����: pData - ���������ݻ�����ָ��
//       nLength - ������������ݳ���
// ����: ʵ�ʶ��������ݳ���
int ReadComm(const char* pPort,void* pData, int nLength)
{
	DWORD dwNumRead = 0;	// �����յ������ݳ���
    HANDLE hComm = HandleOfPort(pPort);
    if(hComm != NULL)
    {
        
	    ReadFile(hComm, pData, (DWORD)nLength, &dwNumRead, NULL);
/*        PurgeComm(hComm, PURGE_TXABORT|
		  PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);*/
    }
	
	return (int)dwNumRead;
}


void InitPool()
{
    for(int i=0;i<COMPOOL_SIZE;i++)
    {
        ComPool[i].hComm = NULL;
        ComPool[i].pPort = NULL;
    }
}