// Config.h: interface for the CConfig class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONFIG_H__3A312D87_9238_455D_B675_7861F13CB211__INCLUDED_)
#define AFX_CONFIG_H__3A312D87_9238_455D_B675_7861F13CB211__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



class CConfig  
{
public:
	CConfig();
	virtual ~CConfig();

    CString SmsC();
    int BautRate();
    int PoolSize();
private:
    CString m_Smsc;//�������ĺ���
    int m_BautRate;//������
    int m_poolSize;//���ڳش�С����Ч��������

    CString m_exePath,m_fileName;

    CString GetParaStr(LPTSTR mainKey,LPTSTR key,CString strDefaul);
    void SetParaInt(LPTSTR mainKey,LPTSTR key,int intValue);
    
    void SetParaStr(LPTSTR mainKey,LPTSTR key,CString strValue);
    
};

#endif // !defined(AFX_CONFIG_H__3A312D87_9238_455D_B675_7861F13CB211__INCLUDED_)
