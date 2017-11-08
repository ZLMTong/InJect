
// InJectDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "InJect.h"
#include "InJectDlg.h"
#include "afxdialogex.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CInJectDlg 对话框



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


// CInJectDlg 消息处理程序

BOOL CInJectDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	//设置扩展风格
	m_List.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	//添加列标题
	CRect rc;
	m_List.GetClientRect(&rc);//指定工作区大小
	int nWidth = rc.Width();
	TCHAR* pcol[] = { L"进程ID",L"进程名"};
	for (int i = 0; i < 2; i++)
	{
		m_List.InsertColumn(i, pcol[i], 0, 150);
	}

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CInJectDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CInJectDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CInJectDlg::ProcessInfo()
{
	//创建进程快照句柄
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE) return;
	//获取第一个进程信息
	stc32.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32FirstW(hProcessSnap, &stc32))
	{
		CloseHandle(hProcessSnap);
		return;
	}
	PROCESSINFO procInfo = { 0 };
	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, procInfo.dwProcID);
	CString proc;
	//遍历进程
	do
	{
		procInfo.dwProcID = stc32.th32ProcessID;
		procInfo.cProcName = stc32.szExeFile;
		procInfo.dwProcPathSize = MAX_PATH;
		//查询进程全部信息
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
	//1. 在指定的进程中申请DLL路径长度的虚拟空间PAGE_EXECUTE_READWRITE
	//DLL文件大小
	DWORD dwDllPathSize = pDllPath.GetLength() * 2 + 4;
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcID);
	//申请保存DLL的路径的虚拟内存
	LPVOID pDllBuff = VirtualAllocEx(hProcess, NULL, dwDllPathSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	
	if (pDllBuff == NULL)
	{
		return;
	}
	//2. 把dll路径写入到申请的内存中
	SIZE_T dwWriteDll = 0;		//写入的DLL路径
	WriteProcessMemory(hProcess, pDllBuff, pDllPath, dwDllPathSize, &dwWriteDll);
	if (dwWriteDll != dwDllPathSize)
	{
		MessageBox(L"注入失败!");
		return;
	}
	//3. 创建远程线程
	HANDLE hReteThread=CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibraryW, pDllBuff, NULL, NULL);
	if (hReteThread == INVALID_HANDLE_VALUE)
	{
		MessageBox(L"创建远程线程失败!");
		CloseHandle(hProcess);
	}
	else
	{
		//处理注入后的信息
		WaitForSingleObject(hReteThread, -1);
		GetExitCodeThread(hReteThread, &exitCode);		//获取线程特征码
		inject.dwBaseDll = (LPVOID)exitCode;			//保存特征码,卸载时用到
		VirtualFreeEx(hProcess, pDllBuff, NULL, MEM_RELEASE);
		CloseHandle(hReteThread);
		CloseHandle(hProcess);
	}
}

void CInJectDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	m_vecProcInfo.clear();
	m_List.DeleteAllItems();
	ProcessInfo();
}

void CInJectDlg::OnNMRClickList2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
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
	// TODO: 在此添加命令处理程序代码	
	
	DWORD dwProcSel = m_List.GetSelectionMark();		//获取当前List索引
	inject.dwPID = m_vecProcInfo[dwProcSel].dwProcID;	//当前需要注入的进程的ID
	//打开对话框,这里设置弹出的文件框中默认的后缀
	CFileDialog OpenDialog(TRUE, L"*", L".dll", OFN_OVERWRITEPROMPT, L"dll文件(*.dll)|*.dll|所有文件(*.*)|*.*||", NULL);
	if (OpenDialog.DoModal() == IDOK)
	{
		//按下OK键以后,自动获取文件路径
		inject.cDllPath = OpenDialog.GetPathName();		//获取文件路径
		AddDLL(inject.cDllPath, inject.dwPID);
	}
}


void CInJectDlg::OnUnload()
{
	// TODO: 在此添加命令处理程序代码
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, inject.dwPID);
	LPTHREAD_START_ROUTINE lpThreadAddr = (LPTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(L"kernel32"), "FreeLibrary");
	HANDLE hReteThread = CreateRemoteThread(hProcess, NULL, NULL, lpThreadAddr, inject.dwBaseDll, NULL, NULL);
	if (hReteThread == INVALID_HANDLE_VALUE)
	{
		MessageBox(L"创建远程线程失败!");
		CloseHandle(hProcess);
	}
	//处理注入后的信息
	WaitForSingleObject(hReteThread, -1);
	CloseHandle(hReteThread);
	CloseHandle(hProcess);
	MessageBox(L"卸载成功");
}
