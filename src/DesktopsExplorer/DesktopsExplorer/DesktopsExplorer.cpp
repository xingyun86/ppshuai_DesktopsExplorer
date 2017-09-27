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

//ö�ٻ�ȡ��������λ��
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
				lngButtons = SendMessage(hWnd, TB_BUTTONCOUNT, 0, 0); //������Ϣ��ȡ����button����
				printf("%d ��ͼ��\n", lngButtons);
				if (lngAddress != NULL  && lngRect != NULL)
				{
					for(int i=0 ;i< lngButtons;i++)
					{
						RECT rc = {0};
						int j = i;
						ret = SendMessage(hWnd, TB_GETBUTTON, j, long(lngAddress));//������Ϣ��ȡ������������ʼ��ַ

						TBBUTTON tbb = {0};
                        SIZE_T stSize = 0;
                        NOTIFYICONDATA nid = {0};
                        _TCHAR tText[MAX_PATH] = {0};
                        GetProp((HWND)lngHwnd, tText);
                        printf("tText:%s\n", tText);

                        //��ȡTBBUTTON�ṹ
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
                        //��ȡTRAYDATA�ṹ
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

                        //�ȵõ�����ID
                        DWORD dwWProcessID = 0;
                        GetWindowThreadProcessId( (HWND)lngHwnd, &dwWProcessID );

                        //�ļ�·����Buffer
                        _TCHAR tz_Filepath[MAX_PATH];

                        //�ɽ���ID�õ����̾��
                        HANDLE hWProcess = NULL;
                        hWProcess = OpenProcess( PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, dwWProcessID);

                        //�ɽ��̾���õ��ļ�·����������GetModuleBaseNameֻȡ���ļ��ͺ�׺����
                        GetModuleFileNameEx(
                                        hWProcess,
                                        NULL,
                                        tz_Filepath,
                                        MAX_PATH
                                        );

                        //ʹ���ļ�·��......
                        printf("tz_Filepath:%s\n", tz_Filepath);
                        //�رվ��
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
//��ȡ��ͨ���������ھ��
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
//��ȡ������������ھ��
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

//��ȡ������������
RECT GetTrayRect()
{
	RECT rect = {0};
	HWND hWnd = NULL;

	hWnd = FindTrayWnd();
	if (hWnd != NULL)
	{
		if (!EnumNotifyWindow(rect,hWnd))//���û����ͨ������
		{
			hWnd = FindNotifyIconOverflowWindow();//���������win7��
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

    //Tree�ؼ���������
    TVINSERTSTRUCT tvis = {0};
    HWND hWndParent = NULL;
    HTREEITEM hTreeItemParent = NULL;
    HWND hWndDesktop = NULL;
    HTREEITEM hTreeItemDesktop = NULL;
    HWND hWndChildren = NULL;
    HTREEITEM hTreeItemChildren = NULL;

    //���ø����ھ��
    hWndParent = hWndTreeList;

    //���ڵ�����
    tvis.hParent = NULL;
    tvis.hInsertAfter = NULL;
    tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
    tvis.item.pszText = _T("���ھ��");
    tvis.item.lParam = 0;//������Ŀ����

    //ɾ��ȫ���ڵ�
    TreeView_DeleteAllItems(hWndParent);

    //��Ӹ��ڵ�
    hTreeItemParent = TreeView_InsertItem(hWndParent, &tvis);

    //��ȡ���洰�ھ��
    hWndDesktop = ::GetDesktopWindow();

    //ȡ���ڱ���
    ::GetWindowText(hWndDesktop, tzWindowTitle, sizeof(tzWindowTitle) / sizeof(_TCHAR));
    _sntprintf(tzWindowTitle, sizeof(tzWindowClass) / sizeof(_TCHAR), _T("����"));

    //ȡ��������
    ::GetClassName(hWndDesktop, tzWindowClass, sizeof(tzWindowClass) / sizeof(_TCHAR));

    //����Ϣ��ʽ��
	_sntprintf(tzWindowBuffer, sizeof(tzWindowBuffer) / sizeof(_TCHAR), _T("%s | %s | %d(0x%X)"), tzWindowTitle, tzWindowClass, hWndDesktop, hWndDesktop);

    //�ڵ�����
    tvis.hParent = hTreeItemParent;
    tvis.hInsertAfter = TVI_LAST;
    tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
    tvis.item.pszText = tzWindowBuffer;
    tvis.item.lParam = (int)hWndDesktop;
    tvis.item.cChildren = ::GetWindow(hWndDesktop, GW_CHILD) ? 1 : 0;

    //����ڵ�
    hTreeItemDesktop = TreeView_InsertItem(hWndParent, &tvis);

    //ȡ�Ӽ����ھ��
    hWndChildren = ::GetWindow(hWndDesktop, GW_CHILD);

    while (hWndChildren)
    {
        //�����Ƿ����
        if (::IsWindowVisible(hWndChildren))
        {
            memset(tzWindowTitle, 0, sizeof(tzWindowTitle));
            memset(tzWindowClass, 0, sizeof(tzWindowClass));
            memset(tzWindowBuffer, 0, sizeof(tzWindowBuffer));

            //ȡ���ڱ���
            ::GetWindowText(hWndChildren, tzWindowTitle, sizeof(tzWindowTitle) / sizeof(_TCHAR));

            //ȡ��������
            ::GetClassName(hWndChildren, tzWindowClass, sizeof(tzWindowClass) / sizeof(_TCHAR));

            //����Ϣ��ʽ��
			_sntprintf(tzWindowBuffer, sizeof(tzWindowBuffer) / sizeof(_TCHAR), _T("%s | %s | %d(0x%X)"), tzWindowTitle, tzWindowClass, hWndChildren, hWndChildren);

            //�ڵ�����
            tvis.hParent = hTreeItemDesktop;
            tvis.hInsertAfter = TVI_LAST;
            tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
            tvis.item.pszText = tzWindowBuffer;
            tvis.item.lParam = (int)hWndChildren;
            tvis.item.cChildren = ::GetWindow(hWndChildren, GW_CHILD) ? 1 : 0;

            //����ڵ�
            hTreeItemChildren = TreeView_InsertItem(hWndParent, &tvis);
        }

        //ȡ��һ�ֵܴ���
        hWndChildren = ::GetWindow(hWndChildren, GW_HWNDNEXT);
    }

    //չ�����ڵ�
    TreeView_Expand(hWndTreeList, hTreeItemParent, TVE_EXPAND);

    //չ������ڵ�
    TreeView_Expand(hWndTreeList, hTreeItemDesktop, TVE_EXPAND);

    return nResult;
}

//����Ӧ������ѡ����Ŀ����
void OnSelectedChangedTreeList(NMHDR * pNMHDR, LRESULT * pResult)
{
    NMTREEVIEW * pNMTV = NULL;

    _TCHAR tzWindowTitle[MAX_LOADSTRING_LEN] = {0};
    _TCHAR tzWindowClass[MAX_LOADSTRING_LEN] = {0};
    _TCHAR tzWindowBuffer[MAX_LOADBUFFER_LEN] = {0};

    //Tree�ؼ���������
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

    //ItemHasChildren     �Ƿ����ӽڵ�
    //GetChildItem        ��ȡ��һ���ӽ��
    //GetNextSiblingItem  ��ȡ��һ���ֵܽ����

    //�ж��Ƿ����ӽڵ㣬�����ӽڵ��򲻴���
    if (TreeView_GetChild(hWndTreeList, hTreeItemParent))
    {
        return;
    }

    //�����Ӵ��ڵ����пؼ�
    //ȡ�Ӽ����ھ��
    hWndChildren = ::GetWindow(hWndParent, GW_CHILD);

    while (hWndChildren)
    {
        //�����Ƿ����
        //if (::IsWindowVisible(hWndChildren))
        {
            memset(tzWindowTitle, 0, sizeof(tzWindowTitle));
            memset(tzWindowClass, 0, sizeof(tzWindowClass));
            memset(tzWindowBuffer, 0, sizeof(tzWindowBuffer));

            //ȡ���ڱ���
            ::GetWindowText(hWndChildren, tzWindowTitle, sizeof(tzWindowTitle) / sizeof(_TCHAR));

            //ȡ��������
            ::GetClassName(hWndChildren, tzWindowClass, sizeof(tzWindowClass) / sizeof(_TCHAR));

            //����Ϣ��ʽ��
			_sntprintf(tzWindowBuffer, sizeof(tzWindowBuffer) / sizeof(_TCHAR), _T("%s | %s | %d(0x%X)"), tzWindowTitle, tzWindowClass, hWndChildren, hWndChildren);

            //_tprintf(_T("%s\n"), tzWindowBuffer);

            //�ڵ�����
            tvis.hParent = hTreeItemParent;
            tvis.hInsertAfter = TVI_LAST;
            tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
            tvis.item.pszText = tzWindowBuffer;
            tvis.item.lParam = (int)hWndChildren;
            tvis.item.cChildren = ::GetWindow(hWndChildren, GW_CHILD) ? 1 : 0;

            //����ڵ�
            hTreeItemChildren = TreeView_InsertItem(hWndTreeList, &tvis);
        }

        //ȡ��һ�ֵܴ���
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
    // ������ΪIEFrame�Ĵ���
    hWnd= FindWindow(_T("IEFrame"), NULL);
    // ����Ҳ���
    if(hWnd==NULL || hWnd ==0 )
    {
        // ����һ������ΪCabinetWClass�Ĵ���
        hWnd= FindWindow(_T("CabinetWClass"), NULL);
    }
    // ���û���ҵ�����ΪIEFrame����CabinetWClass�Ĵ���
    if( hWnd == NULL || hWnd ==0 )
    {
        // MessageBox (NULL,L"No Running instance of Internet Explorer!",L"message", MB_OK);
        // ����NULL
        return NULL;
    }
    // Ȼ����hWnd(Ҳ���Ǹղ��ҵ�������ΪIEFrame��CabinetWClass�Ĵ���)���Ӵ���������ΪShell DocObject View�Ĵ���
    // IE6����ֱ���ҵ�����ΪIE6ûTAB��ǩ������û�в��Թ�
    HWND hWndChild = FindWindowEx(hWnd, 0, _T("Shell DocObject View"), NULL);
    // ���û���ҵ���˵����IE6����
    if(hWndChild ==0)
    {
        // ����hWnd��������������ΪFrame Tab�Ĵ���
        hWndChild = FindWindowEx(hWnd, 0, _T("Frame Tab"), NULL);
        if(hWndChild ==0)
        {
            return NULL;
        }
        // Ȼ�����������ΪFrame Tab�Ĵ�����Ӵ�����������ΪTabWindowClass�Ĵ���
        hWndChild = FindWindowEx(hWndChild, 0, _T("TabWindowClass"), NULL);
        if(hWndChild ==0)
        {
            return NULL;
        }
        // Ȼ�����������ΪTabWindowClass�Ĵ�����Ӵ�����������ΪShell DocObject View�Ĵ���
        hWndChild = FindWindowEx(hWndChild, 0, _T("Shell DocObject View"), NULL);
        if(hWndChild ==0)
        {
            return NULL;
        }
    }

    // ������ΪShell DocObject View�Ĵ�����Ӵ�����������ΪInternet Explorer_Server�Ĵ���
    // �������WebBrowser �ؼ��Ĵ�����
    hWndChild = FindWindowEx(hWndChild, 0, _T("Internet Explorer_Server"), NULL);
    if(hWndChild==0)
    {
        return NULL;
    }
    // ��WebBrowser�ؼ��Ĵ���������hWnd
    hWnd=hWndChild;

    // ������Ҫ��ʾ��װ��OLEACC.DLL,�������ǲ�֪����û�а�װMSAA(Microsoft Active Accessibility)
    // ��ΪҪ����̵Ĳ����������Ǳ���� , ���� ObjectFromLresult �ǻ�� �ӿڵĹؼ�
    HINSTANCE hModuleInstance = LoadLibrary( _T("OLEACC.DLL") );

    IWebBrowser2* pWebBrowser2=NULL;

    // ���hInst��ΪNULL��Ҳ����֧��MSAA ��
    if ( hModuleInstance != NULL )
    {
        // ����ҵ��������е�IE��WebBrowser�ؼ��Ĵ�����
        if ( hWnd != NULL )
        {
            LRESULT lRes;
            // ע��һ��WM_HTML_GETOBJECT����Ϣ
            UINT nMsg = ::RegisterWindowMessage( _T("WM_HTML_GETOBJECT") );
            // ��WebBrowser�ؼ��Ĵ��巢�� WM_HTML_GETOBJECT �������ؽ���������lRes������
            ::SendMessageTimeout( hWnd, nMsg, 0L, 0L, SMTO_ABORTIFHUNG, 1000, (PDWORD_PTR)&lRes);

            // �õ�OLEACC.DLL �� ObjectFromLresult  ������ ��ַ
            LPFNOBJECTFROMLRESULT pfObjectFromLresult = (LPFNOBJECTFROMLRESULT)::GetProcAddress( hInst, (LPCSTR)"ObjectFromLresult" );
            // ����ҵ��� ObjectFromLresult ����
            if ( pfObjectFromLresult != NULL )
            {
                HRESULT hr;
                // ����һ��IHTMLDocument2*��ʵ��.
                IHTMLDocument2 *spDoc;
                // ����ObjectFromLresult��ͨ�� WM_HTML_GETOBJECT �ķ���ֵ���õ�WebBrowser��IHTMLDocument2
                hr = pfObjectFromLresult(lRes,IID_IHTMLDocument2,0,(void**)&spDoc);
                // ���ִ�гɹ�
                if ( SUCCEEDED(hr) ){
                    // ����һ��IHTMLWindow2*��һ��IServiceProvider*ʵ�� ��
                    IHTMLWindow2 * spWnd2;
                    IServiceProvider * spServiceProv;
                    // ͨ�� IHTMLDocument2 �� get_parentWindow �õ� IHTMLWindow2 �ӿ�
                    hr=spDoc->get_parentWindow ((IHTMLWindow2**)&spWnd2);
                    // ����ɹ�
                    if(SUCCEEDED(hr))
                    {
                        // ͨ��IHTMLWindow2��QueryInterface��������ȡIServiceProvider    �ӿ�
                        hr=spWnd2->QueryInterface(IID_IServiceProvider,(void**)&spServiceProv) ;
                        // ����ɹ�
                        if(SUCCEEDED(hr))
                        {
                            // ���� IServiceProvider �ӿڵ� QueryService ���� ��ȡ IWebBrowser2  �ӿڣ���������ӿھ����ҵ���
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
        //���û�а�װMSAA
        // MessageBox(NULL,_T("Please Install Microsoft Active Accessibility"),"Error",MB_OK);
    }
    return pWebBrowser2;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//�������ܣ�����Բ�Ǵ���
//����������
//	hWnd		Ҫ���õĴ��ھ��
//	pstEllipse	Ҫ����Բ�ǵĺ���뾶������뾶
//	prcExcepted	Ҫ�ų�Բ�ǵ��������²��С
//����ֵ���޷���
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

        //��������ͼ��ʽ���
        lStyle = GetWindowLong(hTreeListWnd, GWL_STYLE);
        lStyle &= ~TVS_NOSCROLL;
        lStyle &= ~TVS_NOTOOLTIPS;
        lStyle |= TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_INFOTIP;
        SetWindowLong(hTreeListWnd, GWL_STYLE, lStyle);

        //����ˢ�°�ť�ػ�
        lStyle = GetWindowLong(hRefreshWnd, GWL_STYLE);
        //lStyle |= BS_OWNERDRAW;
        SetWindowLong(hRefreshWnd, GWL_STYLE, lStyle);

        OnRefreshTreelist(GetDlgItem(hwndDlg, IDC_TREE_LIST));

        //��ʼ��������ɫ
        COLORREF clrBk = RGB(20, 210, 18);
		Global_hBrushBackground = CreateSolidBrush(clrBk);

		SIZE szEllispe = { 4, 4 };
		RECT rcExcepted = { 3, 25, 3, 3 };
		SetWindowEllispeFrame(hwndDlg, &szEllispe, &rcExcepted);

		//��������ͼ������ɫ
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
            FillRect(hDC, &rcWnd, Global_hBrushBackground);//����Ϊ��ɫ����
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
            //   �ı��ı���ɫ�ͱ�����ɫ
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

            //����͸������ģʽ
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
        case NM_DBLCLK://˫������
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
