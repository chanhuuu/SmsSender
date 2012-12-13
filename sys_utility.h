// sys_utility.h: interface for the SysUtility class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SYS_UTILITY_H__F46F3E49_B3CB_42C6_86DA_EED8BF4982D6__INCLUDED_)
#define AFX_SYS_UTILITY_H__F46F3E49_B3CB_42C6_86DA_EED8BF4982D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class SysUtility  
{
public:
	SysUtility();
	virtual ~SysUtility();

    //ϵͳ��Ŀ¼
    static CString getBasePath();
    //ϵͳ��ʱĿ¼
    static CString getTempPath();
    //ϵͳImageĿ¼
    static CString getImagePath();

    static CString getTempBmp();
    static CString getTempJpg();

    HMODULE GetCurrentModule(); 

};

#endif // !defined(AFX_SYS_UTILITY_H__F46F3E49_B3CB_42C6_86DA_EED8BF4982D6__INCLUDED_)
