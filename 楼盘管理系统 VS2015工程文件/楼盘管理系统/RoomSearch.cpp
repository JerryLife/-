#include "managerSys.h"
#include "resource.h"

#pragma comment(lib, "comctl32.lib")

static OPENFILENAME ofn;
static Community *pHead = NULL;
static Community *pComSearch = NULL;
static Building *pBuiSearch = NULL;
static Room *pRoomSearch = NULL;
static int companyNum = 1;
static int iPaintState;
static BOOL bRePaint;
static TCHAR comNow[100];	//目前所在com
static int buiNow;			//目前所在bui
static int roomNow;			//目前所在Room
static int iSearch = NOSEARCH;		//是否正在搜索
RECT rectData;
Community pcBuffer[10];
Building pbBuffer[10];
Room prBuffer[10];
HWND hwnd, hwndMainList, hwndMainTitle, hwndSubTitle;
HWND hwndButtonDel, hwndButtonEdit, hwndButtonEnter;
HWND hwndSubList;
HFONT hFontSon, hFontBlack;
enum fState {SUCCEED, FAIL};
enum fState fAddBuiState;		//是否成功添加楼栋
enum fState fAddRoomState;		//是否成功添加房间
int iAutoComNum = 1, iAutoBuiNum = 101;

static TCHAR *szComboBoxData[] = {
	TEXT("户型1：一室一厅一厨一厕"),TEXT("户型18：五室一厅一厨两厕"),
	TEXT("户型2：两室一厅一厨一厕"),TEXT("户型19：两室两厅两厨一厕"),
	TEXT("户型3：三室一厅一厨一厕"),TEXT("户型20：三室两厅两厨一厕"),
	TEXT("户型4：四室一厅一厨一厕"),TEXT("户型21：四室两厅两厨一厕"),
	TEXT("户型5：五室一厅一厨一厕"),TEXT("户型22：五室两厅两厨一厕"),
	TEXT("户型6：一室两厅一厨一厕"),TEXT("户型23：两室两厅一厨两厕"),
	TEXT("户型7：两室两厅一厨一厕"),TEXT("户型24：三室两厅一厨两厕"),
	TEXT("户型8：三室两厅一厨一厕"),TEXT("户型25：四室两厅一厨两厕"),
	TEXT("户型9：四室两厅一厨一厕"),TEXT("户型26：五室两厅一厨两厕"),
	TEXT("户型10：五室两厅一厨一厕"),TEXT("户型27：两室一厅两厨两厕"),
	TEXT("户型11：两室一厅两厨一厕"),TEXT("户型28：三室一厅两厨两厕"),
	TEXT("户型12：三室一厅两厨一厕"),TEXT("户型29：四室一厅两厨两厕"),
	TEXT("户型13：四室一厅两厨一厕"),TEXT("户型30：五室一厅两厨两厕"),
	TEXT("户型14：五室一厅两厨一厕"),TEXT("户型31：两室两厅两厨两厕"),
	TEXT("户型15：两室一厅一厨两厕"),TEXT("户型32：三室两厅两厨两厕"),
	TEXT("户型16：三室一厅一厨两厕"),TEXT("户型33：四室两厅两厨两厕"),
	TEXT("户型17：四室一厅一厨两厕"),TEXT("户型34：五室两厅两厨两厕"),
};


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR szCmdLine, int iCmdShow)
{
	static TCHAR szWinName[] = TEXT("RoomSearch");
	MSG          msg;
	WNDCLASS     wndclass;
	
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = sizeof(unsigned int);
	wndclass.cbWndExtra = sizeof(unsigned int);
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_MainIcon));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = (LPCWSTR)IDM_RoomSearch;
	wndclass.lpszClassName = szWinName;

	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("本程序仅支持 Windows NT 以上系统!"),
			szWinName, MB_ICONERROR);
		return 0;
	}

	hwnd = CreateWindow(szWinName, TEXT("楼盘管理系统"),
		WS_OVERLAPPEDWINDOW,
		263, 127,
		872, 532,
		NULL, NULL, hInstance, NULL);

	HDC hdc = GetDC(hwnd);

	LOGFONT LogFont;
	memset(&LogFont, 0, sizeof(LOGFONT));
	lstrcpy(LogFont.lfFaceName, TEXT("宋体"));
	LogFont.lfWeight = 400;
	LogFont.lfHeight = GetDeviceCaps(hdc, LOGPIXELSY) * 12 / 72; // 字体大小
	LogFont.lfWidth = 0;
	LogFont.lfCharSet = GB2312_CHARSET;
	LogFont.lfOutPrecision = 3;
	LogFont.lfClipPrecision = 2;
	LogFont.lfOrientation = 45;
	LogFont.lfQuality = 1;
	LogFont.lfPitchAndFamily = 2;

	LOGFONT font;
	ZeroMemory(&font, sizeof(LOGFONT));
	font.lfHeight = 18;
	font.lfQuality = PROOF_QUALITY;
	wsprintf(font.lfFaceName, TEXT("微软雅黑"));
	hFontBlack = CreateFontIndirect(&font);

	ReleaseDC(hwnd, hdc);

	// 创建字体"宋体“
	hFontSon = CreateFontIndirect(&LogFont);

	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

///////////////////////////////////////////////////////
//主窗口窗口过程

LRESULT CALLBACK WndProc(HWND hwnd,
	UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	TEXTMETRIC tm;

	static HWND hEdit;
	static TCHAR szFileName[MAX_PATH], szFileTitle[MAX_PATH];
	static RECT rectClass, rectSub;
	static HINSTANCE hInstance;
	static Community *pComPaint;
	static Building *pBuiPaint;
	static Room *pRoomPaint;
	static int iSaveState = FALSE;
	static int cxClient, cyClient, cxChar, cyChar;
	static TCHAR sInf[50];
	int i, iComNum, iBuiNum, iRoomNum, iAns;
	static int iIndex;
	Community *pTailCom;
	Building *pTailBui, *pTailBuiSearch;
	Room *pTailRoom, *pTailRoomSearch;
	TCHAR szBuffer[400];
	TCHAR szFormatBui[] = TEXT("%-20s%-5s%-10s%-15s");

	switch (message)
	{
	case WM_CREATE:
		hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
		InitCommonControls();//初始化listview环境

		hdc = GetDC(hwnd);

		GetTextMetrics(hdc, &tm);
		cxChar = tm.tmAveCharWidth;
		cyChar = tm.tmHeight + tm.tmExternalLeading;

		ReleaseDC(hwnd, hdc);

		hwndSubList = CreateWindowEx(NULL, TEXT("SysListView32"), NULL, 
				WS_VISIBLE | WS_CHILD | WS_BORDER | LVS_REPORT, 
				0, 0, 0, 0, hwnd, (HMENU)ID_SUBLIST,
				(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
		ListView_SetExtendedListViewStyle(hwndSubList,
			LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
		SendMessage(hwndSubList, WM_SETFONT,
			(WPARAM)hFontBlack, TRUE);

		hwndMainList = CreateWindow(TEXT("listbox"), NULL,
			WS_VISIBLE | WS_CHILD | LBS_NOTIFY | WS_VSCROLL | WS_BORDER,
			0, 0, 0, 0, hwnd, (HMENU)ID_MAINLIST,
			(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
		SendMessage(hwndMainList, WM_SETFONT, 
			(WPARAM)GetStockObject(OEM_FIXED_FONT), TRUE);
		
		hwndMainTitle = CreateWindow(TEXT("static"), TEXT("楼 盘"),
			WS_VISIBLE | WS_CHILD | SS_CENTER, 
			0, 0, 0, 0, hwnd, (HMENU)(ID_MAINLISTTITLE),
			(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
		SendMessage(hwndMainTitle, WM_SETFONT, 
			(WPARAM)GetStockObject(OEM_FIXED_FONT), 0);

		hwndButtonDel = CreateWindow(TEXT("button"), TEXT("删除"),
			WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
			0, 0, 0, 0, hwnd, (HMENU)(ID_BUTTONDEL),
			hInstance, NULL);
		SendMessage(hwndButtonDel, WM_SETFONT,
			(WPARAM)GetStockObject(OEM_FIXED_FONT), 0);

		hwndButtonEdit = CreateWindow(TEXT("button"), TEXT("编辑"),
			WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
			0, 0, 0, 0, hwnd, (HMENU)(ID_BUTTONEDIT),
			hInstance, NULL);
		SendMessage(hwndButtonEdit, WM_SETFONT,
			(WPARAM)GetStockObject(OEM_FIXED_FONT), 0);

		hwndButtonEnter = CreateWindow(TEXT("button"), TEXT("进入"),
			WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
			0, 0, 0, 0, hwnd, (HMENU)(ID_BUTTONENTER),
			hInstance, NULL);
		SendMessage(hwndButtonEdit, WM_SETFONT,
			(WPARAM)GetStockObject(OEM_FIXED_FONT), 0);

		FileInitWnd(hwnd);
		iSaveState = TRUE;

		return 0;

	case WM_SIZE:

		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);

		rectData.left = XLINEPOS;
		rectData.right = cxClient;
		rectData.top = 0;
		rectData.bottom = YLINEPOS;

		MoveWindow(hwndSubList, XLINEPOS + 3, YLINEPOS + 3,
			cxClient - XLINEPOS - 6, cyClient - YLINEPOS - 6, TRUE);

		MoveWindow(hwndMainList, 3, cyChar * 5 / 4 + 3,
			XLINEPOS - 6, cyClient - 6, TRUE);

		MoveWindow(hwndMainTitle, 2, 2,
			XLINEPOS -3, cyChar * 5 / 4, TRUE);
		
		return 0;

	case WM_COMMAND:
		
		switch (LOWORD(wParam))
		{
		case ID_BUTTONDEL:

			pTailCom = pHead;

			//循环查找楼盘名
			while (pTailCom != NULL)
			{
				if (!lstrcmp(pTailCom->name, comNow))
				{
					iComNum = pTailCom->communityNum;
					break;
				}
				pTailCom = pTailCom->nextCommunity;
			}

			switch (iPaintState)
			{
			case PA_FIRSTEPCOM:
				iAns = MessageBox(hwnd, TEXT("确定要删除该楼盘吗"),
					TEXT("提示"), MB_OKCANCEL);
				if (iAns == IDOK)
				{
					iSaveState = FALSE;
					DelComInList(iComNum, &pHead);

					if (pHead)
					{
						lstrcpy(comNow, pHead->name);
						UpdateData(&pHead);

						//存储即将绘制的Com地址
						pComPaint = (Community*)malloc(sizeof(Community));
						*pComPaint = *pHead;
					}
					else pComPaint = NULL;

					//确定将要绘制的级别并无效窗口
					iPaintState = PA_FIRSTEPCOM;
					InvalidateRect(hwnd, NULL, TRUE);
				}
				else return 0;
				break;

			case PA_SECSTEPBUI:
			case PA_FIRSTEPBUI:

				iAns = MessageBox(hwnd, TEXT("确定要删除该楼栋吗"),
					TEXT("提示"), MB_OKCANCEL);
				if (iAns == IDOK)
				{
					iSaveState = FALSE;
					DelBuiInList(iComNum, buiNow, &pHead);

					for (pTailCom = pHead; pTailCom; pTailCom = pTailCom->nextCommunity)
						if (lstrcmp(comNow, pTailCom->name) == 0)
						{
							UpdateData(&pHead);

							//存储即将绘制的Com地址
							pComPaint = (Community*)malloc(sizeof(Community));
							*pComPaint = *pTailCom;
							if (pTailCom->buildings)
								buiNow = pTailCom->buildings->buildingNum;
							else buiNow = 0;
							break;
						}

					//确定将要绘制的级别并无效窗口
					iPaintState = PA_FIRSTEPCOM;
					InvalidateRect(hwnd, NULL, TRUE);
				}
				else return 0;
				break;

			case PA_SECSTEPROOM:
				iAns = MessageBox(hwnd, TEXT("确定要删除该房间吗"),
					TEXT("提示"), MB_OKCANCEL);
				if (iAns == IDOK)
				{
					iSaveState = FALSE;
					DelRoomInList(iComNum, buiNow, roomNow, &pHead);

					for (pTailCom = pHead; pTailCom; pTailCom = pTailCom->nextCommunity)
						if (lstrcmp(comNow, pTailCom->name) == 0)
						{
							UpdateData(&pHead);

							//存储即将绘制的Com地址
							pComPaint = (Community*)malloc(sizeof(Community));
							*pComPaint = *pTailCom;
							for (pTailBui = pTailCom->buildings; pTailBui != NULL;
								pTailBui = pTailBui->nextBuilding)
								if (buiNow == pTailBui->buildingNum)
								{
									//存储将要绘制的Bui地址
									pBuiPaint = (Building*)malloc(sizeof(Building));
									*pBuiPaint = *pTailBui;
									break;
								}
							break;
						}

					//确定将要绘制的级别并无效窗口
					iPaintState = PA_SECSTEPBUI;
					InvalidateRect(hwnd, NULL, TRUE);
				}
				else return 0;
				break;

			default:
				break;
			}
			break;
			
		case ID_BUTTONEDIT:

			iSaveState = FALSE;

			switch (iPaintState)
			{
			case PA_FIRSTEPCOM:

				DialogBox(hInstance, MAKEINTRESOURCE(IDD_COMEDIT),
					hwnd, EditComDlgProc);
				
				if (pHead)
				{
					lstrcpy(comNow, pHead->name);
					UpdateData(&pHead);

					//存储即将绘制的Com地址
					pComPaint = (Community*)malloc(sizeof(Community));
					*pComPaint = *pHead;
				}
				else pComPaint = NULL;

				//确定将要绘制的级别并无效窗口
				iPaintState = PA_FIRSTEPCOM;
				InvalidateRect(hwnd, NULL, TRUE);

				break;

			case PA_SECSTEPBUI:
			case PA_FIRSTEPBUI:

				DialogBox(hInstance, MAKEINTRESOURCE(IDD_BUIEDIT),
					hwnd, EditBuiDlgProc);

				for (pTailCom = pHead; pTailCom; pTailCom = pTailCom->nextCommunity)
					if (lstrcmp(comNow, pTailCom->name) == 0)
					{
						UpdateData(&pHead);

						//存储即将绘制的Com地址
						pComPaint = (Community*)malloc(sizeof(Community));
						*pComPaint = *pTailCom;
						for (pTailBui = pTailCom->buildings; pTailBui != NULL;
							pTailBui = pTailBui->nextBuilding)
							if (buiNow == pTailBui->buildingNum)
							{
								//存储将要绘制的Bui地址
								pBuiPaint = (Building*)malloc(sizeof(Building));
								*pBuiPaint = *pTailBui;
								break;
							}
						break;
					}

				//确定将要绘制的级别并无效窗口
				iPaintState = PA_FIRSTEPBUI;
				InvalidateRect(hwnd, NULL, TRUE);
				break;

			case PA_SECSTEPROOM:

				DialogBox(hInstance, MAKEINTRESOURCE(IDD_ROOMEDIT),
					hwnd, EditRoomDlgProc);

				for (pTailCom = pHead; pTailCom; pTailCom = pTailCom->nextCommunity)
					if (lstrcmp(comNow, pTailCom->name) == 0)
					{
						UpdateData(&pHead);

						//存储即将绘制的Com地址
						pComPaint = (Community*)malloc(sizeof(Community));
						*pComPaint = *pTailCom;

						for (pTailBui = pTailCom->buildings; pTailBui != NULL;
							pTailBui = pTailBui->nextBuilding)
							if (buiNow == pTailBui->buildingNum)
							{
								//存储将要绘制的Bui地址
								pBuiPaint = (Building*)malloc(sizeof(Building));
								*pBuiPaint = *pTailBui;

								for (pTailRoom = pTailBui->rooms; pTailRoom != NULL;
									pTailRoom = pTailRoom->nextRoom)
									if (roomNow == pTailRoom->roomNum)
									{
										//存储将要绘制的Bui地址
										pRoomPaint = (Room*)malloc(sizeof(Room));
										*pRoomPaint = *pTailRoom;
									}
								break;
							}
						break;
					}

				//确定将要绘制的级别并无效窗口
				iPaintState = PA_SECSTEPROOM;
				InvalidateRect(hwnd, NULL, TRUE);

				break;

			default:
				break;
			}

			return 0;

		case ID_BUTTONENTER:

			switch (iSearch)
			{
			case SEARCHCOM:

				//获取选中项文本
				SendMessage(hwndMainList, LB_GETTEXT,
					iIndex, (LPARAM)szBuffer);

				for (pTailCom = pHead; pTailCom; pTailCom = pTailCom->nextCommunity)
					if (lstrcmp(szBuffer, pTailCom->name) == 0)
					{
						//存储即将绘制的Com地址
						pComPaint = (Community*)malloc(sizeof(Community));
						*pComPaint = *pTailCom;
						lstrcpy(comNow, szBuffer);
						break;
					}

				EnableWindow(hwndButtonEdit, TRUE);
				EnableWindow(hwndButtonDel, TRUE);

				pComSearch = NULL;
				iPaintState = PA_FIRSTEPCOM;
				InvalidateRect(hwnd, NULL, TRUE);
				MoveWindow(hwndButtonEnter, 0, 0, 0, 0, TRUE);

				break;

			case SEARCHBUI:

				//获取选中项代表楼栋
				for (pTailBuiSearch = pBuiSearch, i = 0; pTailBuiSearch != NULL;
					pTailBuiSearch = pTailBuiSearch->nextBuilding, i++)
					if (i == iIndex)
						break;

				for (pTailCom = pHead; pTailCom; pTailCom = pTailCom->nextCommunity)
					if (lstrcmp(pTailBuiSearch->inCom, pTailCom->name) == 0)
						for(pTailBui = pTailCom->buildings;pTailBui;
							pTailBui = pTailBui->nextBuilding)
							if (pTailBuiSearch->buildingNum == pTailBui->buildingNum)
							{
								pBuiPaint = (Building*)malloc(sizeof(Building));
								*pBuiPaint = *pTailBui;
								buiNow = pTailBui->buildingNum;
								break;
							}
		
				EnableWindow(hwndButtonEdit, TRUE);
				EnableWindow(hwndButtonDel, TRUE);

				pBuiSearch = NULL;
				iPaintState = PA_SECSTEPBUI;
				InvalidateRect(hwnd, NULL, TRUE);
				MoveWindow(hwndButtonEnter, 0, 0, 0, 0, TRUE);

				break;

			case SEARCHROOM:

				//获取选中项代表房间
				for (pTailRoomSearch = pRoomSearch, i = 0; pTailRoomSearch != NULL;
					pTailRoomSearch = pTailRoomSearch->nextRoom, i++)
					if (i == iIndex)
						break;

				for (pTailCom = pHead; pTailCom; pTailCom = pTailCom->nextCommunity)
					if (lstrcmp(pTailRoomSearch->inCom, pTailCom->name) == 0)
						for (pTailBui = pTailCom->buildings; pTailBui;
							pTailBui = pTailBui->nextBuilding)
							if (pTailRoomSearch->buildingNum == pTailBui->buildingNum)
								for(pTailRoom = pTailBui->rooms;pTailRoom;
									pTailRoom = pTailRoom->nextRoom)
									if (pTailRoom->roomNum == pTailRoomSearch->roomNum)
									{
										pRoomPaint = (Room*)malloc(sizeof(Room));
										*pRoomPaint = *pTailRoom;
										roomNow = pTailRoom->roomNum;
										break;
									}

				EnableWindow(hwndButtonEdit, TRUE);
				EnableWindow(hwndButtonDel, TRUE);

				pRoomSearch = NULL;
				iPaintState = PA_SECSTEPROOM;
				InvalidateRect(hwnd, NULL, TRUE);
				MoveWindow(hwndButtonEnter, 0, 0, 0, 0, TRUE);
				break;

			default:
				break;
			}

			iSearch = NOSEARCH;
			break;

			//处理 IDM_FILE 文件系列功能

		case IDM_FILE_SAVE:				//文件-保存数据*

			if (szFileName[0])
			{
				if (FileWriteWnd(hwnd, szFileName))
				{
					iSaveState = TRUE;
					return 1;
				}
				else
				{
					wsprintf(szBuffer, TEXT("无法打开文件 %s"), szFileName);
					MessageBox(hwnd, szBuffer, TEXT("提示"), MB_OK);
					return 0;
				}
			}
			//如果szFileName不存在，则跳转到“另存为”

		case IDM_FILE_SAVE_AS:			//文件-数据另存为*

			if (FileSaveDlg(hwnd, szFileName, szFileTitle))
			{
				SetTitle(hwnd, szFileTitle);

				if (FileWriteWnd(hwnd, szFileName))
				{
					iSaveState = TRUE;
					return 1;
				}
				else
				{
					wsprintf(szBuffer, TEXT("无法打开文件 %s"), szFileName);
					MessageBox(hwnd, szBuffer, TEXT("提示"), MB_OK);
					return 0;
				}
			}
			return 0;

		case IDM_FILE_CREATE:			//文件-新建*

			if (!AskConfirm(hwnd, iSaveState))
				return 0;

			szFileName[0] = '\0';
			szFileTitle[0] = '\0';

			SetTitle(hwnd, szFileTitle);
			iSaveState = TRUE;
			return 0;

		case IDM_FILE_IMPORT:			//文件-导入数据*

			//询问用户是否需要保存
			if (!AskConfirm(hwnd, iSaveState))
				return 0;

			if (FileImportDlg(hwnd, szFileName, szFileTitle))
				if (!FileReadWnd(hEdit, szFileName))
				{
					wsprintf(szBuffer, TEXT("无法打开文件 %s"),
						szFileTitle[0] ? szFileTitle : TEXT("无标题"));
					MessageBox(hwnd, szBuffer, TEXT("楼盘管理系统"),
						MB_OK | MB_ICONEXCLAMATION);

					szFileName[0] = '\0';
					szFileTitle[0] = '\0';
				}
				else
				{
					//初始化选项
					lstrcpy(comNow, pHead->name);
					if (pHead->buildings != NULL)
						buiNow = pHead->buildings->buildingNum;

					//确认将要绘制的成员
					if (pHead)
					{
						pComPaint = (Community*)malloc(sizeof(Community));
						*pComPaint = *pHead;
						if (pHead->buildings)
						{
							pBuiPaint = (Building*)malloc(sizeof(Building));
							*pBuiPaint = *(pHead->buildings);
						}
					}
					else return 0;

					//开始绘制
					iPaintState = PA_FIRSTEPCOM;
					InvalidateRect(hwnd, NULL, TRUE);
				}


			SetTitle(hwnd, szFileTitle);
			iSaveState = TRUE;
			return 0;

		case IDM_FILE_EXIT:				//文件-退出程序*

			SendMessage(hwnd, WM_CLOSE, 0, 0);
			break;

			//处理 IDM_EDIT 编辑部分功能

		case IDM_EDIT_CHANGE_DATA:		//编辑-更改-更改数据

			SendMessage(hwndButtonEdit, BM_CLICK, 0, 0);
			break;

		case IDM_EDIT_ADD_ROOM:			//编辑-添加-添加新房间*

			DialogBox(hInstance, MAKEINTRESOURCE(IDD_ADD_ROOM),
				hwnd, AddRoomDlgProc);
			if (bRePaint)
			{
				//将这个房间结构放进链表^_^
				for (pTailCom = pHead; pTailCom;
					pTailCom = pTailCom->nextCommunity)
				{
					if (lstrcmp(prBuffer[0].inCom, pTailCom->name) == 0)
					{
						prBuffer[0].communityNum = pTailCom->communityNum;
						for (pTailBui = pTailCom->buildings; pTailBui;
							pTailBui = pTailBui->nextBuilding)
							if (prBuffer[0].buildingNum == pTailBui->buildingNum)
							{
								if (CheckSameRoom(prBuffer[0].inCom, 
									prBuffer[0].buildingNum, prBuffer[0].roomNum, &pHead))
									AddRoomToList(prBuffer[0], &pHead,
										prBuffer[0].communityNum, prBuffer[0].buildingNum);
								else 
								{
									MessageBox(hwnd, TEXT("该房间名已在系统中"),
										TEXT("警告"), MB_OK | MB_ICONWARNING);
									break;
								}
								UpdateData(&pHead);

								//存储即将绘制的Bui地址
								pBuiPaint = (Building*)malloc(sizeof(Building));
								*pBuiPaint = *pTailBui;
								break;
							}
					}
				}

				//确定将要绘制的级别并无效窗口
				iPaintState = PA_SECSTEPBUI;
				InvalidateRect(hwnd, NULL, TRUE);

				iSaveState = FALSE;
			}
			break;

		case IDM_EDIT_ADD_BUID:			//编辑-添加-添加新楼栋*

			DialogBox(hInstance, MAKEINTRESOURCE(IDD_ADD_BUI),
				hwnd, AddBuiDlgProc);

			if (bRePaint)
			{
				//将这个楼栋结构放进链表^_^
				for (pTailCom = pHead; pTailCom; pTailCom = pTailCom->nextCommunity)
					if (lstrcmp(pbBuffer[0].inCom, pTailCom->name) == 0)
					{
						if (CheckSameBui(pbBuffer[0].inCom, pbBuffer[0].buildingNum, &pHead))
							AddBuiToList(pbBuffer[0], &pHead, pTailCom->communityNum);
						else
						{
							MessageBox(hwnd, TEXT("该楼栋名已在系统中"),
								TEXT("警告"), MB_OK | MB_ICONWARNING);
							break;
						}
						UpdateData(&pHead);

						//存储即将绘制的Com地址
						pComPaint = (Community*)malloc(sizeof(Community));
						*pComPaint = *pTailCom;
						buiNow = pbBuffer[0].buildingNum;
						break;
					}
				
				//确定将要绘制的级别并无效窗口
				iPaintState = PA_FIRSTEPCOM;
				InvalidateRect(hwnd, NULL, TRUE);

				iSaveState = FALSE;
			}
			break;

		case IDM_EDIT_ADD_COM:			//编辑-添加-添加新楼盘*

			DialogBox(hInstance, MAKEINTRESOURCE(IDD_ADD_COM),
				hwnd, AddComDlgProc);
			if (bRePaint)
			{
				if (pcBuffer[0].name)
				{
					if (CheckSameCom(pcBuffer[0].name, &pHead))
						AddComToList(pcBuffer[0], &pHead);
					else
					{
						MessageBox(hwnd, TEXT("该楼盘名已在系统中"),
							TEXT("警告"), MB_OK | MB_ICONWARNING);
						break;
					}
					UpdateData(&pHead);
					for (pTailCom = pHead; pTailCom; pTailCom = pTailCom->nextCommunity)
						if (lstrcmp(pcBuffer[0].name, pTailCom->name) == 0)
						{
							//存储即将绘制的Com地址
							pComPaint = (Community*)malloc(sizeof(Community));
							*pComPaint = *pTailCom;
							lstrcpy(comNow, pcBuffer[0].name);
							break;
						}
				}

				//确定将要绘制的级别并无效窗口
				iPaintState = PA_FIRSTEPCOM;
				InvalidateRect(hwnd, NULL, TRUE);

				iSaveState = FALSE;
			}

			break;

		case IDM_EDIT_DEL_ROOM:			//编辑-删除-删除房间

			DialogBox(hInstance, MAKEINTRESOURCE(IDD_DEL_ROOM),
				hwnd, DelRoomDlgProc);
			if (bRePaint)
			{
				for (pTailCom = pHead; pTailCom; pTailCom = pTailCom->nextCommunity)
					if (lstrcmp(comNow, pTailCom->name) == 0)
					{
						UpdateData(&pHead);

						//存储即将绘制的Com地址
						pComPaint = (Community*)malloc(sizeof(Community));
						*pComPaint = *pTailCom;
						for (pTailBui = pTailCom->buildings; pTailBui != NULL;
							pTailBui = pTailBui->nextBuilding)						
							if (buiNow == pTailBui->buildingNum)
							{
								//存储将要绘制的Bui地址
								pBuiPaint = (Building*)malloc(sizeof(Building));
								*pBuiPaint = *pTailBui;
							}			
						break;
					}

				//确定将要绘制的级别并无效窗口
				iPaintState = PA_SECSTEPBUI;
				InvalidateRect(hwnd, NULL, TRUE);

				iSaveState = FALSE;
			}
			break;

		case IDM_EDIT_DEL_BUID:			//编辑-删除-删除楼栋

			DialogBox(hInstance, MAKEINTRESOURCE(IDD_DEL_BUI),
				hwnd, DelBuiDlgProc);
			if (bRePaint)
			{
				for (pTailCom = pHead; pTailCom; pTailCom = pTailCom->nextCommunity)
					if (lstrcmp(comNow, pTailCom->name) == 0)
					{
						UpdateData(&pHead);

						//存储即将绘制的Com地址
						pComPaint = (Community*)malloc(sizeof(Community));
						*pComPaint = *pTailCom;
						if(pTailCom->buildings)
							buiNow = pTailCom->buildings->buildingNum;
						else buiNow = 0;
						break;
					}

				//确定将要绘制的级别并无效窗口
				iPaintState = PA_FIRSTEPCOM;
				InvalidateRect(hwnd, NULL, TRUE);

				iSaveState = FALSE;
			}
			break;

		case IDM_EDIT_DEL_COM:			//编辑-删除-删除楼盘

			DialogBox(hInstance, MAKEINTRESOURCE(IDD_DEL_COM),
				hwnd, DelComDlgProc);
			if (bRePaint)
			{
				if (pHead)
				{
					lstrcpy(comNow, pHead->name);
					UpdateData(&pHead);

					//存储即将绘制的Com地址
					pComPaint = (Community*)malloc(sizeof(Community));
					*pComPaint = *pHead;
				}
				else pComPaint = NULL;
				
				//确定将要绘制的级别并无效窗口
				iPaintState = PA_FIRSTEPCOM;
				InvalidateRect(hwnd, NULL, TRUE);

				iSaveState = FALSE;
			}

			break;

			//处理 IDM_SEARCH 查询部分功能

		case IDM_SEARCH:			//查询-信息检索

			DialogBox(hInstance, MAKEINTRESOURCE(IDD_SEARCH),
				hwnd, SearchDlgProc);
			switch (iSearch)
			{
			case SEARCHCOM:
				pComPaint = (Community*)malloc(sizeof(Community));
				if(pComPaint)
					*pComPaint = *pComSearch;
				break;

			case SEARCHBUI:
				pBuiPaint = (Building*)malloc(sizeof(Building));
				if(pBuiSearch)
					*pBuiPaint = *pBuiSearch;
				break;

			case SEARCHROOM:
				pRoomPaint = (Room*)malloc(sizeof(Room));
				if (pRoomSearch)
					*pRoomPaint = *pRoomSearch;
				break;

			default:
				break;
			}

			InvalidateRect(hwnd, NULL, TRUE);
			break;

			//处理 IDM_HELP 帮助部分功能

		case IDM_HELP_ABOUT:			//关于软件

			DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX),
				hwnd, AboutDlgProc);
			break;

		case ID_MAINLIST:

			if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				iIndex = SendMessage(hwndMainList, LB_GETCURSEL, 0, 0);

				//获取选中项文本
				SendMessage(hwndMainList, LB_GETTEXT, iIndex, (LPARAM)szBuffer);

				if (SEARCHING && !lstrcmp(szBuffer, TEXT("【退出搜索】")))
				{
					iSearch = NOSEARCH;
					iPaintState = PA_FIRSTEPCOM;

					EnableWindow(hwndButtonEdit, TRUE);
					EnableWindow(hwndButtonDel, TRUE);

					pComSearch = NULL;
					pBuiSearch = NULL;
					pRoomSearch = NULL;
					MoveWindow(hwndButtonEnter, 0, 0, 0, 0, TRUE);
					InvalidateRect(hwnd, NULL, TRUE);
					break;
				}

				if (iSearch == SEARCHBUI)
				{
					//获取选中项代表楼栋		
					for (pTailBui = pBuiSearch, i = 0; pTailBui != NULL;
						pTailBui = pTailBui->nextBuilding, i++)
						if (i == iIndex)
							break;

					pBuiPaint = (Building*)malloc(sizeof(Building));
					*pBuiPaint = *pTailBui;

					//遍历链表寻找所在楼盘
					for (pTailCom = pHead; pTailCom;
						pTailCom = pTailCom->nextCommunity)
						if (lstrcmp(pTailCom->name, pBuiPaint->inCom) == 0)
						{
							//存储即将绘制的Com地址
							pComPaint = (Community*)malloc(sizeof(Community));
							*pComPaint = *pTailCom;
						}

					InvalidateRect(hwnd, NULL, TRUE);
					break;
				}
				else if (iSearch == SEARCHROOM)
				{
					//获取选中项代表楼栋		
					for (pTailRoom = pRoomSearch, i = 0; pTailRoom != NULL;
						pTailRoom = pTailRoom->nextRoom, i++)
						if (i == iIndex)
							break;

					pRoomPaint = (Room*)malloc(sizeof(Room));
					*pRoomPaint = *pTailRoom;

					//遍历链表寻找所在楼盘、楼栋
					for (pTailCom = pHead; pTailCom;
						pTailCom = pTailCom->nextCommunity)
						if (lstrcmp(pTailCom->name, pRoomPaint->inCom) == 0)
						{
							//存储即将绘制的Com地址
							pComPaint = (Community*)malloc(sizeof(Community));
							*pComPaint = *pTailCom;

							for (pTailBui = pTailCom->buildings; pTailBui;
								pTailBui = pTailBui->nextBuilding)
								if (pTailBui->buildingNum == pRoomPaint->buildingNum)
								{
									pBuiPaint = (Building*)malloc(sizeof(Building));
									*pBuiPaint = *pTailBui;
								}
						}

					InvalidateRect(hwnd, NULL, TRUE);
					break;
				}

				//储存所在楼盘名
				lstrcpy(comNow, szBuffer);

				//遍历链表寻找该楼盘
				for (pTailCom = pHead; pTailCom;
					pTailCom = pTailCom->nextCommunity)
					if (lstrcmp(pTailCom->name, szBuffer) == 0)
					{
						//存储即将绘制的Com地址
						pComPaint = (Community*)malloc(sizeof(Community));
						*pComPaint = *pTailCom;
					}

				//确定将要绘制的级别并无效窗口
				iPaintState = PA_FIRSTEPCOM;
				InvalidateRect(hwnd, NULL, TRUE);
			}
			break;
		}

		break;

	case WM_NOTIFY:

		switch (wParam)
		{
		case ID_SUBLIST:

			//【双击】二级菜单，显示下一级菜单

			if ((((NMHDR *)lParam)->code) == NM_DBLCLK)
			{
				//获取选中项文本
				NM_LISTVIEW *pNMListView = (NM_LISTVIEW *)lParam;
				iIndex = pNMListView->iItem;

				ListView_GetItemText(hwndSubList, iIndex, 0, szBuffer, 4096);

				//当次级列表中为楼栋时，双击进入下一级

				if (iPaintState == PA_FIRSTEPBUI
					|| iPaintState == PA_FIRSTEPCOM)
				{
					//判断双击处是否有数据
					if (iIndex<0 || iIndex>ListView_GetItemCount(hwndSubList))
						break;

					//获取前三位楼栋号
					szBuffer[3] = '\0\0';
					iBuiNum = _ttoi(szBuffer);

					buiNow = iBuiNum;

					//遍历链表寻找所在楼盘
					for (pTailCom = pHead; pTailCom;
						pTailCom = pTailCom->nextCommunity)
						if (lstrcmp(pTailCom->name, comNow) == 0)
						{
							//存储即将绘制的Com地址
							pComPaint = (Community*)malloc(sizeof(Community));
							*pComPaint = *pTailCom;

							//遍历链表寻找该楼栋
							for (pTailBui = pTailCom->buildings; pTailBui;
								pTailBui = pTailBui->nextBuilding)
								if (pTailBui->buildingNum == buiNow)
								{
									//存储即将绘制的Bui地址
									pBuiPaint = (Building*)malloc(sizeof(Building));
									*pBuiPaint = *pTailBui;
									buiNow = pBuiPaint->buildingNum;
									break;
								}
							break;	//跳出循环
						}

					//绘制下一级
					iPaintState = PA_SECSTEPBUI;
					InvalidateRect(hwnd, NULL, TRUE);
				}

				//当前次级列表中为房间时，仅响应双击第一栏返回上一级的双击消息

				else if (iPaintState == PA_SECSTEPBUI
					|| iPaintState == PA_SECSTEPROOM)
				{
					//判断双击处是否为第一项
					if (iIndex != 0)
						break;

					//遍历链表寻找所在楼盘
					for (pTailCom = pHead; pTailCom;
						pTailCom = pTailCom->nextCommunity)
						if (lstrcmp(pTailCom->name, comNow) == 0)
						{
							//存储即将绘制的Com地址
							pComPaint = (Community*)malloc(sizeof(Community));
							*pComPaint = *pTailCom;

							//遍历链表寻找该楼栋
							for (pTailBui = pTailCom->buildings; pTailBui;
								pTailBui = pTailBui->nextBuilding)
								if (pTailBui->buildingNum == buiNow)
								{
									//存储即将绘制的Bui地址
									pBuiPaint = (Building*)malloc(sizeof(Building));
									*pBuiPaint = *pTailBui;
									buiNow = pBuiPaint->buildingNum;
									break;
								}
							break;	//跳出循环
						}

					//绘制上一级
					iPaintState = PA_FIRSTEPBUI;
					InvalidateRect(hwnd, NULL, TRUE);
				}
			}

			//单击二级菜单，改变显示数据

			else if ((((NMHDR *)lParam)->code) == NM_CLICK)
			{
				//获取选中项文本
				NM_LISTVIEW *pNMListView = (NM_LISTVIEW *)lParam;
				iIndex = pNMListView->iItem;

				//判断单击处是否有数据
				if (iIndex<0 || iIndex>ListView_GetItemCount(hwndSubList))
					break;

				ListView_GetItemText(hwndSubList, iIndex, 0, szBuffer, 4096);

				//二级列表框为building的情况

				if (iPaintState == PA_FIRSTEPBUI
					|| iPaintState == PA_FIRSTEPCOM)
				{
					//获取前三位楼栋号
					szBuffer[3] = '\0\0';
					iBuiNum = _ttoi(szBuffer);

					//遍历链表寻找所在楼盘
					for (pTailCom = pHead; pTailCom;
						pTailCom = pTailCom->nextCommunity)
						if (lstrcmp(pTailCom->name, comNow) == 0)
						{
							//存储即将绘制的Com地址
							pComPaint = (Community*)malloc(sizeof(Community));
							*pComPaint = *pTailCom;

							//遍历链表寻找该楼栋
							for (pTailBui = pTailCom->buildings; pTailBui;
								pTailBui = pTailBui->nextBuilding)
								if (pTailBui->buildingNum == iBuiNum)
								{
									//存储即将绘制的Bui地址
									pBuiPaint = (Building*)malloc(sizeof(Building));
									*pBuiPaint = *pTailBui;
									buiNow = pBuiPaint->buildingNum;
									break;
								}
							break;	//跳出循环
						}

					//确定将要绘制的级别并无效窗口
					iPaintState = PA_FIRSTEPBUI;
					InvalidateRect(hwnd, NULL, TRUE);
				}

				//二级列表框为Room的情况

				else if (iPaintState == PA_SECSTEPBUI
					|| iPaintState == PA_SECSTEPROOM)
				{

					//若单击楼栋，则显示楼栋信息

					if (iIndex == 0)
					{
						iPaintState = PA_SECSTEPBUI;
						InvalidateRect(hwnd, NULL, TRUE);
						break;
					}

					//获取前三位房间号
					szBuffer[3] = '\0\0';
					iRoomNum = _ttoi(szBuffer);

					//遍历链表寻找所在楼盘
					for (pTailCom = pHead; pTailCom;
						pTailCom = pTailCom->nextCommunity)
						if (lstrcmp(pTailCom->name, comNow) == 0)
						{
							//存储即将绘制的Com地址
							pComPaint = (Community*)malloc(sizeof(Community));
							*pComPaint = *pTailCom;

							//遍历链表寻找该楼栋
							for (pTailBui = pTailCom->buildings; pTailBui;
								pTailBui = pTailBui->nextBuilding)
								if (pTailBui->buildingNum == buiNow)
								{
									//存储即将绘制的Bui地址
									pBuiPaint = (Building*)malloc(sizeof(Building));
									*pBuiPaint = *pTailBui;

									for (pTailRoom = pTailBui->rooms; pTailRoom;
										pTailRoom = pTailRoom->nextRoom)
									{
										if (pTailRoom->roomNum == iRoomNum)
										{
											//存储即将绘制的Room地址
											pRoomPaint = (Room*)malloc(sizeof(Room));
											*pRoomPaint = *pTailRoom;
										}
									}

								}
							break;	//跳出循环
						}

					//确定将要绘制的级别并无效窗口
					iPaintState = PA_SECSTEPROOM;
					InvalidateRect(hwnd, NULL, TRUE);
				}

			}
		default:
			break;
		}

		return 0;

	case WM_PAINT:

		hdc = BeginPaint(hwnd, &ps);

		SelectObject(hdc, hFontBlack);

		//画出基本框架
		rectSub = DrawBasicBk(hwnd, hdc, cxClient, cyClient);
		UpdateData(&pHead);

		if (iSearch == NOSEARCH)
		{
			if (iPaintState == PA_FIRSTEPCOM)
			{
				Draw1Step(hdc, pComPaint);
				RenewData(hdc, rectData);
				if (pComPaint)
					DrawComInf(hwnd, hdc, *pComPaint);
			}
			else if (iPaintState == PA_SECSTEPBUI)
			{
				Draw2Step(hdc, pBuiPaint);
				RenewData(hdc, rectData);
				if (pBuiPaint)
					DrawBuiInf(hwnd, hdc, *pBuiPaint);
			}
			else if (iPaintState == PA_FIRSTEPBUI)
			{
				Draw1Step(hdc, pComPaint);
				RenewData(hdc, rectData);
				if (pBuiPaint)
					DrawBuiInf(hwnd, hdc, *pBuiPaint);
			}
			else if (iPaintState == PA_SECSTEPROOM)
			{
				Draw2Step(hdc, pBuiPaint);
				RenewData(hdc, rectData);
				if (pRoomPaint)
					DrawRoomInf(hwnd, hdc, *pRoomPaint);
			}
		}
		else if (SEARCHING)
		{
			//重置信息
			SendMessage(hwndMainList, LB_RESETCONTENT, 0, 0);
			ListView_DeleteAllItems(hwndSubList);

			SetWindowText(hwndMainTitle, TEXT("搜索结果"));
			
			switch (iSearch)
			{
			case SEARCHCOM:

				//向搜索结果栏中添加信息
				for (pTailCom = pComSearch; pTailCom;
					pTailCom = pTailCom->nextCommunity)
				{
					SendMessage(hwndMainList, LB_ADDSTRING,
						0, (LPARAM)pTailCom->name);
				}
				SendMessage(hwndMainList, LB_ADDSTRING,
					0, (LPARAM)TEXT("【退出搜索】"));

				if (pComPaint)
				{
					DrawComInf(hwnd, hdc, *pComPaint);
					//iIndex = 0;
				}
				break;
				
			case SEARCHBUI:

				//向搜索结果栏中添加信息
				for (pTailBui = pBuiSearch; pTailBui;
					pTailBui = pTailBui->nextBuilding)
				{
					wsprintf(szBuffer, TEXT("%s%d栋"), 
						pTailBui->inCom, pTailBui->buildingNum);
					SendMessage(hwndMainList, LB_ADDSTRING,
						0, (LPARAM)szBuffer);
				}
				SendMessage(hwndMainList, LB_ADDSTRING,
					0, (LPARAM)TEXT("【退出搜索】"));

				if (pBuiPaint)
				{
					DrawBuiInf(hwnd, hdc, *pBuiPaint);
					//iIndex = 0;
				}
				break;

			case SEARCHROOM:

				//向搜索结果栏中添加信息
				for (pTailRoom = pRoomSearch; pTailRoom;
					pTailRoom = pTailRoom->nextRoom)
				{
					wsprintf(szBuffer, TEXT("%s%d栋%d"),
						pTailRoom->inCom, pTailRoom->buildingNum,
						pTailRoom->roomNum);
					SendMessage(hwndMainList, LB_ADDSTRING,
						0, (LPARAM)szBuffer);
				}
				SendMessage(hwndMainList, LB_ADDSTRING,
					0, (LPARAM)TEXT("【退出搜索】"));

				if (pRoomPaint)
				{
					DrawRoomInf(hwnd, hdc, *pRoomPaint);
					//iIndex = 0;
				}
				break;
				
			default:
				break;
			}

			EnableWindow(hwndButtonEdit, FALSE);
			EnableWindow(hwndButtonDel, FALSE);
		}

		EndPaint(hwnd, &ps);
		
		return 0;

	case WM_CLOSE:
		if (AskConfirm(hwnd, iSaveState))
			DestroyWindow(hwnd);
		return 0;
			
	case WM_QUERYENDSESSION:
		if (AskConfirm(hwnd, iSaveState))
			return 1;
		else
			return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	default:
		break;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}


///////////////////////////////////////////////////////
//“关于‘楼盘管理系统’”（About）对话框窗口过程

BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			EndDialog(hDlg, 0);
			return TRUE;
		}

		break;
	}

	return FALSE;
}


///////////////////////////////////////////////////////
//AskConfirm 
//功能：当用户退出系统时，若文件未保存，则提示其是否需要保存
//     若文件已经保存，则直接退出
//参数：退出的窗口句柄，保存状态（非0为已保存，0为未保存）
//返回：若需要退出系统，则返回TRUE；若不需要，则返回FALSE

BOOL AskConfirm(HWND hwnd, int iSaveState)
{
	int iAns;
	if (!iSaveState)
	{
		iAns = MessageBox(hwnd, TEXT("是否要保存您的更改"), TEXT("提示"), MB_YESNOCANCEL);
		switch (iAns)
		{

		case IDYES:
			SendMessage(hwnd, WM_COMMAND, (WPARAM)IDM_FILE_SAVE, 0);
			return TRUE;

		case IDNO:
			return TRUE;

		case IDCANCEL:
			return FALSE;

		}
	}
	else
		return TRUE;
}


///////////////////////////////////////////////////////
//DrawBasicBk
//窗口基本布局构造函数
//功能：画出窗口的基本布局，不含任何信息
//参数：窗口句柄,设备环境,屏幕宽，屏幕高
//返回：数据部分的RECT矩形

RECT DrawBasicBk(HWND hwnd, HDC hdc,int cxClient, int cyClient)
{
	RECT rectClass, rectData, rectSub;
	
	rectClass.left = 0;
	rectClass.top = 0;
	rectClass.right = XLINEPOS;
	rectClass.bottom = cyClient;

	Rectangle(hdc, rectClass.left, rectClass.top, 
		rectClass.right, rectClass.bottom);

	rectSub.left = XLINEPOS;
	rectSub.top = YLINEPOS;
	rectSub.right = cxClient;
	rectSub.bottom = cyClient;

	Rectangle(hdc, rectSub.left, rectSub.top,
		rectSub.right, rectSub.bottom);

	rectData.left = XLINEPOS;
	rectData.top = 0;
	rectData.right = cxClient;
	rectData.bottom = YLINEPOS;

	SelectObject(hdc, GetStockObject(DC_BRUSH));
	SetDCBrushColor(hdc, RGB(137, 189, 255));   //该颜色为天蓝色

	Rectangle(hdc, rectData.left, rectData.top,
		rectData.right, rectData.bottom);

	SelectObject(hdc, GetStockObject(WHITE_BRUSH));

	return rectData;
}


///////////////////////////////////////////////////////
//create3DepthList
//功能：创建一个三级链表
//参数：1、第一级个数；
//     2、每个第一级所含第二级个数
//     3、每个第二级所含第三级个数
//函数返回：返回该三级链表的头指针

Community *create3DepthList(int len1, int len2, int len3)
{
	Community *head = createComList(len1);
	Community *comTail;
	Building *buiTail;
	for (comTail = head->nextCommunity; comTail; comTail = comTail->nextCommunity)
	{
		comTail->buildings = createBuiList(len2);
		for (buiTail = comTail->buildings->nextBuilding; buiTail; buiTail->nextBuilding)
			buiTail->rooms = createRoomList(len3);
	}
	return head;
}


///////////////////////////////////////////////////////
//createComList
//功能：创建一个room的先进先出的单向链表
//参数：链表长度
//返回：该链表头指针

Community *createComList(int len)
{
	Community *head, *tail;
	tail = head = (Community*)malloc(sizeof(Community));
	while (len-- > 0)
	{
		tail->nextCommunity = (Community*)malloc(sizeof(Community));
		tail = tail->nextCommunity;
	}
	tail->nextCommunity = NULL;
	return head;
}


///////////////////////////////////////////////////////
//createBuiList
//功能：创建一个building的先进先出的单向链表
//参数：链表长度
//返回：该链表头指针

Building *createBuiList(int len)
{
	Building *head, *tail;
	tail = head = (Building*)malloc(sizeof(Building));
	while (len-- > 0)
	{
		tail->nextBuilding = (Building*)malloc(sizeof(Building));
		tail = tail->nextBuilding;
	}
	tail->nextBuilding = NULL;
	return head;
}


///////////////////////////////////////////////////////
//createRoomList
//功能：创建一个room的先进先出的单向链表
//参数：链表长度
//返回：该链表头指针

Room *createRoomList(int len)
{
	Room *head, *tail;
	tail = head = (Room*)malloc(sizeof(Room));
	while (len-- > 0)
	{
		tail->nextRoom = (Room*)malloc(sizeof(Room));
		tail = tail->nextRoom;
	}
	tail->nextRoom = NULL;
	return head;
}


///////////////////////////////////////////////////////
//“添加新楼盘”（AddCommunity）对话框窗口过程

BOOL CALLBACK AddComDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	TCHAR szBuffer[100][100];
	Community *pNewCom;
	int i;
	int iLength[5];

	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:

			//当按下OK时，将所输入内容存进存储区	

			iLength[0] = SendMessage(GetDlgItem(hDlg, IDC_ADD_COM_NAME),
				EM_GETLINE, 0, (LPARAM)szBuffer[0]);
			iLength[1] = SendMessage(GetDlgItem(hDlg, IDC_ADD_COM_ADDR),
				EM_GETLINE, 2, (LPARAM)szBuffer[1]);
			iLength[2] = SendMessage(GetDlgItem(hDlg, IDC_ADD_COM_PHONE),
				EM_GETLINE, 3, (LPARAM)szBuffer[2]);
			iLength[3] = SendMessage(GetDlgItem(hDlg, IDC_ADD_COM_PERSON),
				EM_GETLINE, 4, (LPARAM)szBuffer[3]);

			//检验每一项非空

			for (i = 0; i < 4; i++)
			{
				if (!iLength[i])
				{
					MessageBox(hDlg, TEXT("请输入完整的信息！"), TEXT("提示"), MB_OK);
					return FALSE;
				}
				else {
					*(szBuffer[i] + iLength[i]) = '\0\0';
				}
			}

			//分配空间，并将数据放入一个community结构体

			pNewCom = (Community*)malloc(sizeof(Community));

			lstrcpy(pNewCom->name, szBuffer[0]);
			lstrcpy(pNewCom->address, szBuffer[1]);
			lstrcpy(pNewCom->phone, szBuffer[2]);
			lstrcpy(pNewCom->host.name, szBuffer[3]);

			pNewCom->communityNum = iAutoComNum++;
			pNewCom->avgPrice = 0.0;

			//将这个楼盘存进全局缓冲区，之后加入链表
			pcBuffer[0] = *pNewCom;
			
			bRePaint = TRUE;
			EndDialog(hDlg, TRUE);
			return TRUE;

		case IDCANCEL:
			bRePaint = FALSE;
			EndDialog(hDlg, FALSE);
			return TRUE;
		}

		break;
	}

	return FALSE;
}


///////////////////////////////////////////////////////
//“添加新楼栋”（AddBuilding）对话框窗口过程

BOOL CALLBACK AddBuiDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	TCHAR szBuffer[100][100];
	Building *pNewBui;
	Community *ptailCom, *comTail;
	int i;
	int iLength[5];

	fAddBuiState = FAIL;

	switch (message)
	{
	case WM_INITDIALOG:

		//设置编辑框初始内容
		comTail = pHead;
		while (comTail != NULL)
		{
			if (!lstrcmp(comTail->name, comNow))
			{
				SetWindowText(GetDlgItem(hDlg, IDC_ADD_BUI_NAME), comTail->name);
				SetWindowText(GetDlgItem(hDlg, IDC_ADD_BUI_PHONE), comTail->phone);
				SetWindowText(GetDlgItem(hDlg, IDC_ADD_BUI_PERSON), comTail->host.name);
				break;
			}
			comTail = comTail->nextCommunity;
		}

		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:

			//当按下OK时，将所输入内容存进存储区	

			iLength[0] = SendMessage(GetDlgItem(hDlg, IDC_ADD_BUI_NAME),
				EM_GETLINE, 0, (LPARAM)szBuffer[0]);		  		 
			iLength[1] = SendMessage(GetDlgItem(hDlg, IDC_ADD_BUI_NUM),
				EM_GETLINE, 1, (LPARAM)szBuffer[1]);		 
			iLength[2] = SendMessage(GetDlgItem(hDlg, IDC_ADD_BUI_FLOORS),
				EM_GETLINE, 2, (LPARAM)szBuffer[2]);		
			iLength[3] = SendMessage(GetDlgItem(hDlg, IDC_ADD_BUI_PHONE),
				EM_GETLINE, 3, (LPARAM)szBuffer[3]);
			iLength[4] = SendMessage(GetDlgItem(hDlg, IDC_ADD_BUI_PERSON),
				EM_GETLINE, 4, (LPARAM)szBuffer[4]);

			//检验每一项非空

			for (i = 0; i < 5; i++)
			{
				if (!iLength[i])
				{
					MessageBox(hDlg, TEXT("请输入完整的信息！"), TEXT("提示"), MB_OK);
					return FALSE;
				}
				else {
					*(szBuffer[i] + iLength[i]) = '\0\0';
				}
			}

			//分配空间，并将数据放入一个building结构体

			pNewBui = (Building*)malloc(sizeof(Building));

			lstrcpy(pNewBui->inCom, szBuffer[0]);
			pNewBui->buildingNum = _ttoi(szBuffer[1]);
			pNewBui->numberOfFloors = _ttoi(szBuffer[2]);
			lstrcpy(pNewBui->host.phone, szBuffer[3]);
			lstrcpy(pNewBui->host.name, szBuffer[4]);

			pNewBui->avgPrice = 0.0;
			pNewBui->numberOfRooms = 0;

			//确认是否存在这个地址
			for (ptailCom = pHead; ptailCom; ptailCom = ptailCom->nextCommunity)
				if (lstrcmp(pNewBui->inCom, ptailCom->name) == 0)
					fAddBuiState = SUCCEED;

			//检验是否有对应楼盘
			if (fAddBuiState == FAIL)
			{
				MessageBox(hDlg, TEXT("所输入的楼盘不存在"), TEXT("提示"), MB_OK);
				break;
			}

			//将这个结构存入楼盘全局缓冲区
			pbBuffer[0] = *pNewBui;

			bRePaint = TRUE;	//授权更新界面
			EndDialog(hDlg, TRUE);
			return TRUE;

		case IDCANCEL:
			bRePaint = FALSE;	//禁止更新界面
			EndDialog(hDlg, FALSE);
			return TRUE;
		}

		break;
	}

	return FALSE;
}


///////////////////////////////////////////////////////
//“添加新房间”（AddRoom）对话框窗口过程

BOOL CALLBACK AddRoomDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	TCHAR szBuffer[100][100];
	Room *pNewRoom;
	Community *ptailCom, *comTail;
	Building *ptailBui, *buiTail;
	int i, iSold;
	int iLength[7];

	fAddRoomState = FAIL;

	switch (message)
	{
	case WM_INITDIALOG:

		//向ComboBox框中添加数据
		for (i = 0; i < 34; i++)
			SendMessage(GetDlgItem(hDlg, IDC_ADD_ROOM_STYLE),
				CB_ADDSTRING, 0, (LPARAM)szComboBoxData[i]);

		//设置编辑框初始内容
		comTail = pHead;
		while (comTail != NULL)
		{
			if (!lstrcmp(comTail->name, comNow))
			{
				buiTail = comTail->buildings;
				while (buiTail != NULL)
				{
					if (buiTail->buildingNum == buiNow)
					{
						SetWindowText(GetDlgItem(hDlg, IDC_ADD_ROOM_COMNAME), buiTail->inCom);
						wsprintf(szBuffer[1], TEXT("%d"), buiTail->buildingNum);
						SetWindowText(GetDlgItem(hDlg, IDC_ADD_ROOM_BUINUM), szBuffer[1]);
						break;
					}
					buiTail = buiTail->nextBuilding;
				}
				break;
			}
			comTail = comTail->nextCommunity;
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:

			//当按下OK时，将所输入内容存进存储区	

			iLength[0] = SendMessage(GetDlgItem(hDlg, IDC_ADD_ROOM_COMNAME),
				EM_GETLINE, 0, (LPARAM)szBuffer[0]);
			iLength[1] = SendMessage(GetDlgItem(hDlg, IDC_ADD_ROOM_BUINUM),
				EM_GETLINE, 1, (LPARAM)szBuffer[1]);
			iLength[2] = SendMessage(GetDlgItem(hDlg, IDC_ADD_ROOM_ROOMNUM),
				EM_GETLINE, 2, (LPARAM)szBuffer[2]);
			iLength[3] = SendMessage(GetDlgItem(hDlg, IDC_ADD_ROOM_SIZE),
				EM_GETLINE, 3, (LPARAM)szBuffer[3]);
			iLength[4] = SendMessage(GetDlgItem(hDlg, IDC_ADD_ROOM_PRICE),
				EM_GETLINE, 4, (LPARAM)szBuffer[4]);
			GetWindowText(GetDlgItem(hDlg, IDC_ADD_ROOM_STYLE),
				szBuffer[5], lstrlen(szBuffer[5]));
			iSold = SendMessage(GetDlgItem(hDlg, IDC_ADD_ROOM_SOLD),
				BM_GETCHECK, 0, 0);

			//检验每一项非空

			for (i = 0; i < 5; i++)
			{
				if (!iLength[i])
				{
					MessageBox(hDlg, TEXT("请输入完整的信息！"), TEXT("提示"), MB_OK);
					return FALSE;
				}
				else {
					*(szBuffer[i] + iLength[i]) = '\0\0';
				}
			}

			//分配空间，并将数据放入一个Room结构体

			pNewRoom = (Room*)malloc(sizeof(Room));

			lstrcpy(pNewRoom->inCom, szBuffer[0]);
			pNewRoom->buildingNum = _ttoi(szBuffer[1]);
			pNewRoom->roomNum = _ttoi(szBuffer[2]);
			pNewRoom->roomSize = _ttof(szBuffer[3]);
			pNewRoom->roomPrice = _ttof(szBuffer[4]);
			lstrcpy(pNewRoom->roomType, szBuffer[5]);
			pNewRoom->roomState = iSold;

			//确定是否存在这个地址
			for (ptailCom = pHead; ptailCom;
				ptailCom = ptailCom->nextCommunity)
			{
				if (lstrcmp(pNewRoom->inCom, ptailCom->name) == 0) 
				{
					for (ptailBui = ptailCom->buildings; ptailBui;
						ptailBui = ptailBui->nextBuilding)
						if (pNewRoom->buildingNum == ptailBui->buildingNum)
							fAddRoomState = SUCCEED;
				}
			}

			//检验是否有对应楼盘和楼栋
			if (fAddRoomState == FAIL)
			{
				MessageBox(hDlg, TEXT("所输入的楼盘或楼栋不存在"), TEXT("提示"), MB_OK);
				break;
			}

			//将这个结构存入楼盘全局缓冲区
			prBuffer[0] = *pNewRoom;

			bRePaint = TRUE;	//授权更新界面
			EndDialog(hDlg, TRUE);
			return TRUE;

		case IDCANCEL:
			bRePaint = FALSE;	//禁止更新界面
			EndDialog(hDlg, FALSE);
			return TRUE;
		}

		break;
	}

	return FALSE;
}


///////////////////////////////////////////////////////
//AddComToList
//功能：向链表中添加一个community节点
//参数：将要添加的Community结构体（添加对象链表的头指针.全局变量提供）
//返回：返回更新后链表头指针

BOOL AddComToList(Community newCom, Community **head)
{
	Community *begin, *end;

	//没有楼盘的情况
	if (*head == NULL)
	{
		*head = (Community*)malloc(sizeof(Community));
		**head = newCom;

		//初始化链表指针，重要！
		(**head).nextCommunity = NULL;
		(**head).buildings = NULL;
		return TRUE;
	}

	end = *head, begin = NULL;
	while (end != NULL)
	{
		begin = end;
		end = end->nextCommunity;
	}
	
	begin->nextCommunity =
		(Community*)malloc(sizeof(Community));
	*begin->nextCommunity = newCom;

	//初始化链表指针，重要！！
	(*begin->nextCommunity).nextCommunity = NULL;
	(*begin->nextCommunity).buildings = NULL;
	return TRUE;
}

///////////////////////////////////////////////////////
//AddBuiToList
//功能：向链表中添加一个building节点
//参数：将要添加的Building结构体，添加对象链表的头指针，所属楼盘号
//返回：成功则返回TRUE，失败返回FALSE

BOOL AddBuiToList(Building newBui,Community **head, int comNum)
{
	Building *begBui, *endBui;
	Community *tailCom;

	if (*head == NULL)
		return FALSE;

	for (tailCom = *head; tailCom != NULL
		; tailCom = tailCom->nextCommunity)
	{
		if (tailCom->communityNum == comNum)
		{

			//该楼盘下没有楼栋的情况
			if (tailCom->buildings == NULL)
			{
				tailCom->buildings =
					(Building*)malloc(sizeof(Building));
				*tailCom->buildings = newBui;

				//对于未初始化的结构，这一步初始化很重要！！
				tailCom->buildings->nextBuilding = NULL;
				tailCom->buildings->rooms = NULL;
				return TRUE;
			}

			endBui = tailCom->buildings;
			begBui = NULL;
			while (endBui != NULL)
			{
				begBui = endBui;
				endBui = endBui->nextBuilding;
			}

			begBui->nextBuilding =
				(Building*)malloc(sizeof(Building));
			*begBui->nextBuilding = newBui;

			//初始化链表指针
			begBui->nextBuilding->nextBuilding = NULL;
			begBui->nextBuilding->rooms = NULL;
			return TRUE;
		}
	}
	
	return FALSE;
}


///////////////////////////////////////////////////////
//AddRoomToList
//功能：向链表中添加一个Room节点
//参数：将要添加的Room结构体，添加对象链表的头指针，所属楼盘号，所属楼栋号
//返回：成功则返回TRUE，失败返回FALSE

BOOL AddRoomToList(Room newRoom, Community **head,
	int comNum, int buiNum)
{
	Room *begRoom, *endRoom;
	Community *tailCom;
	Building *tailBui;

	if (*head == NULL)
		return FALSE;

	for (tailCom = *head; tailCom != NULL;
		tailCom = tailCom->nextCommunity)
	{
		if (tailCom->communityNum == comNum)
		{
			//该楼盘下没有楼栋的情况
			if (tailCom->buildings == NULL)
				return FALSE;

			for (tailBui = tailCom->buildings; tailBui != NULL;
				tailBui = tailBui->nextBuilding)
			{
				if (tailBui->buildingNum == buiNum)
				{
					//该楼栋下没有房间的情况
					if (tailBui->rooms == NULL)
					{
						tailBui->rooms =
							(Room*)malloc(sizeof(Room));
						*tailBui->rooms = newRoom;

						//对于未初始化的结构，这一步初始化很重要！！
						tailBui->rooms->nextRoom = NULL;
						return TRUE;
					}

					endRoom = tailBui->rooms;
					begRoom = NULL;
					while (endRoom != NULL)
					{
						begRoom = endRoom;
						endRoom = endRoom->nextRoom;
					}

					begRoom->nextRoom =
						(Room*)malloc(sizeof(Room));
					*begRoom->nextRoom = newRoom;

					//初始化链表指针
					begRoom->nextRoom->nextRoom = NULL;
					return TRUE;
				}
			}			
		}
	}

	return FALSE;
}


///////////////////////////////////////////////////////
//AddBuiToSearch
//功能：创建一个只由bui组成的search
//参数：将要添加的Building结构体，添加对象链表的头指针
//返回：成功则返回TRUE，失败返回FALSE

BOOL AddBuiToSearch(Building newCom, Building **head)
{
	Building *begin, *end;

	//没有楼盘的情况
	if (*head == NULL)
	{
		*head = (Building*)malloc(sizeof(Building));
		**head = newCom;

		//初始化链表指针，重要！
		(**head).nextBuilding = NULL;
		(**head).rooms = NULL;
		return TRUE;
	}

	end = *head, begin = NULL;
	while (end != NULL)
	{
		begin = end;
		end = end->nextBuilding;
	}

	begin->nextBuilding =
		(Building*)malloc(sizeof(Building));
	*begin->nextBuilding = newCom;

	//初始化链表指针，重要！！
	(*begin->nextBuilding).nextBuilding = NULL;
	(*begin->nextBuilding).rooms = NULL;
	return TRUE;
}


///////////////////////////////////////////////////////
//AddRoomToSearch
//功能：创建一个只由bui组成的search
//参数：将要添加的Room结构体，添加对象链表的头指针
//返回：成功则返回TRUE，失败返回FALSE

BOOL AddRoomToSearch(Room newCom, Room **head)
{
	Room *begin, *end;

	//没有楼盘的情况
	if (*head == NULL)
	{
		*head = (Room*)malloc(sizeof(Room));
		**head = newCom;

		//初始化链表指针，重要！
		(**head).nextRoom = NULL;
		return TRUE;
	}

	end = *head, begin = NULL;
	while (end != NULL)
	{
		begin = end;
		end = end->nextRoom;
	}

	begin->nextRoom =
		(Room*)malloc(sizeof(Room));
	*begin->nextRoom = newCom;

	//初始化链表指针，重要！！
	(*begin->nextRoom).nextRoom = NULL;
	return TRUE;
}


///////////////////////////////////////////////////////
//FileInitWnd
//功能：初始化弹出文件窗口的数据
//参数：父窗口句柄hwnd
//返回：无返回值

void FileInitWnd(HWND hwnd)
{
	static TCHAR szFilter[] = 
		TEXT("Leading Files (*.led)\0*.led\0")	\
		TEXT("All Files (*.*)\0*.*\0\0");

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = szFilter;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = NULL;				 //在打开、关闭文件中设置
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL;           //在打开、关闭文件中设置
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = NULL;
	ofn.Flags = 0;						 //在打开、关闭文件中设置
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = TEXT("bin");
	ofn.lCustData = 0L;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;
}


///////////////////////////////////////////////////////
//FileImportDlg
//功能：创建导入文件对话框
//参数：父窗口句柄hwnd，文件名，路径名
//返回：若成功则返回TRUE，失败则返回FALSE

BOOL FileImportDlg(HWND hwnd, PTSTR pstrFileName, PTSTR pstrFileTitle)
{
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = pstrFileName;
	ofn.lpstrFileTitle = pstrFileTitle;
	ofn.Flags = OFN_HIDEREADONLY | OFN_CREATEPROMPT;

	return GetOpenFileName(&ofn);
}


///////////////////////////////////////////////////////
//FileSaveDlg
//功能：创建保存文件对话框
//参数：窗口句柄hwnd，文件名，路径名
//返回：若成功则返回TRUE，失败则返回FALSE

BOOL FileSaveDlg(HWND hwnd, PTSTR pstrFileName, PTSTR pstrFileTitle)
{
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = pstrFileName;
	ofn.lpstrFileTitle = pstrFileTitle;
	ofn.Flags = OFN_OVERWRITEPROMPT;

	return GetSaveFileName(&ofn);
}


///////////////////////////////////////////////////////
//FileReadWnd
//功能：读取文件
//参数：窗口句柄hwnd，路径名
//返回：若成功则返回TRUE，失败则返回FALSE

BOOL FileReadWnd(HWND hwnd, PTSTR pstrLeadFileName)
{
	DWORD dwReadSize;
	HANDLE hFile[3], hLeadFile;
	TCHAR szPrint[50];
	int i;
	int iComNum, iBuiNum, iRoomNum;
	LenData ldData;
	Community *pcTmp;
	Building *pbTmp;
	Room *prTmp;
	TCHAR *szBuffers[] = {
		TEXT("%s - 楼盘.bin"),
		TEXT("%s - 楼栋.bin"),
		TEXT("%s - 房间.bin")
	};
	TCHAR *szPrints[3], *szCompanyName;

	//获取公司名
	szCompanyName = (TCHAR*)malloc(sizeof(TCHAR) * 50);
	lstrcpy(szCompanyName, pstrLeadFileName);
	szCompanyName[lstrlen(szCompanyName) - 4] = '\0\0';

	//编辑数据文件名
	for (i = 0; i < 3; i++)
	{
		szPrints[i] = (TCHAR*)malloc(sizeof(TCHAR) * 50);
		wsprintf(szPrints[i], szBuffers[i], szCompanyName);
	}

	//打开引导文件
	hLeadFile = CreateFile(pstrLeadFileName, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == hLeadFile)
		return FALSE;

	//打开数据文件
	for (i = 0; i < 3; i++)
		hFile[i] = CreateFile(szPrints[i], GENERIC_READ, FILE_SHARE_READ,
			NULL, OPEN_EXISTING, 0, NULL);

	//读取三个文件的大小
	ReadFile(hLeadFile, &ldData, sizeof(LenData), &dwReadSize, NULL);

	//检查读取是否正常
	if ((int)dwReadSize != sizeof(LenData))
	{
		for (i = 0; i < 3; i++)
			CloseHandle(hFile[i]);
		CloseHandle(hLeadFile);
		return FALSE;
	}

	//复制数据
	iComNum = ldData.iCom / sizeof(Community);
	iBuiNum = ldData.iBui / sizeof(Building);
	iRoomNum = ldData.iRoom / sizeof(Room);

	//确定编号开始
	iAutoComNum = iComNum + 1;

	//分配空间
	pcTmp = (Community*)malloc(sizeof(Community));
	pbTmp = (Building*)malloc(sizeof(Building));
	prTmp = (Room*)malloc(sizeof(Room));

	pHead = NULL;
	//读取三个文件并写入链表
	while (iComNum > 0)
	{
		iComNum--;
		ReadFile(hFile[0], pcTmp, sizeof(Community), &dwReadSize, NULL);
		SetFilePointer(hFile[0], sizeof(Community), NULL, FILE_CURRENT);

		//检验是否全部写入成功
		if ((int)dwReadSize != sizeof(Community))
		{
			CloseHandle(hFile[0]);
			return FALSE;
		}
		//加入链表
		AddComToList(*pcTmp, &pHead);

		while (iBuiNum > 0)
		{
			iBuiNum--;
			ReadFile(hFile[1], pbTmp, sizeof(Building), &dwReadSize, NULL);
			SetFilePointer(hFile[1], sizeof(Building), NULL, FILE_CURRENT);

			//检验是否全部写入成功
			if ((int)dwReadSize != sizeof(Building))
			{
				CloseHandle(hFile[1]);
				return FALSE;
			}

			//检验是否遇到楼栋分隔点
			if (pbTmp->buildingNum == -1)
			{
				iBuiNum++;
				break;
			}

			//将楼栋加入链表
			AddBuiToList(*pbTmp, &pHead, pcTmp->communityNum);

			while (iRoomNum > 0)
			{
				iRoomNum--;
				ReadFile(hFile[2], prTmp, sizeof(Room), &dwReadSize, NULL);
				SetFilePointer(hFile[2], sizeof(Room), NULL, FILE_CURRENT);

				//检验是否全部写入成功
				if ((int)dwReadSize != sizeof(Room))
				{
					CloseHandle(hFile[2]);
					return FALSE;
				}

				//检验是否遇到房间分隔点
				if (prTmp->roomNum == -1)
				{
					iRoomNum++;
					break;
				}

				//将房间加入链表
				AddRoomToList(*prTmp, &pHead,
					pcTmp->communityNum, pbTmp->buildingNum);
			}
		}
	}

	//检验数据是否全部读取
	if (iRoomNum || iBuiNum || iComNum)
	{
		for (i = 0; i < 3; i++)
			CloseHandle(hFile[i]);
		CloseHandle(hLeadFile);
		return FALSE;
	}

	//设置主窗口标题内容
	wsprintf(szPrint, TEXT("楼盘管理系统 - %s"), szCompanyName);
	SetWindowText(hwnd, (PTSTR)szPrint);

	//回收句柄和指针
	CloseHandle(hLeadFile);
	for (i = 0; i < 3; i++)
		CloseHandle(hFile[i]);
	free(pcTmp);
	free(pbTmp);
	free(prTmp);

	return TRUE;
}


///////////////////////////////////////////////////////
//FileWriteWnd
//功能：写入文件
//参数：窗口句柄hwnd，引导文件文件名
//返回：若成功则返回TRUE，失败则返回FALSE

BOOL FileWriteWnd(HWND hwnd, PTSTR pstrLeadFileName)
{
	DWORD  dwWriteSize;
	HANDLE hFile[3], hLeadFile;
	int    i,iComLen = 0, iBuiLen = 0, iRoomLen = 0;
	Community *pTailCom;
	Building *pTailBui, *pNullBui;
	Room *pTailRoom, *pNullRoom;
	WORD   wMark = 0xFEFF;
	LenData	Data;
	TCHAR *szCompanyName;
	TCHAR *szBuffers[] = {
		TEXT("%s - 楼盘.bin"),
		TEXT("%s - 楼栋.bin"),
		TEXT("%s - 房间.bin")
	};
	TCHAR *szPrints[3];

	//获取公司名
	szCompanyName = (TCHAR*)malloc(sizeof(TCHAR) * 50);
	lstrcpy(szCompanyName, pstrLeadFileName);
	szCompanyName[lstrlen(szCompanyName) - 4] = '\0\0';

	//编辑数据文件名
	for (i = 0; i < 3; i++)
	{
		szPrints[i] = (TCHAR*)malloc(sizeof(TCHAR) * 50);
		wsprintf(szPrints[i], szBuffers[i], szCompanyName);
	}

	//打开文件
	for (i = 0; i < 3; i++)
		hFile[i] = CreateFile(szPrints[i], GENERIC_WRITE, 0,
			NULL, CREATE_ALWAYS, 0, NULL);
	hLeadFile = CreateFile(pstrLeadFileName, GENERIC_WRITE, 0,
		NULL, CREATE_ALWAYS, 0, NULL);

	if (INVALID_HANDLE_VALUE == hLeadFile)
		return FALSE;

	//编辑bui与room结构在文件中的分隔符

	pNullBui = (Building*)malloc(sizeof(Building));
	pNullRoom = (Room*)malloc(sizeof(Room));
	pNullBui->buildingNum = -1;
	pNullRoom->roomNum = -1;

	//获取所有数据并写入三个文件中

	for (pTailCom = pHead; pTailCom;
		pTailCom = pTailCom->nextCommunity)
	{
		iComLen += sizeof(Community);
		WriteFile(hFile[0], pTailCom, sizeof(Community), &dwWriteSize, NULL);
		SetFilePointer(hFile[0], sizeof(Community), NULL, FILE_CURRENT);

		//检验是否全部写入成功
		if ((int)dwWriteSize != sizeof(Community))
		{
			CloseHandle(hFile[0]);
			return FALSE;
		}

		for (pTailBui = pTailCom->buildings; pTailBui;
			pTailBui = pTailBui->nextBuilding)
		{
			iBuiLen += sizeof(Building);
			WriteFile(hFile[1], pTailBui, sizeof(Building), &dwWriteSize, NULL);
			SetFilePointer(hFile[1], sizeof(Building), NULL, FILE_CURRENT);
			
			//检验是否全部写入成功
			if ((int)dwWriteSize != sizeof(Building))
			{
				CloseHandle(hFile[1]);
				return FALSE;
			}

			for (pTailRoom = pTailBui->rooms; pTailRoom;
				pTailRoom = pTailRoom->nextRoom)
			{
				iRoomLen += sizeof(Room);
				WriteFile(hFile[2], pTailRoom, sizeof(Room), &dwWriteSize, NULL);
				SetFilePointer(hFile[2], sizeof(Room), NULL, FILE_CURRENT);

				//检验是否全部写入成功
				if ((int)dwWriteSize != sizeof(Room))
				{
					CloseHandle(hFile[2]);
					return FALSE;
				}
			}

			//设置Room分隔符,并检验是否成功
			WriteFile(hFile[2], pNullRoom, sizeof(Room), &dwWriteSize, NULL);
			SetFilePointer(hFile[2], sizeof(Room), NULL, FILE_CURRENT);
			if ((int)dwWriteSize != sizeof(Room))
			{
				CloseHandle(hFile[2]);
				return FALSE;
			}
		}

		//设置Building分隔符,并检验是否成功
		WriteFile(hFile[1], pNullBui, sizeof(Building), &dwWriteSize, NULL);
		SetFilePointer(hFile[1], sizeof(Building), NULL, FILE_CURRENT);
		if ((int)dwWriteSize != sizeof(Building))
		{
			CloseHandle(hFile[1]);
			return FALSE;
		}
	}

	//将三个文件的数据大小写入导入文件中
	Data.iCom = iComLen;
	Data.iBui = iBuiLen;
	Data.iRoom = iRoomLen;
	WriteFile(hLeadFile, &Data, sizeof(LenData), &dwWriteSize, NULL);
	if ((int)dwWriteSize != sizeof(LenData))
	{
		for (i = 0; i < 3; i++)
			CloseHandle(hFile[i]);
		return FALSE;
	}

	//检验文件是否为空
	if (!pcBuffer && !pbBuffer && !pcBuffer)
	{
		for (i = 0; i < 3; i++)
			CloseHandle(hFile[i]);
		return FALSE;
	}

	for (i = 0; i < 3; i++)
		CloseHandle(hFile[i]);
	CloseHandle(hLeadFile);

	return TRUE;
}


///////////////////////////////////////////////////////
//SetTitle
//功能：设置窗口内容
//参数：编辑框句柄hwnd，文件名
//返回：无返回值

void SetTitle(HWND hwnd, TCHAR * szTitleName)
{
	TCHAR szCaption[64 + MAX_PATH];

	wsprintf(szCaption, TEXT("%s - %s"), TEXT("楼盘管理系统"),
		szTitleName[0] ? szTitleName : TEXT("未定义"));

	SetWindowText(hwnd, szCaption);
}


///////////////////////////////////////////////////////
//DrawComInf
//功能：绘制楼盘信息窗口
//参数：编辑框句柄hwnd，设备环境hdc，Community结构
//返回：成功返回TRUE，失败返回FALSE

BOOL DrawComInf(HWND hwnd, HDC hdc, Community com)
{
	int cxChar, cyChar, cxClient, cyClient;
	TEXTMETRIC tm;
	TCHAR szBuffer[100];
	RECT rcClient;

	GetTextMetrics(hdc, &tm);
	cxChar = tm.tmAveCharWidth;
	cyChar = tm.tmHeight + tm.tmExternalLeading;
	GetClientRect(hwnd, &rcClient);
	cxClient = rcClient.right;
	cyClient = rcClient.bottom;

	SelectObject(hdc, hFontSon);
	SetBkMode(hdc, TRANSPARENT);

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar,
		szBuffer, wsprintf(szBuffer, TEXT("楼盘名称：%s"), com.name));

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar + cyChar * 1.25, 
		szBuffer, wsprintf(szBuffer, TEXT("楼盘编号：%d"), com.communityNum));

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar + 2 * cyChar * 1.25,
		szBuffer, wsprintf(szBuffer, TEXT("楼栋地址：%s"), com.address));

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar + 3 * cyChar * 1.25,
		szBuffer, wsprintf(szBuffer, TEXT("楼栋数量：%d"), com.numberOfBuildings));

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar + 4 * cyChar * 1.25,
		szBuffer, wsprintf(szBuffer, TEXT("房间数量：%d"), com.numberOfRooms));

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar + 5 * cyChar * 1.25,
		szBuffer, swprintf(szBuffer, TEXT("平均价格：%.2f"), com.avgPrice));

	TextOut(hdc, 2.7 * XLINEPOS, 0.5*cyChar,
		szBuffer, wsprintf(szBuffer, TEXT("联系人  ：%s"), com.host.name));

	TextOut(hdc, 2.7 * XLINEPOS, 0.5*cyChar + cyChar * 1.25,
		szBuffer, wsprintf(szBuffer, TEXT("联系电话：%s"), com.phone));

	DrawButton(hdc, cxClient, YLINEPOS);

	return TRUE;
}


///////////////////////////////////////////////////////
//DrawBuiInf
//功能：绘制楼栋信息窗口
//参数：编辑框句柄hwnd，设备环境hdc，Building结构
//返回：成功返回TRUE，失败返回FALSE

BOOL DrawBuiInf(HWND hwnd, HDC hdc, Building bui)
{
	int cxChar, cyChar, cxClient, cyClient;
	TEXTMETRIC tm;
	TCHAR szBuffer[100];
	RECT rcClient;

	GetTextMetrics(hdc, &tm);
	cxChar = tm.tmAveCharWidth;
	cyChar = tm.tmHeight + tm.tmExternalLeading;
	GetClientRect(hwnd, &rcClient);
	cxClient = rcClient.right;
	cyClient = rcClient.bottom;

	SelectObject(hdc, hFontSon);
	SetBkMode(hdc, TRANSPARENT);

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar,
		szBuffer, wsprintf(szBuffer, TEXT("楼栋编号：%d"), bui.buildingNum));

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar + cyChar * 1.25,
		szBuffer, wsprintf(szBuffer, TEXT("所在楼盘：%s"), bui.inCom));

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar + 2 * cyChar * 1.25,
		szBuffer, wsprintf(szBuffer, TEXT("楼层数   ：%d"), bui.numberOfFloors));

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar + 3 * cyChar * 1.25,
		szBuffer, wsprintf(szBuffer, TEXT("房间数   ：%d"), bui.numberOfRooms));

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar + 4 * cyChar * 1.25,
		szBuffer, swprintf(szBuffer, TEXT("平均价格：%.2f"), bui.avgPrice));

	TextOut(hdc, 2.7 * XLINEPOS, 0.5*cyChar,
		szBuffer, wsprintf(szBuffer, TEXT("联系人   ：%s"), bui.host.name));

	TextOut(hdc, 2.7 * XLINEPOS, 0.5*cyChar + cyChar * 1.25,
		szBuffer, wsprintf(szBuffer, TEXT("联系电话：%s"), bui.host.phone));

	DrawButton(hdc, cxClient, YLINEPOS);

	return TRUE;
}


///////////////////////////////////////////////////////
//DrawRoomInf
//功能：绘制楼栋信息窗口
//参数：编辑框句柄hwnd，设备环境hdc，Room结构
//返回：成功返回TRUE，失败返回FALSE

BOOL DrawRoomInf(HWND hwnd, HDC hdc, Room room)
{
	int cxChar, cyChar, cxClient, cyClient;
	TEXTMETRIC tm;
	TCHAR szBuffer[100], szPrint[100];
	RECT rcClient;

	GetTextMetrics(hdc, &tm);
	cxChar = tm.tmAveCharWidth;
	cyChar = tm.tmHeight + tm.tmExternalLeading;
	GetClientRect(hwnd, &rcClient);
	cxClient = rcClient.right;
	cyClient = rcClient.bottom;

	SelectObject(hdc, hFontSon);
	SetBkMode(hdc, TRANSPARENT);

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar,
		szBuffer, wsprintf(szBuffer, TEXT("房间号   ：%d"), room.roomNum));

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar + cyChar * 1.25,
		szBuffer, wsprintf(szBuffer, TEXT("所在楼盘：%s"), room.inCom));

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar + 2 * cyChar * 1.25,
		szBuffer, wsprintf(szBuffer, TEXT("所在楼栋：%d"), room.buildingNum));

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar + 3 * cyChar * 1.25,
		szBuffer, wsprintf(szBuffer, TEXT("所在楼层：%d"), room.floor));

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar + 4 * cyChar * 1.25,
		szBuffer, swprintf(szBuffer, TEXT("房间价格：%.2f 元 / 平方米"), room.roomPrice));

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar + 5 * cyChar * 1.25,
		szBuffer, swprintf(szBuffer, TEXT("房间总价：%d 元"), room.allPrice));

	TextOut(hdc, 2.5 * XLINEPOS, 0.5*cyChar,
		szBuffer, wsprintf(szBuffer, TEXT("联系人   ：%s"), room.host.name));

	TextOut(hdc, 2.5 * XLINEPOS, 0.5*cyChar + cyChar * 1.25,
		szBuffer, wsprintf(szBuffer, TEXT("联系电话：%s"), room.host.phone));

	TextOut(hdc, 2.5 * XLINEPOS, 0.5*cyChar + 2 * cyChar * 1.25,
		szBuffer, wsprintf(szBuffer, TEXT("%s"), room.roomType));

	if (room.roomState)
		lstrcpy(szBuffer, TEXT("已售出"));
	else lstrcpy(szBuffer, TEXT("未售出"));
	TextOut(hdc, 2.5 * XLINEPOS, 0.5*cyChar + 3 * cyChar * 1.25,
		szPrint, wsprintf(szPrint, TEXT("售出情况：%s"), szBuffer));

	DrawButton(hdc, cxClient, YLINEPOS);
	roomNow = room.roomNum;

	return TRUE;
}


///////////////////////////////////////////////////////
//RenewData
//功能：将数据区恢复为背景色
//参数：设备环境hdc，Building结构
//返回：无返回

void RenewData(HDC hdc, RECT rectData)
{
	SelectObject(hdc, GetStockObject(DC_BRUSH));
	SetDCBrushColor(hdc, RGB(137, 189, 255));   //该颜色为天蓝色

	Rectangle(hdc, rectData.left, rectData.top,
		rectData.right, rectData.bottom);

	SelectObject(hdc, GetStockObject(BLACK_BRUSH));
}


///////////////////////////////////////////////////////
//UpdataData
//功能：更新整个链表的数据，包括房间数，均价等
//参数：将要更新的链表的头指针的地址
//返回：若完成所有更新则返回TRUE，否则返回FALSE

BOOL UpdateData(Community **ppHead)
{
	Community *pCom;
	Building *pBui;
	Room *pRoom;
	float avgComPrice = 0.0, avgBuiPrice = 0.0;
	int comRoomNum = 0, buiRoomNum = 0, comBuiNum = 0;
	float sumBuiPrice = 0.0, sumComPrice = 0.0;

	for (pCom = *ppHead; pCom; pCom = pCom->nextCommunity)
	{
		//初始化楼盘价格和房间数
		sumComPrice = 0.0;
		comRoomNum = 0;
		comBuiNum = 0;

		for (pBui = pCom->buildings; pBui; pBui = pBui->nextBuilding)
		{
			//初始化楼栋价格和房间数、楼栋数
			sumBuiPrice = 0.0;
			buiRoomNum = 0;

			lstrcpy(pBui->inCom, pCom->name);

			for (pRoom = pBui->rooms; pRoom; pRoom = pRoom->nextRoom)
			{
				pRoom->buildingNum = pBui->buildingNum;

				//计算房间总价
				pRoom->allPrice = pRoom->roomSize * pRoom->roomPrice;
				pRoom->floor = pRoom->roomNum / 100;	//计算楼层数
				pRoom->host = pBui->host;	//房间联系人与楼栋一致
				
				//加入楼栋总价格和总房间数
				sumBuiPrice += pRoom->roomPrice;
				buiRoomNum++;
			}

			//将数据写入链表
			pBui->numberOfRooms = buiRoomNum;
			if(buiRoomNum)
				pBui->avgPrice = sumBuiPrice / buiRoomNum;
			else pBui->avgPrice = 0.0;

			//加入楼盘总价格和楼栋数
			sumComPrice += sumBuiPrice;
			comRoomNum += buiRoomNum;
			comBuiNum++;
		}
		
		//将数据写入链表
		pCom->numberOfRooms = comRoomNum;
		if (comRoomNum != 0)
			pCom->avgPrice = sumComPrice / comRoomNum;
		else  pCom->avgPrice = 0.0;
		pCom->numberOfBuildings = comBuiNum;
	}

	return TRUE;
}


///////////////////////////////////////////////////////
//Draw1Step
//功能：WM_PAINT消息中绘制第一级(包含Com与Bui)，不显示数据
//参数：设备环境hdc，显示数据的com指针（NULL表示无楼盘）
//返回：无返回

void Draw1Step(HDC hdc, Community *pChosenCom)
{
	HINSTANCE hInstance;
	Community *pCom;
	Building *pBui;
	TCHAR szPrint[100];
	TCHAR *pszSubTitle[] = {
		TEXT("楼栋号"),
		TEXT("所在楼盘"),
		TEXT("平均价格"),
		TEXT("联系电话")
	};
	int piColWid[] = { 80,250,170,115 };
	int iCol;
	LVCOLUMN lvc;
	LVITEM lvi;

	hInstance = (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);

	lvc.mask = LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
	
	//设置字体和背景
	SendMessage(hwndSubList, WM_SETFONT, (WPARAM)hFontSon, TRUE);
	SetWindowText(hwndMainTitle, TEXT("楼 盘"));

	//添加listview的4列
	for (iCol = 0; iCol < 4; iCol++)
	{
		lvc.iSubItem = iCol;
		lvc.pszText = pszSubTitle[iCol];
		lvc.cx = piColWid[iCol];
		lvc.fmt = LVCFMT_CENTER;

		ListView_DeleteColumn(hwndSubList, iCol);
		ListView_InsertColumn(hwndSubList, iCol, &lvc);
	}

	//重置主菜单数据
	SendMessage(hwndMainList, LB_RESETCONTENT, 0, 0);
	ListView_DeleteAllItems(hwndSubList);

	/**************************************************/
	/*
	col1      col2     col3      col4
	item0   subitem1  subitem2  subitem3
	item1   subitem1  subitem2  subitem3
	item2   subitem1  subitem2  subitem3
	......(item2 == subitem0)
	*/

	//空表示目前无楼盘，直接退出
	if (!pChosenCom)
		return;

	//填充表格
	lvi.mask = LVIF_TEXT | LVIF_STATE;
	lvi.iSubItem = 0;	//初始化行数
	lvi.pszText = LPSTR_TEXTCALLBACK;
	lvi.state = 0;
	lvi.stateMask = 0;
	lvi.iItem = 0;

	for (pCom = pHead; pCom; pCom = pCom->nextCommunity)
	{
		SendMessage(hwndMainList, LB_ADDSTRING, 0, (LPARAM)pCom->name);
		if (pCom->communityNum == pChosenCom->communityNum)
			for (pBui = pCom->buildings; pBui; pBui = pBui->nextBuilding)
			{
				//添加一行表格
				ListView_InsertItem(hwndSubList, &lvi);

				//像一行添加文本
				wsprintf(szPrint, TEXT("%d"), pBui->buildingNum);
				ListView_SetItemText(hwndSubList, lvi.iItem, 0, szPrint);
				wsprintf(szPrint, TEXT("%s"), pBui->inCom);
				ListView_SetItemText(hwndSubList, lvi.iItem, 1, szPrint);
				swprintf(szPrint, TEXT("%.2f"), pBui->avgPrice);
				ListView_SetItemText(hwndSubList, lvi.iItem, 2, szPrint);
				wsprintf(szPrint, TEXT("%s"), pBui->host.phone);
				ListView_SetItemText(hwndSubList, lvi.iItem, 3, szPrint);

				lvi.iItem++;
			}
	}
}

///////////////////////////////////////////////////////
//Draw2Step
//功能：WM_PAINT消息中绘制第二级，不显示数据
//参数：设备环境hdc,指向所选楼栋的指针
//返回：无返回

void Draw2Step(HDC hdc,Building *pBui)  
{
	Room *pRoom;
	Community *pCom;
	HINSTANCE hInstance;
	TCHAR szPrint[100];
	TCHAR *pszSubTitle[] = {
		TEXT("房间号"),
		TEXT("所在层数"),
		TEXT("房间价格"),
		TEXT("房间面积")
	};
	int piColWid[] = { 80,250,170,115 };
	int iCol;
	LVCOLUMN lvc;
	LVITEM lvi;

	hInstance = (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);

	lvc.mask = LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;

	//设置字体和背景
	SendMessage(hwndSubList, WM_SETFONT, (WPARAM)hFontSon, TRUE);
	SetWindowText(hwndMainTitle, TEXT("楼 盘"));

	SendMessage(hwndMainList, LB_RESETCONTENT, 0, 0);
	for (pCom = pHead; pCom; pCom = pCom->nextCommunity)
		SendMessage(hwndMainList, LB_ADDSTRING, 0, (LPARAM)pCom->name);

	//添加listview的4列
	for (iCol = 0; iCol < 4; iCol++)
	{
		lvc.iSubItem = iCol;
		lvc.pszText = pszSubTitle[iCol];
		lvc.cx = piColWid[iCol];
		lvc.fmt = LVCFMT_CENTER;

		ListView_DeleteColumn(hwndSubList, iCol);
		ListView_InsertColumn(hwndSubList, iCol, &lvc);
	}

	//重置主菜单数据
	ListView_DeleteAllItems(hwndSubList);

	if (!pBui)
		return;

	//填充表格
	lvi.mask = LVIF_TEXT | LVIF_STATE;
	lvi.iSubItem = 0;	//初始化行数
	lvi.pszText = LPSTR_TEXTCALLBACK;
	lvi.state = 0;
	lvi.stateMask = 0;
	lvi.iItem = 0;

	//更新子菜单

		//第一栏为该楼栋，以供查看楼栋信息和返回上一级
	wsprintf(szPrint, TEXT("%s，%d栋"), pBui->inCom, pBui->buildingNum);
	
		//添加一行表格
	ListView_InsertItem(hwndSubList, &lvi);
	ListView_SetItemText(hwndSubList, lvi.iItem, 1, szPrint);
	lvi.iItem++;

		//之后为房间基本信息
	for (pRoom = pBui->rooms; pRoom; pRoom = pRoom->nextRoom)
	{
		//添加一行表格
		ListView_InsertItem(hwndSubList, &lvi);

		//像一行添加文本
		wsprintf(szPrint, TEXT("%d"), pRoom->roomNum);
		ListView_SetItemText(hwndSubList, lvi.iItem, 0, szPrint);
		wsprintf(szPrint, TEXT("%d"), pRoom->floor);
		ListView_SetItemText(hwndSubList, lvi.iItem, 1, szPrint);
		swprintf(szPrint, TEXT("%.2f"), pRoom->roomPrice);
		ListView_SetItemText(hwndSubList, lvi.iItem, 2, szPrint);
		swprintf(szPrint, TEXT("%.2f"), pRoom->roomSize);
		ListView_SetItemText(hwndSubList, lvi.iItem, 3, szPrint);

		lvi.iItem++;	
	}
}


///////////////////////////////////////////////////////
//DelComInList
//功能：删除链表中的一个Community节点
//参数：将要删除的Community编号，添加对象链表的头指针的地址
//返回：返回更新后链表头指针

BOOL DelComInList(int comNum, Community **head)
{
	Community *comTail, *comLastTail;

	//没有楼盘的情况
	if (*head == NULL)
		return FALSE;

	//只有一个节点的情况
	if ((*head)->nextCommunity == NULL)
	{
		//检查第一个节点是不是所求值
		if ((*head)->communityNum == comNum)
		{
			//只有一个节点，直接指向NULL
			*head = NULL;
			return TRUE;
		}
		else return FALSE;
	}

	//检查第一个节点是不是所求值
	if ((*head)->communityNum == comNum)
	{
		//有多个节点，连到下一个
		*head = (*head)->nextCommunity;
		return TRUE;
	}

	comTail = (*head)->nextCommunity;
	comLastTail = *head;

	//循环删除该楼盘
	while (comTail != NULL)
	{
		if (comTail->communityNum == comNum)
		{
			comLastTail->nextCommunity = comTail->nextCommunity;
			return TRUE;
		}
		
		comTail = comTail->nextCommunity;
		comLastTail = comLastTail->nextCommunity;
	}
	
	return FALSE;
}


///////////////////////////////////////////////////////
//DelBuiInList
//功能：删除链表中的一个Building节点
//参数：将要删除的Building编号，添加对象链表的头指针的地址
//返回：返回更新后链表头指针

BOOL DelBuiInList(int comNum, int buiNum, Community **head)

{
	Community *comTail;
	Building *buiTail, *buiLastTail;

	//没有楼盘的情况
	if (*head == NULL)
		return FALSE;

	for (comTail = *head; comTail != NULL;
		comTail = comTail->nextCommunity)
	{
		if (comNum == comTail->communityNum)
		{
			//没有楼栋的情况
			if (comTail->buildings == NULL)
				return FALSE;

			//只有一个节点的情况
			if (comTail->buildings->nextBuilding == NULL)
			{
				//检查第一个节点是不是所求值
				if (comTail->buildings->buildingNum == buiNum)
				{
					//只有一个节点，直接指向NULL
					comTail->buildings = NULL;
					return TRUE;
				}
				else return FALSE;
			}

			//检查第一个节点是不是所求值
			if (comTail->buildings->buildingNum == buiNum)
			{
				//指向下一个节点
				comTail->buildings = comTail->buildings->nextBuilding;
				return TRUE;
			}

			//初始化两个指针
			buiLastTail = comTail->buildings;
			buiTail = comTail->buildings->nextBuilding;

			//循环删除
			while (buiTail != NULL)
			{
				if (buiTail->buildingNum == buiNum)
				{
					buiLastTail->nextBuilding = buiTail->nextBuilding;
					return TRUE;
				}
				buiLastTail = buiLastTail->nextBuilding;
				buiTail = buiTail->nextBuilding;
			}
		}
	}
	return FALSE;
}

///////////////////////////////////////////////////////
//DelRoomInList
//功能：删除链表中的一个Room节点
//参数：将要删除的Room编号，添加对象链表的头指针的地址
//返回：返回更新后链表头指针

BOOL DelRoomInList(int comNum, int buiNum, int roomNum, Community **head)
{
	Community *comTail;
	Building *buiTail;
	Room *roomTail, *roomLastTail;

	//没有楼盘的情况
	if (*head == NULL)
		return FALSE;

	for (comTail = *head; comTail != NULL;
		comTail = comTail->nextCommunity)
	{
		if (comNum == comTail->communityNum)
		{
			//没有楼栋的情况
			if (comTail->buildings == NULL)
				return FALSE;

			for (buiTail = comTail->buildings; buiTail != NULL;
				buiTail = buiTail->nextBuilding)
			{
				if (buiTail->buildingNum == buiNum)
				{
					//没有房间的情况
					if (buiTail->rooms == NULL)
						return FALSE;

					//只有一个节点的情况
					if (buiTail->rooms->nextRoom == NULL)
					{
						//检查第一个节点是不是所求值
						if (buiTail->rooms->roomNum == roomNum)
						{
							//只有一个节点，直接指向NULL
							buiTail->rooms = NULL;
							return TRUE;
						}
						else return FALSE;
					}

					//检查第一个节点是不是所求值
					if (buiTail->rooms->roomNum == roomNum)
					{
						//指向下一个
						buiTail->rooms = buiTail->rooms->nextRoom;
						return TRUE;
					}

					//初始化两个指针
					roomLastTail = buiTail->rooms;
					roomTail = buiTail->rooms->nextRoom;

					//循环删除
					while (roomTail != NULL)
					{
						if (roomTail->roomNum == roomNum)
						{
							roomLastTail->nextRoom = roomTail->nextRoom;
							return TRUE;
						}
						roomLastTail = roomLastTail->nextRoom;
						roomTail = roomTail->nextRoom;
					}
				}
			}		
		}
	}
	return FALSE;
}


///////////////////////////////////////////////////////
//“删除楼盘”（DeleteCommunity）对话框窗口过程

BOOL CALLBACK DelComDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	TCHAR szBuffer[100][100];
	Community *comTail;
	int iComDelNum = -1;
	static int iStateEdit;	//1表示输入的是名字，2表示输入的是楼盘号
	int iLength[5];

	switch (message)
	{
	case WM_INITDIALOG:

		SendMessage(GetDlgItem(hDlg, IDC_RADIO3), 
			BM_SETCHECK, BST_CHECKED, 0);
		//Disable第二个编辑框
		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNUM), FALSE);
		iStateEdit = 1;
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_RADIO3:

			if (HIWORD(wParam) == BN_CLICKED)
			{
				//交换enable的编辑框
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNAME), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNUM), FALSE);
				SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNUM), TEXT(""));
				iStateEdit = 1;
			}
			break;

		case IDC_RADIO4:

			if (HIWORD(wParam) == BN_CLICKED)
			{
				//交换enable的编辑框
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNUM), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNAME), FALSE);
				SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNAME), TEXT(""));
				iStateEdit = 2;
			}
			break;

		case IDOK:

			//当按下OK时，将所输入内容存进存储区	

			if (iStateEdit == 1)
				iLength[0] = SendMessage(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNAME),
					EM_GETLINE, 0, (LPARAM)szBuffer[0]);
			else if (iStateEdit == 2)
				iLength[0] = SendMessage(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNUM),
					EM_GETLINE, 1, (LPARAM)szBuffer[0]);

			//检验输入非空

			if (!iLength[0])
			{
				MessageBox(hDlg, TEXT("请输入完整的信息！"), TEXT("提示"), MB_OK);
				return FALSE;
			}
			else *(szBuffer[0] + iLength[0]) = '\0\0';
			
			//存储或寻找需要删除的楼栋号

			if (iStateEdit == 1)
			{
				comTail = pHead;

				//循环查找楼盘名
				while (comTail != NULL)
				{
					if (!lstrcmp(comTail->name, szBuffer[0]))
					{
						iComDelNum = comTail->communityNum;
						break;
					}
					comTail = comTail->nextCommunity;
				}
			}
			else if (iStateEdit == 2)
			{
				iComDelNum = _ttoi(szBuffer[0]);
			}

			//检查楼盘名是否正确
			if (iComDelNum == -1)
			{
				MessageBox(hDlg, TEXT("该楼盘名称不存在！"), TEXT("提示"), MB_OK);
				return FALSE;
			}

			//删除，并检验楼盘号
			if (!DelComInList(iComDelNum, &pHead))
			{
				MessageBox(hDlg, TEXT("该楼盘号不存在！"), TEXT("提示"), MB_OK);
				return FALSE;
			}

			bRePaint = TRUE;
			EndDialog(hDlg, TRUE);
			return TRUE;

		case IDCANCEL:
			bRePaint = FALSE;
			EndDialog(hDlg, FALSE);
			return TRUE;
		}
		break;
	}

	return FALSE;
}


///////////////////////////////////////////////////////
//“删除楼盘”（DeleteBui）对话框窗口过程

BOOL CALLBACK DelBuiDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	TCHAR szBuffer[100][100];
	Community *comTail;
	int i, iComDelNum = -1, iBuiDelNum = -1;
	static int iStateEdit;	//1表示输入的是名字，2表示输入的是楼盘号
	int iLength[5];

	switch (message)
	{
	case WM_INITDIALOG:

		SendMessage(GetDlgItem(hDlg, IDC_RADIO3),
			BM_SETCHECK, BST_CHECKED, 0);
		//Disable第二个编辑框
		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNUM), FALSE);
		iStateEdit = 1;
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_RADIO3:

			if (HIWORD(wParam) == BN_CLICKED)
			{
				//交换enable的编辑框
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNAME), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNUM), FALSE);
				SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNUM), TEXT(""));
				iStateEdit = 1;
			}
			break;

		case IDC_RADIO4:

			if (HIWORD(wParam) == BN_CLICKED)
			{
				//交换enable的编辑框
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNUM), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNAME), FALSE);
				SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNAME), TEXT(""));
				iStateEdit = 2;
			}
			break;

		case IDOK:

			//当按下OK时，将所输入内容存进存储区	

			if (iStateEdit == 1)
				iLength[0] = SendMessage(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNAME),
					EM_GETLINE, 0, (LPARAM)szBuffer[0]);
			else if (iStateEdit == 2)
				iLength[0] = SendMessage(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNUM),
					EM_GETLINE, 1, (LPARAM)szBuffer[0]);
			
			iLength[1] = SendMessage(GetDlgItem(hDlg, IDC_EDIT_DEL_BUINUM),
				EM_GETLINE, 3, (LPARAM)szBuffer[1]);

			//检验每一项非空

			for (i = 0; i < 2; i++)
			{
				if (!iLength[i])
				{
					MessageBox(hDlg, TEXT("请输入完整的信息！"), TEXT("提示"), MB_OK);
					return FALSE;
				}
				else {
					*(szBuffer[i] + iLength[i]) = '\0\0';
				}
			}

			//存储或寻找需要删除的楼栋号

			if (iStateEdit == 1)
			{
				comTail = pHead;

				//循环查找楼盘名
				while (comTail != NULL)
				{
					if (!lstrcmp(comTail->name, szBuffer[0]))
					{
						iComDelNum = comTail->communityNum;
						break;
					}
					comTail = comTail->nextCommunity;
				}
			}
			else if (iStateEdit == 2)
			{
				iComDelNum = _ttoi(szBuffer[0]);
			}
			iBuiDelNum = _ttoi(szBuffer[1]);

			//检查楼盘名是否正确
			if (iComDelNum == -1)
			{
				MessageBox(hDlg, TEXT("该楼盘名称不存在！"), TEXT("提示"), MB_OK);
				return FALSE;
			}

			//删除，并检验楼栋号
			if (!DelBuiInList(iComDelNum, iBuiDelNum, &pHead))
			{
				MessageBox(hDlg, TEXT("找不到该楼栋！"), TEXT("提示"), MB_OK);
				return FALSE;
			}

			bRePaint = TRUE;
			EndDialog(hDlg, TRUE);
			return TRUE;

		case IDCANCEL:
			bRePaint = FALSE;
			EndDialog(hDlg, FALSE);
			return TRUE;
		}
		break;
	}

	return FALSE;
}


///////////////////////////////////////////////////////
//“删除楼盘”（DeleteRoom）对话框窗口过程

BOOL CALLBACK DelRoomDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	TCHAR szBuffer[100][100];
	Community *comTail;
	int i;
	int iComDelNum = -1, iBuiDelNum = -1, iRoomDelNum = -1;
	static int iStateEdit;	//1表示输入的是名字，2表示输入的是楼盘号
	int iLength[5];

	switch (message)
	{
	case WM_INITDIALOG:

		SendMessage(GetDlgItem(hDlg, IDC_RADIO3),
			BM_SETCHECK, BST_CHECKED, 0);
		//Disable第二个编辑框
		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNUM), FALSE);
		iStateEdit = 1;
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_RADIO3:

			if (HIWORD(wParam) == BN_CLICKED)
			{
				//交换enable的编辑框
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNAME), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNUM), FALSE);
				SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNUM), TEXT(""));
				iStateEdit = 1;
			}
			break;

		case IDC_RADIO4:

			if (HIWORD(wParam) == BN_CLICKED)
			{
				//交换enable的编辑框
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNUM), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNAME), FALSE);
				SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNAME), TEXT(""));
				iStateEdit = 2;
			}
			break;

		case IDOK:

			//当按下OK时，将所输入内容存进存储区	

			if (iStateEdit == 1)
				iLength[0] = SendMessage(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNAME),
					EM_GETLINE, 0, (LPARAM)szBuffer[0]);
			else if (iStateEdit == 2)
				iLength[0] = SendMessage(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNUM),
					EM_GETLINE, 1, (LPARAM)szBuffer[0]);

			iLength[1] = SendMessage(GetDlgItem(hDlg, IDC_EDIT_DEL_BUINUM),
				EM_GETLINE, 2, (LPARAM)szBuffer[1]);
			iLength[2] = SendMessage(GetDlgItem(hDlg, IDC_EDIT_DEL_ROOMNUM),
				EM_GETLINE, 3, (LPARAM)szBuffer[2]);

			//检验每一项非空

			for (i = 0; i < 3; i++)
			{
				if (!iLength[i])
				{
					MessageBox(hDlg, TEXT("请输入完整的信息！"), TEXT("提示"), MB_OK);
					return FALSE;
				}
				else {
					*(szBuffer[i] + iLength[i]) = '\0\0';
				}
			}

			//将所在楼盘名转化为楼盘号

			if (iStateEdit == 1)
			{
				comTail = pHead;

				//循环查找楼盘名
				while (comTail != NULL)
				{
					if (!lstrcmp(comTail->name, szBuffer[0]))
					{
						iComDelNum = comTail->communityNum;
						break;
					}
					comTail = comTail->nextCommunity;
				}
			}
			else if (iStateEdit == 2)
			{
				iComDelNum = _ttoi(szBuffer[0]);
			}
			iBuiDelNum = _ttoi(szBuffer[1]);
			iRoomDelNum = _ttoi(szBuffer[2]);

			//检查楼盘名是否正确
			if (iComDelNum == -1)
			{
				MessageBox(hDlg, TEXT("该楼盘名称不存在！"), TEXT("提示"), MB_OK);
				return FALSE;
			}

			//删除，并检验房间号
			if (!DelRoomInList(iComDelNum, iBuiDelNum, iRoomDelNum, &pHead))
			{
				MessageBox(hDlg, TEXT("找不到该房间！"), TEXT("提示"), MB_OK);
				return FALSE;
			}

			bRePaint = TRUE;
			EndDialog(hDlg, TRUE);
			return TRUE;

		case IDCANCEL:
			bRePaint = FALSE;
			EndDialog(hDlg, FALSE);
			return TRUE;
		}
		break;
	}
	return FALSE;
}


///////////////////////////////////////////////////////
//绘制按钮
//参数：hdc，信息窗口右下角的坐标

void DrawButton(HDC hdc,int x,int y)
{
	int cxChar, cyChar;
	TEXTMETRIC tm;

	GetTextMetrics(hdc, &tm);
	cxChar = tm.tmAveCharWidth;
	cyChar = tm.tmHeight + tm.tmExternalLeading;

	MoveWindow(hwndButtonDel, x - cxChar * 8, y - cyChar * 2,
		cxChar * 6, cyChar * 7 / 4, TRUE);

	MoveWindow(hwndButtonEdit, x - cxChar * 16, y - cyChar * 2,
		cxChar * 6, cyChar * 7 / 4, TRUE);

	if (SEARCHING)
		MoveWindow(hwndButtonEnter, x - cxChar * 24, y - cyChar * 2, 
			cxChar * 6, cyChar * 7 / 4, TRUE);

	ShowWindow(hwndButtonEnter, SW_SHOW);
}

///////////////////////////////////////////////////////
//“编辑楼盘信息”（EditComDlgProc）对话框窗口过程

BOOL CALLBACK EditComDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	TCHAR szBuffer[100][100];
	Community *comTail;
	int i;
	int iLength[5];

	switch (message)
	{
	case WM_INITDIALOG:

		//设置编辑框初始内容
		comTail = pHead;
		while (comTail != NULL)
		{
			if (!lstrcmp(comTail->name, comNow))
			{
				SetWindowText(GetDlgItem(hDlg, IDC_ADD_COM_NAME), comTail->name);
				SetWindowText(GetDlgItem(hDlg, IDC_ADD_COM_ADDR), comTail->address);
				SetWindowText(GetDlgItem(hDlg, IDC_ADD_COM_PHONE), comTail->phone);
				SetWindowText(GetDlgItem(hDlg, IDC_ADD_COM_PERSON), comTail->host.name);
				break;
			}
			comTail = comTail->nextCommunity;
		}
		return TRUE;

	case WM_COMMAND:

		switch (LOWORD(wParam))
		{
		case IDOK:

			//当按下OK时，将所输入内容存进存储区	

			iLength[0] = SendMessage(GetDlgItem(hDlg, IDC_ADD_COM_NAME),
				EM_GETLINE, 0, (LPARAM)szBuffer[0]);
			iLength[1] = SendMessage(GetDlgItem(hDlg, IDC_ADD_COM_ADDR),
				EM_GETLINE, 1, (LPARAM)szBuffer[1]);
			iLength[2] = SendMessage(GetDlgItem(hDlg, IDC_ADD_COM_PHONE),
				EM_GETLINE, 2, (LPARAM)szBuffer[2]);
			iLength[3] = SendMessage(GetDlgItem(hDlg, IDC_ADD_COM_PERSON),
				EM_GETLINE, 3, (LPARAM)szBuffer[3]);

			//检验每一项非空

			for (i = 0; i < 4; i++)
			{
				if (!iLength[i])
				{
					MessageBox(hDlg, TEXT("请输入完整的信息！"), TEXT("提示"), MB_OK);
					return FALSE;
				}
				else {
					*(szBuffer[i] + iLength[i]) = '\0\0';
				}
			}

			//修改链表内数据

			comTail = pHead;
			while (comTail != NULL)
			{
				if (!lstrcmp(comTail->name, comNow))
				{
					if (CheckSameCom(szBuffer[0], &pHead) ||
						(lstrcmp(comTail->name, szBuffer[0]) == 0))
					{
						lstrcpy(comTail->name, szBuffer[0]);
						lstrcpy(comTail->address, szBuffer[1]);
						lstrcpy(comTail->phone, szBuffer[2]);
						lstrcpy(comTail->host.name, szBuffer[3]);

						lstrcpy(comNow, comTail->name);
					}
					else
					{
						MessageBox(hwnd, TEXT("该楼盘号已在系统中"),
							TEXT("警告"), MB_OK | MB_ICONWARNING);
						return FALSE;
					}
					break;
				}
				comTail = comTail->nextCommunity;
			}

			bRePaint = TRUE;
			EndDialog(hDlg, TRUE);
			return TRUE;

		case IDCANCEL:
			bRePaint = FALSE;
			EndDialog(hDlg, FALSE);
			return TRUE;
		}

		break;
	}

	return FALSE;
}


///////////////////////////////////////////////////////
//“编辑楼栋信息”（EditBuiDlgProc）对话框窗口过程

BOOL CALLBACK EditBuiDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	TCHAR szBuffer[100][100];
	Community *comTail;
	Building *buiTail;
	int i;
	int iLength[5];

	switch (message)
	{
	case WM_INITDIALOG:

		//设置编辑框初始内容
		comTail = pHead;
		while (comTail != NULL)
		{
			if (!lstrcmp(comTail->name, comNow))
			{
				buiTail = comTail->buildings;
				while (buiTail != NULL)
				{
					if (buiTail->buildingNum == buiNow)
					{
						wsprintf(szBuffer[0], TEXT("%d"), buiTail->buildingNum);
						SetWindowText(GetDlgItem(hDlg, IDC_ADD_BUI_NUM), szBuffer[0]);
						wsprintf(szBuffer[1], TEXT("%d"), buiTail->numberOfFloors);
						SetWindowText(GetDlgItem(hDlg, IDC_ADD_BUI_FLOORS), szBuffer[1]);
						SetWindowText(GetDlgItem(hDlg, IDC_ADD_BUI_PHONE), buiTail->host.phone);
						SetWindowText(GetDlgItem(hDlg, IDC_ADD_BUI_PERSON), buiTail->host.name);
						break;
					}
					buiTail = buiTail->nextBuilding;
				}
				break;
			}
			comTail = comTail->nextCommunity;
		}
		return TRUE;

	case WM_COMMAND:

		switch (LOWORD(wParam))
		{
		case IDOK:

			//当按下OK时，将所输入内容存进存储区

			iLength[0] = SendMessage(GetDlgItem(hDlg, IDC_ADD_BUI_NUM),
				EM_GETLINE, 0, (LPARAM)szBuffer[0]);
			iLength[1] = SendMessage(GetDlgItem(hDlg, IDC_ADD_BUI_FLOORS),
				EM_GETLINE, 1, (LPARAM)szBuffer[1]);
			iLength[2] = SendMessage(GetDlgItem(hDlg, IDC_ADD_BUI_PHONE),
				EM_GETLINE, 2, (LPARAM)szBuffer[2]);
			iLength[3] = SendMessage(GetDlgItem(hDlg, IDC_ADD_BUI_PERSON),
				EM_GETLINE, 3, (LPARAM)szBuffer[3]);

			//检验每一项非空

			for (i = 0; i < 4; i++)
			{
				if (!iLength[i])
				{
					MessageBox(hDlg, TEXT("请输入完整的信息！"), TEXT("提示"), MB_OK);
					return FALSE;
				}
				else {
					*(szBuffer[i] + iLength[i]) = '\0\0';
				}
			}

			//修改链表内数据

			comTail = pHead;
			while (comTail != NULL)
			{

				if (!lstrcmp(comTail->name, comNow))
				{
					buiTail = comTail->buildings;
					while (buiTail != NULL)
					{
						if (buiTail->buildingNum == buiNow)
						{
							if (CheckSameBui(comTail->name,
								_ttoi(szBuffer[0]), &pHead) ||
								(buiTail->buildingNum == _ttoi(szBuffer[0])))
							{
								buiTail->buildingNum = _ttoi(szBuffer[0]);
								buiTail->numberOfFloors = _ttoi(szBuffer[1]);
								lstrcpy(buiTail->host.phone, szBuffer[2]);
								lstrcpy(buiTail->host.name, szBuffer[3]);

								buiNow = buiTail->buildingNum;
							}
							else
							{
								MessageBox(hwnd, TEXT("该楼栋号已在系统中"),
									TEXT("警告"), MB_OK | MB_ICONWARNING);
								return FALSE;
							}
							break;
						}
						buiTail = buiTail->nextBuilding;
					}
					break;
				}
				comTail = comTail->nextCommunity;
			}

			bRePaint = TRUE;
			EndDialog(hDlg, TRUE);
			return TRUE;

		case IDCANCEL:
			bRePaint = FALSE;
			EndDialog(hDlg, FALSE);
			return TRUE;
		}

		break;
	}

	return FALSE;
}


///////////////////////////////////////////////////////
//“编辑房间信息”（EditRoomDlgProc）对话框窗口过程

BOOL CALLBACK EditRoomDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	TCHAR szBuffer[100][100];
	Community *comTail;
	Building *buiTail;
	Room *roomTail;
	int i, iSold;
	int iLength[5];

	switch (message)
	{
	case WM_INITDIALOG:

		//向ComboBox框中添加数据
		for (i = 0; i < 34; i++)
			SendMessage(GetDlgItem(hDlg, IDC_ADD_ROOM_STYLE),
				CB_ADDSTRING, 0, (LPARAM)szComboBoxData[i]);

		//设置编辑框初始内容
		comTail = pHead;
		while (comTail != NULL)
		{
			if (!lstrcmp(comTail->name, comNow))
			{
				buiTail = comTail->buildings;
				while (buiTail != NULL)
				{
					if (buiTail->buildingNum == buiNow)
					{
						roomTail = buiTail->rooms;
						while (roomTail != NULL)
						{
							if (roomTail->roomNum == roomNow)
							{
								wsprintf(szBuffer[2], TEXT("%d"), roomTail->roomNum);
								SetWindowText(GetDlgItem(hDlg, IDC_ADD_ROOM_ROOMNUM), szBuffer[2]);
								swprintf(szBuffer[3], TEXT("%.2f"), roomTail->roomSize);
								SetWindowText(GetDlgItem(hDlg, IDC_ADD_ROOM_SIZE), szBuffer[3]);
								swprintf(szBuffer[4], TEXT("%.2f"), roomTail->roomPrice);
								SetWindowText(GetDlgItem(hDlg, IDC_ADD_ROOM_PRICE), szBuffer[4]);

								//设置默认户型
								for (i = 0; i < 34; i++)
									if (!lstrcmp(szComboBoxData[i], roomTail->roomType))
										break;
								SendMessage(GetDlgItem(hDlg, IDC_ADD_ROOM_STYLE),
									CB_SETCURSEL, i, 0);

								//设置默认售出状态
								if (roomTail->roomState)
									SendMessage(GetDlgItem(hDlg, IDC_ADD_ROOM_SOLD),
										BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
								break;
							}
							roomTail = roomTail->nextRoom;
						}					
						break;
					}
					buiTail = buiTail->nextBuilding;
				}
				break;
			}
			comTail = comTail->nextCommunity;
		}
		return TRUE;

	case WM_COMMAND:

		switch (LOWORD(wParam))
		{
		case IDOK:

			//当按下OK时，将所输入内容存进存储区	

			iLength[0] = SendMessage(GetDlgItem(hDlg, IDC_ADD_ROOM_ROOMNUM),
				EM_GETLINE, 0, (LPARAM)szBuffer[0]);
			iLength[1] = SendMessage(GetDlgItem(hDlg, IDC_ADD_ROOM_SIZE),
				EM_GETLINE, 1, (LPARAM)szBuffer[1]);
			iLength[2] = SendMessage(GetDlgItem(hDlg, IDC_ADD_ROOM_PRICE),
				EM_GETLINE, 2, (LPARAM)szBuffer[2]);
			GetWindowText(GetDlgItem(hDlg, IDC_ADD_ROOM_STYLE),
				szBuffer[3], lstrlen(szBuffer[3]));
			iSold = SendMessage(GetDlgItem(hDlg, IDC_ADD_ROOM_SOLD),
				BM_GETCHECK, 0, 0);

			//检验每一项非空

			for (i = 0; i < 3; i++)
			{
				if (!iLength[i])
				{
					MessageBox(hDlg, TEXT("请输入完整的信息！"), TEXT("提示"), MB_OK);
					return FALSE;
				}
				else {
					*(szBuffer[i] + iLength[i]) = '\0\0';
				}
			}

			//修改链表内数据

			comTail = pHead;
			while (comTail != NULL)
			{

				if (!lstrcmp(comTail->name, comNow))
				{
					buiTail = comTail->buildings;
					while (buiTail != NULL)
					{
						if (buiTail->buildingNum == buiNow)
						{
							roomTail = buiTail->rooms;
							while (roomTail != NULL)
							{
								if (roomTail->roomNum == roomNow)
								{
									if (CheckSameRoom(comTail->name,
										buiTail->buildingNum, _ttoi(szBuffer[0]), &pHead) ||
										(roomTail->roomNum == _ttoi(szBuffer[0])))
									{
										roomTail->roomNum = _ttoi(szBuffer[0]);
										roomTail->roomSize = _ttof(szBuffer[1]);
										roomTail->roomPrice = _ttof(szBuffer[2]);
										lstrcpy(roomTail->roomType, szBuffer[3]);
										roomTail->roomState = iSold;

										roomNow = roomTail->roomNum;
									}
									else
									{
										MessageBox(hwnd, TEXT("该房间名已在系统中"),
											TEXT("警告"), MB_OK | MB_ICONWARNING);
										return FALSE;
									}					
									break;
								}
								roomTail = roomTail->nextRoom;
							}						
							break;
						}
						buiTail = buiTail->nextBuilding;
					}
					break;
				}
				comTail = comTail->nextCommunity;
			}

			bRePaint = TRUE;
			EndDialog(hDlg, TRUE);
			return TRUE;

		case IDCANCEL:
			bRePaint = FALSE;
			EndDialog(hDlg, FALSE);
			return TRUE;
		}
		break;
	}
	return FALSE;
}


///////////////////////////////////////////////////////
//计算出每一个楼盘的相似度
//参数：标准SearchData结构，对比的楼盘
//返回：这个输入信息与该楼盘的相似度

int GetComSim(SearchData data, Community com)
{
	int weigh = 0;

	if (data.mask != SD_COM)
		return -1;

	if (!lstrcmp(data.comName, com.name))
		weigh += 3;
	if (data.comNum == com.communityNum)
		weigh += 2;
	if (!lstrcmp(data.comPerson, com.host.name))
		weigh += 1;
	if (!lstrcmp(data.comPhone, com.phone))
		weigh += 1;

	return weigh;
}


///////////////////////////////////////////////////////
//计算出每一个楼栋的相似度
//参数：标准SearchData结构，对比的楼栋，该楼栋所在楼盘
//返回：这个输入信息与该楼栋的相似度

int GetBuiSim(SearchData data, Community com, Building bui)
{
	int weigh = 0;

	if (data.mask != SD_BUI)
		return -1;

	if (!lstrcmp(data.comName, com.name))
		weigh += 5;
	if (data.comNum == com.communityNum)
		weigh += 2;
	if (!lstrcmp(data.comPerson, com.host.name))
		weigh += 1;
	if (!lstrcmp(data.comPhone, com.phone))
		weigh += 1;

	if (data.buiNum == bui.buildingNum)
		weigh += 5;
	if (!lstrcmp(data.buiPerson, bui.host.name))
		weigh += 1;
	if (!lstrcmp(data.buiPhone, bui.host.phone))
		weigh += 1;

	return weigh;
}


///////////////////////////////////////////////////////
//计算出每一个房间的相似度
//参数：标准SearchData结构，对比的房间，该房间所在楼栋、楼盘
//返回：这个输入信息与该房间的相似度

int GetRoomSim(SearchData data, Community com, Building bui, Room room)
{
	int weigh = 0;

	if (data.mask != SD_ROOM)
		return -1;

	if (!lstrcmp(data.comName, com.name))
		weigh += 5;
	if (data.comNum == com.communityNum)
		weigh += 3;
	if (!lstrcmp(data.comPerson, com.host.name))
		weigh += 1;
	if (!lstrcmp(data.comPhone, com.phone))
		weigh += 1;

	if (data.buiNum == bui.buildingNum)
		weigh += 5;
	if (!lstrcmp(data.buiPerson, bui.host.name))
		weigh += 1;
	if (!lstrcmp(data.buiPhone, bui.host.phone))
		weigh += 1;

	if (data.roomNum == room.roomNum)
		weigh += 5;
	if (!lstrcmp(data.roomType, room.roomType))
		weigh += 1;
	if (data.roomFloor == room.floor)
		weigh += 2;
	if (data.roomSize <= room.roomSize + 10 &&
		data.roomSize >= room.roomSize - 10)
		weigh += 2;
	if (room.roomPrice >= data.roomLoPrice &&
		room.roomPrice <= data.roomHiPrice)
		weigh += 2;
	if (room.allPrice <= data.allPrice)
		weigh += 3;
	if (room.roomState == data.roomSold)
		weigh += 2;

	return weigh;
}


///////////////////////////////////////////////////////
//“搜索功能”（Search）对话框窗口过程

BOOL CALLBACK SearchDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	TCHAR szBuffer[100][100];
	Community *comTail;
	Building *buiTail;
	Room *roomTail;
	SearchData sData;
	BOOL success = FALSE;
	int i, iSold;
	static int iSearchPos = 0;	//1表示com，2表示bui，3表示room，0表示未选择
	int iLength[20];

	switch (message)
	{
	case WM_INITDIALOG:

		//向ComboBox框中添加数据
		for (i = 0; i < 34; i++)
			SendMessage(GetDlgItem(hDlg, IDC_S_ROOMTYPE),
				CB_ADDSTRING, 0, (LPARAM)szComboBoxData[i]);

		//设置默认选项楼盘
		SendMessage(GetDlgItem(hDlg, IDC_SCOM),
			BM_SETCHECK, BST_CHECKED, 0);
		//Disable楼栋和房间
		EnableWindow(GetDlgItem(hDlg, IDC_S_BUINUM), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_S_BUIPERSON), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_S_BUIPHONE), FALSE);

		EnableWindow(GetDlgItem(hDlg, IDC_S_ROOMNUM), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_S_ROOMSIZE), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_S_ROOMFLOOR), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_S_ALLPRICE), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_S_LOPRICE), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_S_HIPRICE), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_S_ROOMTYPE), FALSE);

		iSearchPos = 1;

		return TRUE;

	case WM_COMMAND:

		switch (LOWORD(wParam))
		{
		case IDC_SCOM:

			if (HIWORD(wParam) == BN_CLICKED)
			{
				//Disable楼栋和房间
				EnableWindow(GetDlgItem(hDlg, IDC_S_BUINUM), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_BUIPERSON), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_BUIPHONE), FALSE);

				EnableWindow(GetDlgItem(hDlg, IDC_S_ROOMNUM), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_ROOMSIZE), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_ROOMFLOOR), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_ALLPRICE), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_LOPRICE), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_HIPRICE), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_ROOMTYPE), FALSE);

				iSearchPos = 1;
			}
			break;

		case IDC_SBUI:

			if (HIWORD(wParam) == BN_CLICKED)
			{
				//Disable房间

				EnableWindow(GetDlgItem(hDlg, IDC_S_ROOMNUM), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_ROOMSIZE), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_ROOMFLOOR), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_ALLPRICE), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_LOPRICE), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_HIPRICE), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_ROOMTYPE), FALSE);

				//Enable楼栋

				EnableWindow(GetDlgItem(hDlg, IDC_S_BUINUM), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_BUIPERSON), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_BUIPHONE), TRUE);

				iSearchPos = 2;
			}
			break;

		case IDC_SROOM:

			if (HIWORD(wParam) == BN_CLICKED)
			{
				//Enable房间和楼栋

				EnableWindow(GetDlgItem(hDlg, IDC_S_ROOMNUM), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_ROOMSIZE), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_ROOMFLOOR), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_ALLPRICE), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_LOPRICE), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_HIPRICE), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_ROOMTYPE), TRUE);

				EnableWindow(GetDlgItem(hDlg, IDC_S_BUINUM), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_BUIPERSON), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_BUIPHONE), TRUE);

				iSearchPos = 3;
			}
			break;

		case IDOK:

			switch (iSearchPos)
			{
			case 0:		//若搜索对象为空（基本不可能出现）
				MessageBox(hDlg, TEXT("请选择将要搜索的对象"), TEXT("提示"), MB_OK);
				break;

			case 1:		//若搜索对象为楼盘

				sData.mask = SD_COM;
				
				iLength[0] = SendMessage(GetDlgItem(hDlg, IDC_S_COMNAME),
					EM_GETLINE, 0, (LPARAM)szBuffer[0]);
				iLength[1] = SendMessage(GetDlgItem(hDlg, IDC_S_COMNUM),
					EM_GETLINE, 0, (LPARAM)szBuffer[1]);
				iLength[2] = SendMessage(GetDlgItem(hDlg, IDC_S_COMPERSON),
					EM_GETLINE, 0, (LPARAM)szBuffer[2]);
				iLength[3] = SendMessage(GetDlgItem(hDlg, IDC_S_COMPHONE),
					EM_GETLINE, 0, (LPARAM)szBuffer[3]);

				//将每一个字符串添加结尾
				for (i = 0; i < 4; i++)
					*(szBuffer[i] + iLength[i]) = '\0\0';

				//录入搜索框信息
				lstrcpy(sData.comName, szBuffer[0]);
				sData.comNum = _ttoi(szBuffer[1]);
				lstrcpy(sData.comPerson, szBuffer[2]);
				lstrcpy(sData.comPhone, szBuffer[3]);
				
				//构建相似的搜索结果组成链表
				pComSearch = NULL;
				success = FALSE;
				for(comTail=pHead;comTail!=NULL;
					comTail=comTail->nextCommunity)
					if (GetComSim(sData, *comTail))
					{
						AddComToList(*comTail, &pComSearch);
						success = TRUE;
					}

				if (success == FALSE)
				{
					MessageBox(hwnd, TEXT("未找到搜索结果"), TEXT("提示"), MB_OK);
					return FALSE;
				}

				//将搜索结果的链表进行排序
				pComSearch = RankCom(sData, pComSearch);

				iSearch = SEARCHCOM;
				break;

			case 2:		//若搜索对象为楼栋

				sData.mask = SD_BUI;

				iLength[0] = SendMessage(GetDlgItem(hDlg, IDC_S_COMNAME),
					EM_GETLINE, 0, (LPARAM)szBuffer[0]);
				iLength[1] = SendMessage(GetDlgItem(hDlg, IDC_S_COMNUM),
					EM_GETLINE, 0, (LPARAM)szBuffer[1]);
				iLength[2] = SendMessage(GetDlgItem(hDlg, IDC_S_COMPERSON),
					EM_GETLINE, 0, (LPARAM)szBuffer[2]);
				iLength[3] = SendMessage(GetDlgItem(hDlg, IDC_S_COMPHONE),
					EM_GETLINE, 0, (LPARAM)szBuffer[3]);

				iLength[4] = SendMessage(GetDlgItem(hDlg, IDC_S_BUINUM),
					EM_GETLINE, 0, (LPARAM)szBuffer[4]);
				iLength[5] = SendMessage(GetDlgItem(hDlg, IDC_S_BUIPERSON),
					EM_GETLINE, 0, (LPARAM)szBuffer[5]);
				iLength[6] = SendMessage(GetDlgItem(hDlg, IDC_S_BUIPHONE),
					EM_GETLINE, 0, (LPARAM)szBuffer[6]);

				//将每一个字符串添加结尾
				for (i = 0; i < 7; i++)
					*(szBuffer[i] + iLength[i]) = '\0\0';

				//录入搜索框信息
				lstrcpy(sData.comName, szBuffer[0]);
				sData.comNum = _ttoi(szBuffer[1]);
				lstrcpy(sData.comPerson, szBuffer[2]);
				lstrcpy(sData.comPhone, szBuffer[3]);
				sData.buiNum = _ttoi(szBuffer[4]);
				lstrcpy(sData.buiPerson, szBuffer[5]);
				lstrcpy(sData.buiPhone, szBuffer[6]);

				//构建相似的搜索结果组成链表
				success = FALSE;
				pBuiSearch = NULL;
				for (comTail = pHead; comTail != NULL;
					comTail = comTail->nextCommunity)
					for (buiTail = comTail->buildings; buiTail != NULL;
						buiTail = buiTail->nextBuilding)
						if (GetBuiSim(sData, *comTail, *buiTail))
						{
							AddBuiToSearch(*buiTail, &pBuiSearch);
							success = TRUE;
						}

				if (success == FALSE)
				{
					MessageBox(hwnd, TEXT("未找到搜索结果"), TEXT("提示"), MB_OK);
					return FALSE;
				}

				//将搜索结果的链表进行排序
				pBuiSearch = RankBui(sData, pBuiSearch);

				iSearch = SEARCHBUI;
				break;

			case 3:		//若搜索对象为房间

				sData.mask = SD_ROOM;

				iLength[0] = SendMessage(GetDlgItem(hDlg, IDC_S_COMNAME),
					EM_GETLINE, 0, (LPARAM)szBuffer[0]);
				iLength[1] = SendMessage(GetDlgItem(hDlg, IDC_S_COMNUM),
					EM_GETLINE, 0, (LPARAM)szBuffer[1]);
				iLength[2] = SendMessage(GetDlgItem(hDlg, IDC_S_COMPERSON),
					EM_GETLINE, 0, (LPARAM)szBuffer[2]);
				iLength[3] = SendMessage(GetDlgItem(hDlg, IDC_S_COMPHONE),
					EM_GETLINE, 0, (LPARAM)szBuffer[3]);

				iLength[4] = SendMessage(GetDlgItem(hDlg, IDC_S_BUINUM),
					EM_GETLINE, 0, (LPARAM)szBuffer[4]);
				iLength[5] = SendMessage(GetDlgItem(hDlg, IDC_S_BUIPERSON),
					EM_GETLINE, 0, (LPARAM)szBuffer[5]);
				iLength[6] = SendMessage(GetDlgItem(hDlg, IDC_S_BUIPHONE),
					EM_GETLINE, 0, (LPARAM)szBuffer[6]);

				iLength[7] = SendMessage(GetDlgItem(hDlg, IDC_S_ROOMNUM),
					EM_GETLINE, 0, (LPARAM)szBuffer[7]);
				iLength[8] = SendMessage(GetDlgItem(hDlg, IDC_S_ROOMSIZE),
					EM_GETLINE, 0, (LPARAM)szBuffer[8]);
				iLength[9] = SendMessage(GetDlgItem(hDlg, IDC_S_ROOMFLOOR),
					EM_GETLINE, 0, (LPARAM)szBuffer[9]);
				iLength[10] = SendMessage(GetDlgItem(hDlg, IDC_S_ALLPRICE),
					EM_GETLINE, 0, (LPARAM)szBuffer[10]);
				iLength[11] = SendMessage(GetDlgItem(hDlg, IDC_S_LOPRICE),
					EM_GETLINE, 0, (LPARAM)szBuffer[11]);
				iLength[12] = SendMessage(GetDlgItem(hDlg, IDC_S_HIPRICE),
					EM_GETLINE, 0, (LPARAM)szBuffer[12]);
				GetWindowText(GetDlgItem(hDlg, IDC_S_ROOMTYPE),
					szBuffer[13], lstrlen(szBuffer[13]));
				iSold = SendMessage(GetDlgItem(hDlg, IDC_S_SOLD), BM_GETCHECK, 0, 0);

				//将每一个字符串添加结尾
				for (i = 0; i < 13; i++)
					*(szBuffer[i] + iLength[i]) = '\0\0';

				//录入搜索框信息
				lstrcpy(sData.comName, szBuffer[0]);
				sData.comNum = _ttoi(szBuffer[1]);
				lstrcpy(sData.comPerson, szBuffer[2]);
				lstrcpy(sData.comPhone, szBuffer[3]);
				sData.buiNum = _ttoi(szBuffer[4]);
				lstrcpy(sData.buiPerson, szBuffer[5]);
				lstrcpy(sData.buiPhone, szBuffer[6]);

				sData.roomNum = _ttoi(szBuffer[7]);
				sData.roomSize = _ttof(szBuffer[8]);
				sData.roomFloor = _ttof(szBuffer[9]);
				sData.allPrice = 10000.0 * _ttof(szBuffer[10]);
				sData.roomLoPrice = _ttof(szBuffer[11]);
				sData.roomHiPrice = _ttof(szBuffer[12]);
				lstrcpy(sData.roomType, szBuffer[13]);
				sData.roomSold = iSold;

				//构建相似的搜索结果组成链表
				success = FALSE;
				pRoomSearch = NULL;
				for (comTail = pHead; comTail != NULL;
					comTail = comTail->nextCommunity)
					for (buiTail = comTail->buildings; buiTail != NULL;
						buiTail = buiTail->nextBuilding)
						for (roomTail = buiTail->rooms; roomTail != NULL;
							roomTail = roomTail->nextRoom)
						if (GetRoomSim(sData, *comTail, *buiTail, *roomTail))
						{
							AddRoomToSearch(*roomTail, &pRoomSearch);
							success = TRUE;
						}

				if (success == FALSE)
				{
					MessageBox(hwnd, TEXT("未找到搜索结果"), TEXT("提示"), MB_OK);
					return FALSE;
				}

				//将搜索结果的链表进行排序
				pRoomSearch = RankRoom(sData, pRoomSearch);

				iSearch = SEARCHROOM;

				break;

			default:
				break;
			}

			EndDialog(hDlg, TRUE);
			return TRUE;

		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			return TRUE;

		default:
			break;
		}
	}
	return FALSE;
}


///////////////////////////////////////////////////////
//对Com链表按照权重进行排序
//参数：标准比较输入数据，排序的链表头指针
//返回：返回为排完序的链表的头指针

Community *RankCom(SearchData sData, Community *pComSearch)
{
	Community *comTail, *p1, *p2, *p3;
	Community *comEnd, *tmp;
	Community *aHead;

	if (pComSearch == NULL)
		return NULL;

	if (pComSearch->nextCommunity == NULL)
		return pComSearch;

	aHead = (Community*)malloc(sizeof(Community));
	aHead->nextCommunity = pComSearch;

	comEnd = NULL, tmp = NULL;
	while (aHead != comEnd)
	{
		tmp = comTail = aHead;
		while (comTail->nextCommunity->nextCommunity != comEnd)
		{
			if (GetComSim(sData, *(comTail->nextCommunity))
				< GetComSim(sData, *(comTail->nextCommunity->nextCommunity)))
			{
				//交换 p->next 和 p->next->next 的位置
				p1 = comTail->nextCommunity;
				p2 = p1->nextCommunity;
				p3 = p2->nextCommunity;
				comTail->nextCommunity = p2;
				p2->nextCommunity = p1;
				p1->nextCommunity = p3;
				tmp = comTail->nextCommunity->nextCommunity;
			}
			comTail = comTail->nextCommunity;
		}
		comEnd = tmp;
	}

	return aHead->nextCommunity;
}


///////////////////////////////////////////////////////
//对Bui链表按照权重进行排序
//参数：标准比较输入数据，排序的链表头指针
//返回：返回为排完序的链表的头指针

Building *RankBui(SearchData sData, Building *pBuiSearch)
{
	Building *buiTail, *p1, *p2, *p3;
	Building *buiEnd, *tmp;
	Building *aHead;
	Community com, comNext, *pTailCom;

	if (pBuiSearch == NULL)
		return NULL;

	if (pBuiSearch->nextBuilding == NULL)
		return pBuiSearch;

	aHead = (Building*)malloc(sizeof(Building));
	aHead->nextBuilding = pBuiSearch;

	buiEnd = NULL, tmp = NULL;
	while (aHead != buiEnd)
	{
		tmp = buiTail = aHead;
		while (buiTail->nextBuilding->nextBuilding != buiEnd)
		{
			//找到两个所在的楼盘
			for (pTailCom = pHead; pTailCom != NULL; 
				pTailCom = pTailCom->nextCommunity)
			{
				if (lstrcmp(buiTail->nextBuilding->inCom,
					pTailCom->name) == 0)
					com = *pTailCom;
				if (lstrcmp(buiTail->nextBuilding->nextBuilding->inCom,
					pTailCom->name) == 0)
					comNext = *pTailCom;
			}

			if (GetBuiSim(sData, com, *(buiTail->nextBuilding))
				< GetBuiSim(sData, comNext, 
					*(buiTail->nextBuilding->nextBuilding)))
			{
				//交换 p->next 和 p->next->next 的位置
				p1 = buiTail->nextBuilding;
				p2 = p1->nextBuilding;
				p3 = p2->nextBuilding;
				buiTail->nextBuilding = p2;
				p2->nextBuilding = p1;
				p1->nextBuilding = p3;
				tmp = buiTail->nextBuilding->nextBuilding;
			}
			buiTail = buiTail->nextBuilding;
		}
		buiEnd = tmp;
	}

	return aHead->nextBuilding;
}


///////////////////////////////////////////////////////
//对Room链表按照权重进行排序
//参数：标准比较输入数据，排序的链表头指针
//返回：返回为排完序的链表的头指针

Room *RankRoom(SearchData sData, Room *pRoomSearch)
{
	Room *roomTail, *p1, *p2, *p3;
	Room *roomEnd, *tmp;
	Room *aHead;
	Building bui, buiNext, *pTailBui;
	Community com, comNext, *pTailCom;

	if (pRoomSearch == NULL)
		return NULL;

	if (pRoomSearch->nextRoom == NULL)
		return pRoomSearch;

	aHead = (Room*)malloc(sizeof(Room));
	aHead->nextRoom = pRoomSearch;

	roomEnd = NULL, tmp = NULL;
	while (aHead != roomEnd)
	{
		tmp = roomTail = aHead;
		while (roomTail->nextRoom->nextRoom != roomEnd)
		{
			//找到两个所在的楼盘
			for (pTailCom = pHead; pTailCom != NULL;
				pTailCom = pTailCom->nextCommunity)
			{
				if (lstrcmp(roomTail->nextRoom->inCom,
					pTailCom->name) == 0)
					com = *pTailCom;
				if (lstrcmp(roomTail->nextRoom->nextRoom->inCom,
					pTailCom->name) == 0)
					comNext = *pTailCom;
				for (pTailBui = pTailCom->buildings; pTailBui != NULL;
					pTailBui = pTailBui->nextBuilding)
				{
					if (roomTail->nextRoom->buildingNum
						== pTailBui->buildingNum)
						bui = *pTailBui;
					if (roomTail->nextRoom->nextRoom->buildingNum
						== pTailBui->buildingNum)
						buiNext = *pTailBui;
				}
			}

			if (GetRoomSim(sData, com, bui, *(roomTail->nextRoom))
				< GetRoomSim(sData, comNext, buiNext,
					*(roomTail->nextRoom->nextRoom)))
			{
				//交换 p->next 和 p->next->next 的位置
				p1 = roomTail->nextRoom;
				p2 = p1->nextRoom;
				p3 = p2->nextRoom;
				roomTail->nextRoom = p2;
				p2->nextRoom = p1;
				p1->nextRoom = p3;
				tmp = roomTail->nextRoom->nextRoom;
			}
			roomTail = roomTail->nextRoom;
		}
		roomEnd = tmp;
	}

	return aHead->nextRoom;
}


///////////////////////////////////////////////////////
//检查是否有重复的房间
//参数：所在楼盘名，所在楼栋号，房间号，排序的链表头指针
//返回：如没有返回TRUE，有返回FALSE

BOOL CheckSameRoom(TCHAR comName[], int buiNum, int roomNum, Community **head)
{
	Community *comTail;
	Building *buiTail;
	Room *roomTail;

	for (comTail = *head; comTail; comTail = comTail->nextCommunity)
		if (!lstrcmp(comTail->name, comName))
			for (buiTail = comTail->buildings; buiTail; buiTail = buiTail->nextBuilding)
				if (buiNum == buiTail->buildingNum)
					for (roomTail = buiTail->rooms; roomTail; roomTail = roomTail->nextRoom)
						if (roomNum == roomTail->roomNum)
							return FALSE;
	
	return TRUE;
}


///////////////////////////////////////////////////////
//检查是否有重复的楼栋
//参数：所在楼盘名，楼栋号，排序的链表头指针
//返回：如没有返回TRUE，有返回FALSE

BOOL CheckSameBui(TCHAR comName[], int buiNum, Community **head)
{
	Community *comTail;
	Building *buiTail;

	for (comTail = *head; comTail; comTail = comTail->nextCommunity)
		if (!lstrcmp(comTail->name, comName))
			for (buiTail = comTail->buildings; buiTail; buiTail = buiTail->nextBuilding)
				if (buiNum == buiTail->buildingNum)
					return FALSE;

	return TRUE;
}


///////////////////////////////////////////////////////
//检查是否有重复的楼栋
//参数：楼盘号，排序的链表头指针
//返回：如没有返回TRUE，有返回FALSE

BOOL CheckSameCom(TCHAR comName[], Community **head)
{
	Community *comTail;

	for (comTail = *head; comTail; comTail = comTail->nextCommunity)
		if (!lstrcmp(comTail->name, comName))
			return FALSE;

	return TRUE;
}