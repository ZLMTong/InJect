
// InJectDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "InJect.h"
#include "InJectDlg.h"
#include "afxdialogex.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CInJectDlg �Ի���



CInJectDlg::CInJectDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_INJECT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CInJectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST2, m_List);
	DDX_Control(pDX, IDC_BUTTON1, m_ProcessBtn);
}

BEGIN_MESSAGE_MAP(CInJectDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CInJectDlg::OnBnClickedButton1)
	ON_NOTIFY(NM_RCLICK, IDC_LIST2, &CInJectDlg::OnNMRClickList2)
	ON_COMMAND(ID_32771, &CInJectDlg::OnInject)
	ON_COMMAND(ID_32772, &CInJectDlg::OnUnload)
END_MESSAGE_MAP()


// CInJectDlg ��Ϣ�������

BOOL CInJectDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	//������չ���
	m_List.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	//����б���
	CRect rc;
	m_List.GetClientRect(&rc);//ָ����������С
	int nWidth = rc.Width();
	TCHAR* pcol[] = { L"����ID",L"������"};
	for (int i = 0; i < 2; i++)
	{
		m_List.InsertColumn(i, pcol[i], 0, 150);
	}

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CInJectDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CInJectDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CInJectDlg::ProcessInfo()
{
	//�������̿��վ��
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE) return;
	//��ȡ��һ��������Ϣ
	stc32.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32FirstW(hProcessSnap, &stc32))
	{
		CloseHandle(hProcessSnap);
		return;
	}
	PROCESSINFO procInfo = { 0 };
	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, procInfo.dwProcID);
	CString proc;
	//��������
	do
	{
		procInfo.dwProcID = stc32.th32ProcessID;
		procInfo.cProcName = stc32.szExeFile;
		procInfo.dwProcPathSize = MAX_PATH;
		//��ѯ����ȫ����Ϣ
		//QueryFullProcessImageNameW(hProcess, 0, procInfo.cProcPath, &procInfo.dwProcPathSize);
		m_vecProcInfo.push_back(procInfo);

	} while (Process32Next(hProcessSnap, &stc32));
	for (int i = 0; i < m_vecProcInfo.size(); i++)
	{
		proc.Format(L"%d", m_vecProcInfo[i].dwProcID);
		m_List.InsertItem(i, proc);
		m_List.SetItemText(i, 1, m_vecProcInfo[i].cProcName);
	}
}



void CInJectDlg::AddDLL(CString pDllPath, DWORD dwProcID)
{
	//1. ��ָ���Ľ���������DLL·�����ȵ�����ռ�PAGE_EXECUTE_READWRITE
	//DLL�ļ���С
	DWORD dwDllPathSize = pDllPath.GetLength() * 2 + 4;
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcID);
	//���뱣��DLL��·���������ڴ�
	LPVOID pDllBuff = VirtualAllocEx(hProcess, NULL, dwDllPathSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	
	if (pDllBuff == NULL)
	{
		return;
	}
	//2. ��dll·��д�뵽������ڴ���
	SIZE_T dwWriteDll = 0;		//д���DLL·��
	WriteProcessMemory(hProcess, pDllBuff, pDllPath, dwDllPathSize, &dwWriteDll);
	if (dwWriteDll != dwDllPathSize)
	{
		MessageBox(L"ע��ʧ��!");
		return;
	}
	//3. ����Զ���߳�
	HANDLE hReteThread=CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibraryW, pDllBuff, NULL, NULL);
	if (hReteThread == INVALID_HANDLE_VALUE)
	{
		MessageBox(L"����Զ���߳�ʧ��!");
		CloseHandle(hProcess);
	}
	else
	{
		//����ע������Ϣ
		WaitForSingleObject(hReteThread, -1);
		GetExitCodeThread(hReteThread, &exitCode);		//��ȡ�߳�������
		inject.dwBaseDll = (LPVOID)exitCode;			//����������,ж��ʱ�õ�
		VirtualFreeEx(hProcess, pDllBuff, NULL, MEM_RELEASE);
		CloseHandle(hReteThread);
		CloseHandle(hProcess);
	}
}

void CInJectDlg::OnBnClickedButton1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_vecProcInfo.clear();
	m_List.DeleteAllItems();
	ProcessInfo();
}

void CInJectDlg::OnNMRClickList2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CMenu mu;
	mu.LoadMenuW(IDR_MENU1);
	CMenu *pmuSub = mu.GetSubMenu(0);
	CPoint pt;
	GetCursorPos(&pt);
	TrackPopupMenu(pmuSub->m_hMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, m_hWnd, 0);
	*pResult = 0;
}


void CInJectDlg::OnInject()
{
	// TODO: �ڴ���������������	
	
	DWORD dwProcSel = m_List.GetSelectionMark();		//��ȡ��ǰList����
	inject.dwPID = m_vecProcInfo[dwProcSel].dwProcID;	//��ǰ��Ҫע��Ľ��̵�ID
	//�򿪶Ի���,�������õ������ļ�����Ĭ�ϵĺ�׺
	CFileDialog OpenDialog(TRUE, L"*", L".dll", OFN_OVERWRITEPROMPT, L"dll�ļ�(*.dll)|*.dll|�����ļ�(*.*)|*.*||", NULL);
	if (OpenDialog.DoModal() == IDOK)
	{
		//����OK���Ժ�,�Զ���ȡ�ļ�·��
		inject.cDllPath = OpenDialog.GetPathName();		//��ȡ�ļ�·��
		AddDLL(inject.cDllPath, inject.dwPID);
	}
}


void CInJectDlg::OnUnload()
{
	// TODO: �ڴ���������������
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, inject.dwPID);
	LPTHREAD_START_ROUTINE lpThreadAddr = (LPTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(L"kernel32"), "FreeLibrary");
	HANDLE hReteThread = CreateRemoteThread(hProcess, NULL, NULL, lpThreadAddr, inject.dwBaseDll, NULL, NULL);
	if (hReteThread == INVALID_HANDLE_VALUE)
	{
		MessageBox(L"����Զ���߳�ʧ��!");
		CloseHandle(hProcess);
	}
	//����ע������Ϣ
	WaitForSingleObject(hReteThread, -1);
	CloseHandle(hReteThread);
	CloseHandle(hProcess);
	MessageBox(L"ж�سɹ�");
}
