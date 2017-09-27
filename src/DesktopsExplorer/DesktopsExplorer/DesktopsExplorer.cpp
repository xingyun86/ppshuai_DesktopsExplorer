#include <windows.h>
#include <commctrl.h>
#include <psapi.h>
#include <stdio.h>
#include <tchar.h>
#include <locale.h>
#include "resource.h"

#define MAX_LOADSTRING_LEN  MAXBYTE
#define MAX_LOADBUFFER_LEN  MAXWORD

HINSTANCE hInst;

//枚举获取托盘区域位置
bool EnumNotifyWindow(RECT &rect,HWND hWnd)
{
	//RECT rect = {0};
	bool bSuc = false;
	unsigned long lngPID = 0;
	long ret = 0,lngButtons = 0;
	long lngHwndAdr = 0,lngHwnd = 0,lngTextAdr = 0,lngButtonID = 0;
	HANDLE hProcess = NULL;
	LPVOID lngAddress = NULL;
	LPVOID lngRect = NULL;

	if (hWnd != NULL)
	{
		ret = GetWindowThreadProcessId(hWnd, &lngPID);
		if(ret != 0 && lngPID != 0)
		{
			hProcess = OpenProcess(PROCESS_ALL_ACCESS|PROCESS_VM_OPERATION|PROCESS_VM_READ|PROCESS_VM_WRITE,0,lngPID);//
			if (hProcess != NULL)
			{
				lngAddress = VirtualAllocEx(hProcess, 0, 0x4000, MEM_COMMIT, PAGE_READWRITE);
				lngRect = VirtualAllocEx(hProcess, 0, sizeof(RECT), MEM_COMMIT, PAGE_READWRITE);
				lngButtons = SendMessage(hWnd, TB_BUTTONCOUNT, 0, 0); //发送消息获取托盘button数量
				printf("%d 个图标\n", lngButtons);
				if (lngAddress != NULL  && lngRect != NULL)
				{
					for(int i=0 ;i< lngButtons;i++)
					{
						RECT rc = {0};
						int j = i;
						ret = SendMessage(hWnd, TB_GETBUTTON, j, long(lngAddress));//发送消息获取托盘项数据起始地址

						TBBUTTON tbb = {0};
                        SIZE_T stSize = 0;
                        NOTIFYICONDATA nid = {0};
                        _TCHAR tText[MAX_PATH] = {0};
                        GetProp((HWND)lngHwnd, tText);
                        printf("tText:%s\n", tText);

                        //读取TBBUTTON结构
						ReadProcessMemory(hProcess, LPVOID(lngAddress), &tbb, sizeof(tbb), &stSize);
						printf("0x%08X:0x%08X\n", lngAddress, stSize);

                        if (tbb.iString != 0xffffffff)
                        {
                            char name[255] = {0};
							ret = ReadProcessMemory(hProcess, (void *)tbb.iString, name, 255, &stSize);
                            printf("name:%s\n", name);
                        }// End if (tbb.iString != 0xffffffff)

						DWORD dwMemoryOffset = 0;
                        DWORD dwMemoryAddress = 0;
                        //读取TRAYDATA结构
                        dwMemoryOffset = (_TCHAR *)(&nid.szTip) - (_TCHAR *)(&nid);
                        ReadProcessMemory(hProcess, (LPVOID)((BYTE *)lngAddress + dwMemoryOffset), &dwMemoryAddress, sizeof(dwMemoryAddress), 0);
						ReadProcessMemory(hProcess, (LPVOID)(dwMemoryAddress), &nid.szTip, sizeof(nid.szTip) / sizeof(_TCHAR), &stSize);

                        if((nid.szTip[0]) == 'L')
                        {
                            dwMemoryOffset = (_TCHAR *)(&nid.hWnd) - (_TCHAR *)(&nid);
							ReadProcessMemory(hProcess, (LPVOID)((BYTE *)lngAddress + dwMemoryOffset), &nid.hWnd, sizeof(nid.hWnd), &stSize);
                            printf("dwMemoryOffset=%d,nid.hWnd:0x%08X\n", dwMemoryOffset, nid.hWnd);
                            dwMemoryOffset = (_TCHAR *)(&nid.uID) - (_TCHAR *)(&nid);
							ReadProcessMemory(hProcess, (LPVOID)((BYTE *)lngAddress + dwMemoryOffset), &nid.uID, sizeof(nid.uID), &stSize);

                            printf("dwMemoryOffset=%d,nid.uID:0x%08X\n", dwMemoryOffset, nid.uID);
                            ::SendMessage(hWnd, TB_HIDEBUTTON, nid.uID, (LPARAM)TRUE);
                            //::SendMessage(hWnd, TB_DELETEBUTTON, nid.uID, (LPARAM)0);

                            // Redraw tray window (to fix bug in multi-line tray area)
                            RedrawWindow(hWnd, NULL, 0, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
                            UpdateWindow(hWnd);
                            printf("Found Destination[0x%08X--0x%08X]!!!\n", nid.hWnd, nid.uID);
                        }
                        printf("dwMemoryOffset=%d,nid.szTip:%ws\n", dwMemoryOffset, (LPWSTR)nid.szTip);

                        dwMemoryOffset = (_TCHAR *)(&nid.szInfo) - (_TCHAR *)(&nid);
						ReadProcessMemory(hProcess, (LPVOID)((BYTE *)lngAddress + dwMemoryOffset), &dwMemoryAddress, sizeof(dwMemoryAddress), 0);
						ReadProcessMemory(hProcess, (LPVOID)(dwMemoryAddress), &nid.szInfo, sizeof(nid.szInfo) / sizeof(_TCHAR), &stSize);

                        printf("dwMemoryOffset=%d,nid.szInfo:%ws\n", dwMemoryOffset, nid.szInfo);
                        dwMemoryOffset = (_TCHAR *)(&nid.szInfoTitle) - (_TCHAR *)(&nid);
						ReadProcessMemory(hProcess, (LPVOID)((BYTE *)lngAddress + dwMemoryOffset), &dwMemoryAddress, sizeof(dwMemoryAddress), 0);
						ReadProcessMemory(hProcess, (LPVOID)(dwMemoryAddress), &nid.szInfoTitle, sizeof(nid.szInfoTitle) / sizeof(_TCHAR), &stSize);

                        printf("dwMemoryOffset=%d,nid.szInfoTitle:%ws\n", dwMemoryOffset, nid.szInfoTitle);

                        //先得到进程ID
                        DWORD dwWProcessID = 0;
                        GetWindowThreadProcessId( (HWND)lngHwnd, &dwWProcessID );

                        //文件路径的Buffer
                        _TCHAR tz_Filepath[MAX_PATH];

                        //由进程ID得到进程句柄
                        HANDLE hWProcess = NULL;
                        hWProcess = OpenProcess( PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, dwWProcessID);

                        //由进程句柄得到文件路径（或者用GetModuleBaseName只取得文件和后缀名）
                        GetModuleFileNameEx(
                                        hWProcess,
                                        NULL,
                                        tz_Filepath,
                                        MAX_PATH
                                        );

                        //使用文件路径......
                        printf("tz_Filepath:%s\n", tz_Filepath);
                        //关闭句柄
                        CloseHandle(hWProcess);
                        hWProcess = 0;
					}
				}
				if (lngAddress != NULL)
				{
					VirtualFreeEx( hProcess, lngAddress, 0x4096, MEM_DECOMMIT);
					VirtualFreeEx( hProcess, lngAddress, 0, MEM_RELEASE);
				}
				if (lngRect != NULL)
				{
					VirtualFreeEx( hProcess, lngRect, sizeof(RECT), MEM_DECOMMIT);
					VirtualFreeEx( hProcess, lngRect, 0, MEM_RELEASE);
				}
				CloseHandle(hProcess);
				hProcess = 0;
			}
		}
	}
	return bSuc;
}
//获取普通托盘区窗口句柄
HWND FindTrayWnd()
{
	HWND hWnd = NULL;
	HWND hWndPaper = NULL;

	if ((hWnd = FindWindow(_T("Shell_TrayWnd"), NULL)) != NULL)
	{
		if ((hWnd = FindWindowEx(hWnd, 0, _T("TrayNotifyWnd"), NULL)) != NULL)
		{
			hWndPaper = FindWindowEx(hWnd, 0, _T("SysPager"), NULL);
			if (!hWndPaper)
			{
				hWnd = FindWindowEx(hWnd, 0, _T("ToolbarWindow32"), NULL);
			}
			else
			{
				hWnd = FindWindowEx(hWndPaper, 0, _T("ToolbarWindow32"), NULL);
			}
		}
	}

	return hWnd;
}
//获取溢出托盘区窗口句柄
HWND FindNotifyIconOverflowWindow()
{
	HWND hWnd = NULL;

	hWnd = FindWindow(_T("NotifyIconOverflowWindow"), NULL);
	if (hWnd != NULL)
	{
		hWnd = FindWindowEx(hWnd, NULL, _T("ToolbarWindow32"), NULL);
	}

	return hWnd;
}

//获取托盘区域数据
RECT GetTrayRect()
{
	RECT rect = {0};
	HWND hWnd = NULL;

	hWnd = FindTrayWnd();
	if (hWnd != NULL)
	{
		if (!EnumNotifyWindow(rect,hWnd))//如果没在普通托盘区
		{
			hWnd = FindNotifyIconOverflowWindow();//在溢出区（win7）
			if (hWnd != NULL)
			{
				EnumNotifyWindow(rect,hWnd);
			}
		}
	}

	return rect;
}

int OnRefreshTreelist(HWND hWndTreeList)
{
    int nResult = 0;

    _TCHAR tzWindowTitle[MAX_LOADSTRING_LEN] = {0};
    _TCHAR tzWindowClass[MAX_LOADSTRING_LEN] = {0};
    _TCHAR tzWindowBuffer[MAX_LOADBUFFER_LEN] = {0};

    //Tree控件操作变量
    TVINSERTSTRUCT tvis = {0};
    HWND hWndParent = NULL;
    HTREEITEM hTreeItemParent = NULL;
    HWND hWndDesktop = NULL;
    HTREEITEM hTreeItemDesktop = NULL;
    HWND hWndChildren = NULL;
    HTREEITEM hTreeItemChildren = NULL;

    //设置根窗口句柄
    hWndParent = hWndTreeList;

    //根节点数据
    tvis.hParent = NULL;
    tvis.hInsertAfter = NULL;
    tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
    tvis.item.pszText = _T("窗口句柄");
    tvis.item.lParam = 0;//设置项目个数

    //删除全部节点
    TreeView_DeleteAllItems(hWndParent);

    //添加根节点
    hTreeItemParent = TreeView_InsertItem(hWndParent, &tvis);

    //获取桌面窗口句柄
    hWndDesktop = ::GetDesktopWindow();

    //取窗口标题
    ::GetWindowText(hWndDesktop, tzWindowTitle, sizeof(tzWindowTitle) / sizeof(_TCHAR));
    _sntprintf(tzWindowTitle, sizeof(tzWindowClass) / sizeof(_TCHAR), _T("桌面"));

    //取窗口类名
    ::GetClassName(hWndDesktop, tzWindowClass, sizeof(tzWindowClass) / sizeof(_TCHAR));

    //把信息格式化
	_sntprintf(tzWindowBuffer, sizeof(tzWindowBuffer) / sizeof(_TCHAR), _T("%s | %s | %d(0x%X)"), tzWindowTitle, tzWindowClass, hWndDesktop, hWndDesktop);

    //节点数据
    tvis.hParent = hTreeItemParent;
    tvis.hInsertAfter = TVI_LAST;
    tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
    tvis.item.pszText = tzWindowBuffer;
    tvis.item.lParam = (int)hWndDesktop;
    tvis.item.cChildren = ::GetWindow(hWndDesktop, GW_CHILD) ? 1 : 0;

    //插入节点
    hTreeItemDesktop = TreeView_InsertItem(hWndParent, &tvis);

    //取子级窗口句柄
    hWndChildren = ::GetWindow(hWndDesktop, GW_CHILD);

    while (hWndChildren)
    {
        //窗口是否可视
        if (::IsWindowVisible(hWndChildren))
        {
            memset(tzWindowTitle, 0, sizeof(tzWindowTitle));
            memset(tzWindowClass, 0, sizeof(tzWindowClass));
            memset(tzWindowBuffer, 0, sizeof(tzWindowBuffer));

            //取窗口标题
            ::GetWindowText(hWndChildren, tzWindowTitle, sizeof(tzWindowTitle) / sizeof(_TCHAR));

            //取窗口类名
            ::GetClassName(hWndChildren, tzWindowClass, sizeof(tzWindowClass) / sizeof(_TCHAR));

            //把信息格式化
			_sntprintf(tzWindowBuffer, sizeof(tzWindowBuffer) / sizeof(_TCHAR), _T("%s | %s | %d(0x%X)"), tzWindowTitle, tzWindowClass, hWndChildren, hWndChildren);

            //节点数据
            tvis.hParent = hTreeItemDesktop;
            tvis.hInsertAfter = TVI_LAST;
            tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
            tvis.item.pszText = tzWindowBuffer;
            tvis.item.lParam = (int)hWndChildren;
            tvis.item.cChildren = ::GetWindow(hWndChildren, GW_CHILD) ? 1 : 0;

            //插入节点
            hTreeItemChildren = TreeView_InsertItem(hWndParent, &tvis);
        }

        //取下一兄弟窗口
        hWndChildren = ::GetWindow(hWndChildren, GW_HWNDNEXT);
    }

    //展开根节点
    TreeView_Expand(hWndTreeList, hTreeItemParent, TVE_EXPAND);

    //展开桌面节点
    TreeView_Expand(hWndTreeList, hTreeItemDesktop, TVE_EXPAND);

    return nResult;
}

//树响应函数，选择项目处理
void OnSelectedChangedTreeList(NMHDR * pNMHDR, LRESULT * pResult)
{
    NMTREEVIEW * pNMTV = NULL;

    _TCHAR tzWindowTitle[MAX_LOADSTRING_LEN] = {0};
    _TCHAR tzWindowClass[MAX_LOADSTRING_LEN] = {0};
    _TCHAR tzWindowBuffer[MAX_LOADBUFFER_LEN] = {0};

    //Tree控件操作变量
    TVINSERTSTRUCT tvis = {0};
    HWND hWndTreeList = NULL;
    HWND hWndParent = NULL;
    HTREEITEM hTreeItemParent = NULL;
    HWND hWndChildren = NULL;
    HTREEITEM hTreeItemChildren = NULL;

    pNMTV = (NMTREEVIEW *)pNMHDR;
    hWndTreeList = pNMTV->hdr.hwndFrom;
    //::GetClassName(hWndTreeList, tzWindowClass, sizeof(tzWindowClass) / sizeof(_TCHAR));
    //_tprintf(_T("%s\n"), tzWindowClass);

    //hTreeItemParent = pNMTV->itemNew.hItem;
    hTreeItemParent = TreeView_GetSelection(hWndTreeList);

    tvis.item.hItem = hTreeItemParent;
    tvis.item.mask = TVIF_PARAM;
    TreeView_GetItem(hWndTreeList, &tvis.item);
    hWndParent = (HWND)tvis.item.lParam;

    //ItemHasChildren     是否有子节点
    //GetChildItem        获取第一个子结点
    //GetNextSiblingItem  获取下一个兄弟结点结点

    //判断是否有子节点，已有子节点则不处理
    if (TreeView_GetChild(hWndTreeList, hTreeItemParent))
    {
        return;
    }

    //遍历子窗口的所有控件
    //取子级窗口句柄
    hWndChildren = ::GetWindow(hWndParent, GW_CHILD);

    while (hWndChildren)
    {
        //窗口是否可视
        //if (::IsWindowVisible(hWndChildren))
        {
            memset(tzWindowTitle, 0, sizeof(tzWindowTitle));
            memset(tzWindowClass, 0, sizeof(tzWindowClass));
            memset(tzWindowBuffer, 0, sizeof(tzWindowBuffer));

            //取窗口标题
            ::GetWindowText(hWndChildren, tzWindowTitle, sizeof(tzWindowTitle) / sizeof(_TCHAR));

            //取窗口类名
            ::GetClassName(hWndChildren, tzWindowClass, sizeof(tzWindowClass) / sizeof(_TCHAR));

            //把信息格式化
			_sntprintf(tzWindowBuffer, sizeof(tzWindowBuffer) / sizeof(_TCHAR), _T("%s | %s | %d(0x%X)"), tzWindowTitle, tzWindowClass, hWndChildren, hWndChildren);

            //_tprintf(_T("%s\n"), tzWindowBuffer);

            //节点数据
            tvis.hParent = hTreeItemParent;
            tvis.hInsertAfter = TVI_LAST;
            tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
            tvis.item.pszText = tzWindowBuffer;
            tvis.item.lParam = (int)hWndChildren;
            tvis.item.cChildren = ::GetWindow(hWndChildren, GW_CHILD) ? 1 : 0;

            //插入节点
            hTreeItemChildren = TreeView_InsertItem(hWndTreeList, &tvis);
        }

        //取下一兄弟窗口
        hWndChildren = ::GetWindow(hWndChildren, GW_HWNDNEXT);
    }
}
#include <mshtml.h>
#include <oleacc.h>
#include <exdisp.h>
#include <servprov.h>
#include <shlguid.h>

#if !defined(SID_SWebBrowserApp)
//EXTERN_C const IID IID_IWebBrowserApp;
#define SID_SWebBrowserApp IID_IWebBrowserApp
#endif
//typedef STDMETHODIMP(*LPFNOBJECTFROMLRESULT)(LRESULT, REFIID, WPARAM, void**);

IWebBrowser2 * GetIEFromHWnd()
{
    HWND hWnd ;
    // 找类名为IEFrame的窗体
    hWnd= FindWindow(_T("IEFrame"), NULL);
    // 如果找不到
    if(hWnd==NULL || hWnd ==0 )
    {
        // 则找一个类名为CabinetWClass的窗体
        hWnd= FindWindow(_T("CabinetWClass"), NULL);
    }
    // 如果没有找到类名为IEFrame或者CabinetWClass的窗体
    if( hWnd == NULL || hWnd ==0 )
    {
        // MessageBox (NULL,L"No Running instance of Internet Explorer!",L"message", MB_OK);
        // 返回NULL
        return NULL;
    }
    // 然后在hWnd(也就是刚才找到的类名为IEFrame或CabinetWClass的窗体)的子窗体中类名为Shell DocObject View的窗体
    // IE6可以直接找到，因为IE6没TAB标签，不过没有测试过
    HWND hWndChild = FindWindowEx(hWnd, 0, _T("Shell DocObject View"), NULL);
    // 如果没有找到，说明是IE6以上
    if(hWndChild ==0)
    {
        // 就在hWnd的子体中找类名为Frame Tab的窗体
        hWndChild = FindWindowEx(hWnd, 0, _T("Frame Tab"), NULL);
        if(hWndChild ==0)
        {
            return NULL;
        }
        // 然后继续在类名为Frame Tab的窗体的子窗体中找类名为TabWindowClass的窗体
        hWndChild = FindWindowEx(hWndChild, 0, _T("TabWindowClass"), NULL);
        if(hWndChild ==0)
        {
            return NULL;
        }
        // 然后继续在类名为TabWindowClass的窗体的子窗体中找类名为Shell DocObject View的窗体
        hWndChild = FindWindowEx(hWndChild, 0, _T("Shell DocObject View"), NULL);
        if(hWndChild ==0)
        {
            return NULL;
        }
    }

    // 在类名为Shell DocObject View的窗体的子窗体中找类名为Internet Explorer_Server的窗体
    // 这个就是WebBrowser 控件的窗体了
    hWndChild = FindWindowEx(hWndChild, 0, _T("Internet Explorer_Server"), NULL);
    if(hWndChild==0)
    {
        return NULL;
    }
    // 将WebBrowser控件的窗体句柄赋给hWnd
    hWnd=hWndChild;

    // 我们需要显示地装载OLEACC.DLL,这样我们才知道有没有安装MSAA(Microsoft Active Accessibility)
    // 因为要跨进程的操作，所以是必须的 , 他的 ObjectFromLresult 是获得 接口的关键
    HINSTANCE hModuleInstance = LoadLibrary( _T("OLEACC.DLL") );

    IWebBrowser2* pWebBrowser2=NULL;

    // 如果hInst不为NULL，也就是支持MSAA 。
    if ( hModuleInstance != NULL )
    {
        // 如果找到了运行中的IE的WebBrowser控件的窗体句柄
        if ( hWnd != NULL )
        {
            LRESULT lRes;
            // 注册一个WM_HTML_GETOBJECT的消息
            UINT nMsg = ::RegisterWindowMessage( _T("WM_HTML_GETOBJECT") );
            // 象WebBrowser控件的窗体发送 WM_HTML_GETOBJECT 并将返回结果，存放在lRes变量里
            ::SendMessageTimeout( hWnd, nMsg, 0L, 0L, SMTO_ABORTIFHUNG, 1000, (PDWORD_PTR)&lRes);

            // 得到OLEACC.DLL 里 ObjectFromLresult  方法的 地址
            LPFNOBJECTFROMLRESULT pfObjectFromLresult = (LPFNOBJECTFROMLRESULT)::GetProcAddress( hInst, (LPCSTR)"ObjectFromLresult" );
            // 如果找到了 ObjectFromLresult 方法
            if ( pfObjectFromLresult != NULL )
            {
                HRESULT hr;
                // 声明一个IHTMLDocument2*的实例.
                IHTMLDocument2 *spDoc;
                // 利用ObjectFromLresult的通过 WM_HTML_GETOBJECT 的返回值，得到WebBrowser的IHTMLDocument2
                hr = pfObjectFromLresult(lRes,IID_IHTMLDocument2,0,(void**)&spDoc);
                // 如果执行成功
                if ( SUCCEEDED(hr) ){
                    // 声明一个IHTMLWindow2*和一个IServiceProvider*实例 。
                    IHTMLWindow2 * spWnd2;
                    IServiceProvider * spServiceProv;
                    // 通过 IHTMLDocument2 的 get_parentWindow 得到 IHTMLWindow2 接口
                    hr=spDoc->get_parentWindow ((IHTMLWindow2**)&spWnd2);
                    // 如果成功
                    if(SUCCEEDED(hr))
                    {
                        // 通过IHTMLWindow2的QueryInterface方法，获取IServiceProvider    接口
                        hr=spWnd2->QueryInterface(IID_IServiceProvider,(void**)&spServiceProv) ;
                        // 如果成功
                        if(SUCCEEDED(hr))
                        {
                            // 利用 IServiceProvider 接口的 QueryService 方法 获取 IWebBrowser2  接口，至此这个接口就算找到了
                            hr = spServiceProv->QueryService(
                                SID_SWebBrowserApp,
                                IID_IWebBrowser2,
                                (void**)&pWebBrowser2
                            );
                        }
                    }
                }
            }
        }
        ::FreeLibrary(hModuleInstance);
        hModuleInstance = NULL;
    }
    else{
        //如果没有安装MSAA
        // MessageBox(NULL,_T("Please Install Microsoft Active Accessibility"),"Error",MB_OK);
    }
    return pWebBrowser2;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//函数功能：设置圆角窗口
//函数参数：
//	hWnd		要设置的窗口句柄
//	pstEllipse	要设置圆角的横向半径和纵向半径
//	prcExcepted	要排除圆角的左上右下侧大小
//返回值：无返回
void SetWindowEllispeFrame(HWND hWnd, SIZE * pszEllipse = 0, RECT * prcExcepted = 0)
{
	HRGN hRgnWindow = 0;
	POINT ptPosition = { 0, 0 };
	RECT rcWindow = { 0, 0, 0, 0 };

	GetWindowRect(hWnd, &rcWindow);
	if (prcExcepted)
	{
		ptPosition.x = prcExcepted->left;
		ptPosition.y = prcExcepted->top;
		rcWindow.left += prcExcepted->left;
		rcWindow.top += prcExcepted->top;
		rcWindow.right -= prcExcepted->right;
		rcWindow.bottom -= prcExcepted->bottom;
	}

	hRgnWindow = CreateRoundRectRgn(ptPosition.x, ptPosition.y, 
		rcWindow.right, rcWindow.bottom, pszEllipse->cx, pszEllipse->cy);
	if (hRgnWindow)
	{
		SetWindowRgn(hWnd, hRgnWindow, TRUE);
	}
}
HBRUSH Global_hBrushBackground = 0;
BOOL CALLBACK DlgMain(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
    {
        LONG lStyle = 0;
        HWND hTreeListWnd = GetDlgItem(hwndDlg, IDC_TREE_LIST);
        HWND hRefreshWnd = GetDlgItem(hwndDlg, IDC_BUTTON_REFRESH);

        //设置树视图样式风格
        lStyle = GetWindowLong(hTreeListWnd, GWL_STYLE);
        lStyle &= ~TVS_NOSCROLL;
        lStyle &= ~TVS_NOTOOLTIPS;
        lStyle |= TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_INFOTIP;
        SetWindowLong(hTreeListWnd, GWL_STYLE, lStyle);

        //设置刷新按钮重绘
        lStyle = GetWindowLong(hRefreshWnd, GWL_STYLE);
        //lStyle |= BS_OWNERDRAW;
        SetWindowLong(hRefreshWnd, GWL_STYLE, lStyle);

        OnRefreshTreelist(GetDlgItem(hwndDlg, IDC_TREE_LIST));

        //初始化背景颜色
        COLORREF clrBk = RGB(20, 210, 18);
		Global_hBrushBackground = CreateSolidBrush(clrBk);

		SIZE szEllispe = { 4, 4 };
		RECT rcExcepted = { 3, 25, 3, 3 };
		SetWindowEllispeFrame(hwndDlg, &szEllispe, &rcExcepted);

		//设置树视图背景颜色
		TreeView_SetBkColor(GetDlgItem(hwndDlg, IDC_TREE_LIST), clrBk);
    }
    return TRUE;
    case WM_LBUTTONDOWN:
    {
        #ifndef SC_DRAGMOVE
        #define SC_DRAGMOVE 0xF012
        SendMessage(hwndDlg, WM_SYSCOMMAND, SC_DRAGMOVE, 0);
        #undef SC_DRAGMOVE
        #endif // SC_DRAGMOVE
    }
    return TRUE;
    case WM_PAINT:
        {
            PAINTSTRUCT ps = { 0 };
            HDC hDC = BeginPaint(hwndDlg, &ps);

            SetBkMode(hDC, TRANSPARENT);

            RECT rcWnd = { 0 };
            GetClientRect(hwndDlg, &rcWnd);
            //HBRUSH hBrush = CreateSolidBrush(RGB(0, 255, 0));
            FillRect(hDC, &rcWnd, Global_hBrushBackground);//设置为绿色背景
            //DeleteObject(hBrush);
            //hBrush = 0;

            EndPaint(hwndDlg, &ps);
        }
        break;
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORMSGBOX:
    case WM_CTLCOLORSCROLLBAR:
    case WM_CTLCOLORSTATIC:
        {
            HDC hDC = (HDC)(wParam);
            HWND hWnd = (HWND)(lParam);
            //   改变文本颜色和背景颜色
            switch (GetDlgCtrlID(hWnd))
            {
            case IDC_BUTTON_REFRESH:
            {
                SetTextColor(hDC, RGB(0x00, 0x00, 0xFF));
                _TCHAR tButtonText[MAX_PATH] = {0};
                GetWindowText(hWnd, tButtonText, MAX_PATH);
                RECT rcButton = {0};
                GetClientRect(hWnd, &rcButton);
                DrawText(hDC, tButtonText, _tcslen(tButtonText), &rcButton, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
            break;
            default:
            {
                SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
            }
            break;
            }

            //设置透明背景模式
            SetBkMode(hDC, TRANSPARENT);
            //SetBkColor(hDC, GetSysColor(COLOR_BTNFACE));
            //SetBkColor(hDC, RGB(255, 0, 0));

            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LONG)TRUE);
            return (LRESULT)Global_hBrushBackground;// GetSysColorBrush(COLOR_BTNFACE);
        }
        break;
    case WM_CLOSE:
    {
        EndDialog(hwndDlg, 0);
    }
    return TRUE;

    case WM_COMMAND:
    {
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON_REFRESH:
		{
			OnRefreshTreelist(GetDlgItem(hwndDlg, IDC_TREE_LIST));
		}
		break;
		case IDCANCEL:
		case IDOK:
		{
			EndDialog(hwndDlg, LOWORD(wParam));
		}
		break;
		default:
		{

		}
		break;
		}
    }
    return TRUE;
    case WM_NOTIFY:
    {
        NMHDR * pNMHDR = (NMHDR *)lParam;
        switch(pNMHDR->code)
        {
        case NM_DBLCLK://双击操作
            {
                switch(pNMHDR->idFrom)
                {
                    case IDC_TREE_LIST:
                    {
                        OnSelectedChangedTreeList((NMHDR *)lParam, 0);
                    }
                    break;
                }
            }
            break;
        }
    }
    return TRUE;
    }
    return FALSE;
}


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    int nResult = 0;

    _tsetlocale(LC_ALL, _T("chs"));

    GetTrayRect();
	
    hInst = hInstance;
    InitCommonControls();
	nResult = DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_MAIN), NULL, (DLGPROC)DlgMain);

    return nResult;
}
