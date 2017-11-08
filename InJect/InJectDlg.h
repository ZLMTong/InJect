
// InJectDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "windows.h"
#include <TlHelp32.h>
#include <shlobj.h>
#include <processthreadsapi.h>
#include <vector>
using namespace std;


// CInJectDlg 对话框
class CInJectDlg : public CDialogEx
{
// 构造
public:
	CInJectDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_INJECT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	typedef struct PROCESSINFO
	{
		DWORD dwProcID;				//进程ID
		CString cProcName;			//进程名称
		TCHAR cProcPath[MAX_PATH];	//进程路径
		DWORD dwProcPathSize;		//进程路径大小
	}PROCESSINFO,*PPROCESSINFO;

	typedef struct INJECTINFO
	{
		CString cDllPath;			//注入DLL的路径
		DWORD dwPID;				//进程的ID
		LPVOID dwBaseDll;			//Dll基址
	}INJECTINFO,*PINJECTINFO;
	INJECTINFO inject;
	vector<PROCESSINFO>m_vecProcInfo;
	CListCtrl m_List;
	CButton m_ProcessBtn;
	HANDLE hProcessSnap;	//进程快照句柄
	HANDLE hProcess;		//进程句柄
	PROCESSENTRY32 stc32;	//进程快照信息
	DWORD exitCode;			//线程退出码
	void ProcessInfo();
	void AddDLL(CString pDllPath, DWORD dwProcID);
	afx_msg void OnBnClickedButton1();
	afx_msg void OnNMRClickList2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnInject();
	afx_msg void OnUnload();
};
