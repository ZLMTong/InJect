
// InJectDlg.h : ͷ�ļ�
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


// CInJectDlg �Ի���
class CInJectDlg : public CDialogEx
{
// ����
public:
	CInJectDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_INJECT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	typedef struct PROCESSINFO
	{
		DWORD dwProcID;				//����ID
		CString cProcName;			//��������
		TCHAR cProcPath[MAX_PATH];	//����·��
		DWORD dwProcPathSize;		//����·����С
	}PROCESSINFO,*PPROCESSINFO;

	typedef struct INJECTINFO
	{
		CString cDllPath;			//ע��DLL��·��
		DWORD dwPID;				//���̵�ID
		LPVOID dwBaseDll;			//Dll��ַ
	}INJECTINFO,*PINJECTINFO;
	INJECTINFO inject;
	vector<PROCESSINFO>m_vecProcInfo;
	CListCtrl m_List;
	CButton m_ProcessBtn;
	HANDLE hProcessSnap;	//���̿��վ��
	HANDLE hProcess;		//���̾��
	PROCESSENTRY32 stc32;	//���̿�����Ϣ
	DWORD exitCode;			//�߳��˳���
	void ProcessInfo();
	void AddDLL(CString pDllPath, DWORD dwProcID);
	afx_msg void OnBnClickedButton1();
	afx_msg void OnNMRClickList2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnInject();
	afx_msg void OnUnload();
};
