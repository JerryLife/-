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
static TCHAR comNow[100];	//Ŀǰ����com
static int buiNow;			//Ŀǰ����bui
static int roomNow;			//Ŀǰ����Room
static int iSearch = NOSEARCH;		//�Ƿ���������
RECT rectData;
Community pcBuffer[10];
Building pbBuffer[10];
Room prBuffer[10];
HWND hwnd, hwndMainList, hwndMainTitle, hwndSubTitle;
HWND hwndButtonDel, hwndButtonEdit, hwndButtonEnter;
HWND hwndSubList;
HFONT hFontSon, hFontBlack;
enum fState {SUCCEED, FAIL};
enum fState fAddBuiState;		//�Ƿ�ɹ����¥��
enum fState fAddRoomState;		//�Ƿ�ɹ���ӷ���
int iAutoComNum = 1, iAutoBuiNum = 101;

static TCHAR *szComboBoxData[] = {
	TEXT("����1��һ��һ��һ��һ��"),TEXT("����18������һ��һ������"),
	TEXT("����2������һ��һ��һ��"),TEXT("����19��������������һ��"),
	TEXT("����3������һ��һ��һ��"),TEXT("����20��������������һ��"),
	TEXT("����4������һ��һ��һ��"),TEXT("����21��������������һ��"),
	TEXT("����5������һ��һ��һ��"),TEXT("����22��������������һ��"),
	TEXT("����6��һ������һ��һ��"),TEXT("����23����������һ������"),
	TEXT("����7����������һ��һ��"),TEXT("����24����������һ������"),
	TEXT("����8����������һ��һ��"),TEXT("����25����������һ������"),
	TEXT("����9����������һ��һ��"),TEXT("����26����������һ������"),
	TEXT("����10����������һ��һ��"),TEXT("����27������һ����������"),
	TEXT("����11������һ������һ��"),TEXT("����28������һ����������"),
	TEXT("����12������һ������һ��"),TEXT("����29������һ����������"),
	TEXT("����13������һ������һ��"),TEXT("����30������һ����������"),
	TEXT("����14������һ������һ��"),TEXT("����31������������������"),
	TEXT("����15������һ��һ������"),TEXT("����32������������������"),
	TEXT("����16������һ��һ������"),TEXT("����33������������������"),
	TEXT("����17������һ��һ������"),TEXT("����34������������������"),
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
		MessageBox(NULL, TEXT("�������֧�� Windows NT ����ϵͳ!"),
			szWinName, MB_ICONERROR);
		return 0;
	}

	hwnd = CreateWindow(szWinName, TEXT("¥�̹���ϵͳ"),
		WS_OVERLAPPEDWINDOW,
		263, 127,
		872, 532,
		NULL, NULL, hInstance, NULL);

	HDC hdc = GetDC(hwnd);

	LOGFONT LogFont;
	memset(&LogFont, 0, sizeof(LOGFONT));
	lstrcpy(LogFont.lfFaceName, TEXT("����"));
	LogFont.lfWeight = 400;
	LogFont.lfHeight = GetDeviceCaps(hdc, LOGPIXELSY) * 12 / 72; // �����С
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
	wsprintf(font.lfFaceName, TEXT("΢���ź�"));
	hFontBlack = CreateFontIndirect(&font);

	ReleaseDC(hwnd, hdc);

	// ��������"���塰
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
//�����ڴ��ڹ���

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
		InitCommonControls();//��ʼ��listview����

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
		
		hwndMainTitle = CreateWindow(TEXT("static"), TEXT("¥ ��"),
			WS_VISIBLE | WS_CHILD | SS_CENTER, 
			0, 0, 0, 0, hwnd, (HMENU)(ID_MAINLISTTITLE),
			(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
		SendMessage(hwndMainTitle, WM_SETFONT, 
			(WPARAM)GetStockObject(OEM_FIXED_FONT), 0);

		hwndButtonDel = CreateWindow(TEXT("button"), TEXT("ɾ��"),
			WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
			0, 0, 0, 0, hwnd, (HMENU)(ID_BUTTONDEL),
			hInstance, NULL);
		SendMessage(hwndButtonDel, WM_SETFONT,
			(WPARAM)GetStockObject(OEM_FIXED_FONT), 0);

		hwndButtonEdit = CreateWindow(TEXT("button"), TEXT("�༭"),
			WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
			0, 0, 0, 0, hwnd, (HMENU)(ID_BUTTONEDIT),
			hInstance, NULL);
		SendMessage(hwndButtonEdit, WM_SETFONT,
			(WPARAM)GetStockObject(OEM_FIXED_FONT), 0);

		hwndButtonEnter = CreateWindow(TEXT("button"), TEXT("����"),
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

			//ѭ������¥����
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
				iAns = MessageBox(hwnd, TEXT("ȷ��Ҫɾ����¥����"),
					TEXT("��ʾ"), MB_OKCANCEL);
				if (iAns == IDOK)
				{
					iSaveState = FALSE;
					DelComInList(iComNum, &pHead);

					if (pHead)
					{
						lstrcpy(comNow, pHead->name);
						UpdateData(&pHead);

						//�洢�������Ƶ�Com��ַ
						pComPaint = (Community*)malloc(sizeof(Community));
						*pComPaint = *pHead;
					}
					else pComPaint = NULL;

					//ȷ����Ҫ���Ƶļ�����Ч����
					iPaintState = PA_FIRSTEPCOM;
					InvalidateRect(hwnd, NULL, TRUE);
				}
				else return 0;
				break;

			case PA_SECSTEPBUI:
			case PA_FIRSTEPBUI:

				iAns = MessageBox(hwnd, TEXT("ȷ��Ҫɾ����¥����"),
					TEXT("��ʾ"), MB_OKCANCEL);
				if (iAns == IDOK)
				{
					iSaveState = FALSE;
					DelBuiInList(iComNum, buiNow, &pHead);

					for (pTailCom = pHead; pTailCom; pTailCom = pTailCom->nextCommunity)
						if (lstrcmp(comNow, pTailCom->name) == 0)
						{
							UpdateData(&pHead);

							//�洢�������Ƶ�Com��ַ
							pComPaint = (Community*)malloc(sizeof(Community));
							*pComPaint = *pTailCom;
							if (pTailCom->buildings)
								buiNow = pTailCom->buildings->buildingNum;
							else buiNow = 0;
							break;
						}

					//ȷ����Ҫ���Ƶļ�����Ч����
					iPaintState = PA_FIRSTEPCOM;
					InvalidateRect(hwnd, NULL, TRUE);
				}
				else return 0;
				break;

			case PA_SECSTEPROOM:
				iAns = MessageBox(hwnd, TEXT("ȷ��Ҫɾ���÷�����"),
					TEXT("��ʾ"), MB_OKCANCEL);
				if (iAns == IDOK)
				{
					iSaveState = FALSE;
					DelRoomInList(iComNum, buiNow, roomNow, &pHead);

					for (pTailCom = pHead; pTailCom; pTailCom = pTailCom->nextCommunity)
						if (lstrcmp(comNow, pTailCom->name) == 0)
						{
							UpdateData(&pHead);

							//�洢�������Ƶ�Com��ַ
							pComPaint = (Community*)malloc(sizeof(Community));
							*pComPaint = *pTailCom;
							for (pTailBui = pTailCom->buildings; pTailBui != NULL;
								pTailBui = pTailBui->nextBuilding)
								if (buiNow == pTailBui->buildingNum)
								{
									//�洢��Ҫ���Ƶ�Bui��ַ
									pBuiPaint = (Building*)malloc(sizeof(Building));
									*pBuiPaint = *pTailBui;
									break;
								}
							break;
						}

					//ȷ����Ҫ���Ƶļ�����Ч����
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

					//�洢�������Ƶ�Com��ַ
					pComPaint = (Community*)malloc(sizeof(Community));
					*pComPaint = *pHead;
				}
				else pComPaint = NULL;

				//ȷ����Ҫ���Ƶļ�����Ч����
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

						//�洢�������Ƶ�Com��ַ
						pComPaint = (Community*)malloc(sizeof(Community));
						*pComPaint = *pTailCom;
						for (pTailBui = pTailCom->buildings; pTailBui != NULL;
							pTailBui = pTailBui->nextBuilding)
							if (buiNow == pTailBui->buildingNum)
							{
								//�洢��Ҫ���Ƶ�Bui��ַ
								pBuiPaint = (Building*)malloc(sizeof(Building));
								*pBuiPaint = *pTailBui;
								break;
							}
						break;
					}

				//ȷ����Ҫ���Ƶļ�����Ч����
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

						//�洢�������Ƶ�Com��ַ
						pComPaint = (Community*)malloc(sizeof(Community));
						*pComPaint = *pTailCom;

						for (pTailBui = pTailCom->buildings; pTailBui != NULL;
							pTailBui = pTailBui->nextBuilding)
							if (buiNow == pTailBui->buildingNum)
							{
								//�洢��Ҫ���Ƶ�Bui��ַ
								pBuiPaint = (Building*)malloc(sizeof(Building));
								*pBuiPaint = *pTailBui;

								for (pTailRoom = pTailBui->rooms; pTailRoom != NULL;
									pTailRoom = pTailRoom->nextRoom)
									if (roomNow == pTailRoom->roomNum)
									{
										//�洢��Ҫ���Ƶ�Bui��ַ
										pRoomPaint = (Room*)malloc(sizeof(Room));
										*pRoomPaint = *pTailRoom;
									}
								break;
							}
						break;
					}

				//ȷ����Ҫ���Ƶļ�����Ч����
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

				//��ȡѡ�����ı�
				SendMessage(hwndMainList, LB_GETTEXT,
					iIndex, (LPARAM)szBuffer);

				for (pTailCom = pHead; pTailCom; pTailCom = pTailCom->nextCommunity)
					if (lstrcmp(szBuffer, pTailCom->name) == 0)
					{
						//�洢�������Ƶ�Com��ַ
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

				//��ȡѡ�������¥��
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

				//��ȡѡ���������
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

			//���� IDM_FILE �ļ�ϵ�й���

		case IDM_FILE_SAVE:				//�ļ�-��������*

			if (szFileName[0])
			{
				if (FileWriteWnd(hwnd, szFileName))
				{
					iSaveState = TRUE;
					return 1;
				}
				else
				{
					wsprintf(szBuffer, TEXT("�޷����ļ� %s"), szFileName);
					MessageBox(hwnd, szBuffer, TEXT("��ʾ"), MB_OK);
					return 0;
				}
			}
			//���szFileName�����ڣ�����ת�������Ϊ��

		case IDM_FILE_SAVE_AS:			//�ļ�-�������Ϊ*

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
					wsprintf(szBuffer, TEXT("�޷����ļ� %s"), szFileName);
					MessageBox(hwnd, szBuffer, TEXT("��ʾ"), MB_OK);
					return 0;
				}
			}
			return 0;

		case IDM_FILE_CREATE:			//�ļ�-�½�*

			if (!AskConfirm(hwnd, iSaveState))
				return 0;

			szFileName[0] = '\0';
			szFileTitle[0] = '\0';

			SetTitle(hwnd, szFileTitle);
			iSaveState = TRUE;
			return 0;

		case IDM_FILE_IMPORT:			//�ļ�-��������*

			//ѯ���û��Ƿ���Ҫ����
			if (!AskConfirm(hwnd, iSaveState))
				return 0;

			if (FileImportDlg(hwnd, szFileName, szFileTitle))
				if (!FileReadWnd(hEdit, szFileName))
				{
					wsprintf(szBuffer, TEXT("�޷����ļ� %s"),
						szFileTitle[0] ? szFileTitle : TEXT("�ޱ���"));
					MessageBox(hwnd, szBuffer, TEXT("¥�̹���ϵͳ"),
						MB_OK | MB_ICONEXCLAMATION);

					szFileName[0] = '\0';
					szFileTitle[0] = '\0';
				}
				else
				{
					//��ʼ��ѡ��
					lstrcpy(comNow, pHead->name);
					if (pHead->buildings != NULL)
						buiNow = pHead->buildings->buildingNum;

					//ȷ�Ͻ�Ҫ���Ƶĳ�Ա
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

					//��ʼ����
					iPaintState = PA_FIRSTEPCOM;
					InvalidateRect(hwnd, NULL, TRUE);
				}


			SetTitle(hwnd, szFileTitle);
			iSaveState = TRUE;
			return 0;

		case IDM_FILE_EXIT:				//�ļ�-�˳�����*

			SendMessage(hwnd, WM_CLOSE, 0, 0);
			break;

			//���� IDM_EDIT �༭���ֹ���

		case IDM_EDIT_CHANGE_DATA:		//�༭-����-��������

			SendMessage(hwndButtonEdit, BM_CLICK, 0, 0);
			break;

		case IDM_EDIT_ADD_ROOM:			//�༭-���-����·���*

			DialogBox(hInstance, MAKEINTRESOURCE(IDD_ADD_ROOM),
				hwnd, AddRoomDlgProc);
			if (bRePaint)
			{
				//���������ṹ�Ž�����^_^
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
									MessageBox(hwnd, TEXT("�÷���������ϵͳ��"),
										TEXT("����"), MB_OK | MB_ICONWARNING);
									break;
								}
								UpdateData(&pHead);

								//�洢�������Ƶ�Bui��ַ
								pBuiPaint = (Building*)malloc(sizeof(Building));
								*pBuiPaint = *pTailBui;
								break;
							}
					}
				}

				//ȷ����Ҫ���Ƶļ�����Ч����
				iPaintState = PA_SECSTEPBUI;
				InvalidateRect(hwnd, NULL, TRUE);

				iSaveState = FALSE;
			}
			break;

		case IDM_EDIT_ADD_BUID:			//�༭-���-�����¥��*

			DialogBox(hInstance, MAKEINTRESOURCE(IDD_ADD_BUI),
				hwnd, AddBuiDlgProc);

			if (bRePaint)
			{
				//�����¥���ṹ�Ž�����^_^
				for (pTailCom = pHead; pTailCom; pTailCom = pTailCom->nextCommunity)
					if (lstrcmp(pbBuffer[0].inCom, pTailCom->name) == 0)
					{
						if (CheckSameBui(pbBuffer[0].inCom, pbBuffer[0].buildingNum, &pHead))
							AddBuiToList(pbBuffer[0], &pHead, pTailCom->communityNum);
						else
						{
							MessageBox(hwnd, TEXT("��¥��������ϵͳ��"),
								TEXT("����"), MB_OK | MB_ICONWARNING);
							break;
						}
						UpdateData(&pHead);

						//�洢�������Ƶ�Com��ַ
						pComPaint = (Community*)malloc(sizeof(Community));
						*pComPaint = *pTailCom;
						buiNow = pbBuffer[0].buildingNum;
						break;
					}
				
				//ȷ����Ҫ���Ƶļ�����Ч����
				iPaintState = PA_FIRSTEPCOM;
				InvalidateRect(hwnd, NULL, TRUE);

				iSaveState = FALSE;
			}
			break;

		case IDM_EDIT_ADD_COM:			//�༭-���-�����¥��*

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
						MessageBox(hwnd, TEXT("��¥��������ϵͳ��"),
							TEXT("����"), MB_OK | MB_ICONWARNING);
						break;
					}
					UpdateData(&pHead);
					for (pTailCom = pHead; pTailCom; pTailCom = pTailCom->nextCommunity)
						if (lstrcmp(pcBuffer[0].name, pTailCom->name) == 0)
						{
							//�洢�������Ƶ�Com��ַ
							pComPaint = (Community*)malloc(sizeof(Community));
							*pComPaint = *pTailCom;
							lstrcpy(comNow, pcBuffer[0].name);
							break;
						}
				}

				//ȷ����Ҫ���Ƶļ�����Ч����
				iPaintState = PA_FIRSTEPCOM;
				InvalidateRect(hwnd, NULL, TRUE);

				iSaveState = FALSE;
			}

			break;

		case IDM_EDIT_DEL_ROOM:			//�༭-ɾ��-ɾ������

			DialogBox(hInstance, MAKEINTRESOURCE(IDD_DEL_ROOM),
				hwnd, DelRoomDlgProc);
			if (bRePaint)
			{
				for (pTailCom = pHead; pTailCom; pTailCom = pTailCom->nextCommunity)
					if (lstrcmp(comNow, pTailCom->name) == 0)
					{
						UpdateData(&pHead);

						//�洢�������Ƶ�Com��ַ
						pComPaint = (Community*)malloc(sizeof(Community));
						*pComPaint = *pTailCom;
						for (pTailBui = pTailCom->buildings; pTailBui != NULL;
							pTailBui = pTailBui->nextBuilding)						
							if (buiNow == pTailBui->buildingNum)
							{
								//�洢��Ҫ���Ƶ�Bui��ַ
								pBuiPaint = (Building*)malloc(sizeof(Building));
								*pBuiPaint = *pTailBui;
							}			
						break;
					}

				//ȷ����Ҫ���Ƶļ�����Ч����
				iPaintState = PA_SECSTEPBUI;
				InvalidateRect(hwnd, NULL, TRUE);

				iSaveState = FALSE;
			}
			break;

		case IDM_EDIT_DEL_BUID:			//�༭-ɾ��-ɾ��¥��

			DialogBox(hInstance, MAKEINTRESOURCE(IDD_DEL_BUI),
				hwnd, DelBuiDlgProc);
			if (bRePaint)
			{
				for (pTailCom = pHead; pTailCom; pTailCom = pTailCom->nextCommunity)
					if (lstrcmp(comNow, pTailCom->name) == 0)
					{
						UpdateData(&pHead);

						//�洢�������Ƶ�Com��ַ
						pComPaint = (Community*)malloc(sizeof(Community));
						*pComPaint = *pTailCom;
						if(pTailCom->buildings)
							buiNow = pTailCom->buildings->buildingNum;
						else buiNow = 0;
						break;
					}

				//ȷ����Ҫ���Ƶļ�����Ч����
				iPaintState = PA_FIRSTEPCOM;
				InvalidateRect(hwnd, NULL, TRUE);

				iSaveState = FALSE;
			}
			break;

		case IDM_EDIT_DEL_COM:			//�༭-ɾ��-ɾ��¥��

			DialogBox(hInstance, MAKEINTRESOURCE(IDD_DEL_COM),
				hwnd, DelComDlgProc);
			if (bRePaint)
			{
				if (pHead)
				{
					lstrcpy(comNow, pHead->name);
					UpdateData(&pHead);

					//�洢�������Ƶ�Com��ַ
					pComPaint = (Community*)malloc(sizeof(Community));
					*pComPaint = *pHead;
				}
				else pComPaint = NULL;
				
				//ȷ����Ҫ���Ƶļ�����Ч����
				iPaintState = PA_FIRSTEPCOM;
				InvalidateRect(hwnd, NULL, TRUE);

				iSaveState = FALSE;
			}

			break;

			//���� IDM_SEARCH ��ѯ���ֹ���

		case IDM_SEARCH:			//��ѯ-��Ϣ����

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

			//���� IDM_HELP �������ֹ���

		case IDM_HELP_ABOUT:			//�������

			DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX),
				hwnd, AboutDlgProc);
			break;

		case ID_MAINLIST:

			if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				iIndex = SendMessage(hwndMainList, LB_GETCURSEL, 0, 0);

				//��ȡѡ�����ı�
				SendMessage(hwndMainList, LB_GETTEXT, iIndex, (LPARAM)szBuffer);

				if (SEARCHING && !lstrcmp(szBuffer, TEXT("���˳�������")))
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
					//��ȡѡ�������¥��		
					for (pTailBui = pBuiSearch, i = 0; pTailBui != NULL;
						pTailBui = pTailBui->nextBuilding, i++)
						if (i == iIndex)
							break;

					pBuiPaint = (Building*)malloc(sizeof(Building));
					*pBuiPaint = *pTailBui;

					//��������Ѱ������¥��
					for (pTailCom = pHead; pTailCom;
						pTailCom = pTailCom->nextCommunity)
						if (lstrcmp(pTailCom->name, pBuiPaint->inCom) == 0)
						{
							//�洢�������Ƶ�Com��ַ
							pComPaint = (Community*)malloc(sizeof(Community));
							*pComPaint = *pTailCom;
						}

					InvalidateRect(hwnd, NULL, TRUE);
					break;
				}
				else if (iSearch == SEARCHROOM)
				{
					//��ȡѡ�������¥��		
					for (pTailRoom = pRoomSearch, i = 0; pTailRoom != NULL;
						pTailRoom = pTailRoom->nextRoom, i++)
						if (i == iIndex)
							break;

					pRoomPaint = (Room*)malloc(sizeof(Room));
					*pRoomPaint = *pTailRoom;

					//��������Ѱ������¥�̡�¥��
					for (pTailCom = pHead; pTailCom;
						pTailCom = pTailCom->nextCommunity)
						if (lstrcmp(pTailCom->name, pRoomPaint->inCom) == 0)
						{
							//�洢�������Ƶ�Com��ַ
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

				//��������¥����
				lstrcpy(comNow, szBuffer);

				//��������Ѱ�Ҹ�¥��
				for (pTailCom = pHead; pTailCom;
					pTailCom = pTailCom->nextCommunity)
					if (lstrcmp(pTailCom->name, szBuffer) == 0)
					{
						//�洢�������Ƶ�Com��ַ
						pComPaint = (Community*)malloc(sizeof(Community));
						*pComPaint = *pTailCom;
					}

				//ȷ����Ҫ���Ƶļ�����Ч����
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

			//��˫���������˵�����ʾ��һ���˵�

			if ((((NMHDR *)lParam)->code) == NM_DBLCLK)
			{
				//��ȡѡ�����ı�
				NM_LISTVIEW *pNMListView = (NM_LISTVIEW *)lParam;
				iIndex = pNMListView->iItem;

				ListView_GetItemText(hwndSubList, iIndex, 0, szBuffer, 4096);

				//���μ��б���Ϊ¥��ʱ��˫��������һ��

				if (iPaintState == PA_FIRSTEPBUI
					|| iPaintState == PA_FIRSTEPCOM)
				{
					//�ж�˫�����Ƿ�������
					if (iIndex<0 || iIndex>ListView_GetItemCount(hwndSubList))
						break;

					//��ȡǰ��λ¥����
					szBuffer[3] = '\0\0';
					iBuiNum = _ttoi(szBuffer);

					buiNow = iBuiNum;

					//��������Ѱ������¥��
					for (pTailCom = pHead; pTailCom;
						pTailCom = pTailCom->nextCommunity)
						if (lstrcmp(pTailCom->name, comNow) == 0)
						{
							//�洢�������Ƶ�Com��ַ
							pComPaint = (Community*)malloc(sizeof(Community));
							*pComPaint = *pTailCom;

							//��������Ѱ�Ҹ�¥��
							for (pTailBui = pTailCom->buildings; pTailBui;
								pTailBui = pTailBui->nextBuilding)
								if (pTailBui->buildingNum == buiNow)
								{
									//�洢�������Ƶ�Bui��ַ
									pBuiPaint = (Building*)malloc(sizeof(Building));
									*pBuiPaint = *pTailBui;
									buiNow = pBuiPaint->buildingNum;
									break;
								}
							break;	//����ѭ��
						}

					//������һ��
					iPaintState = PA_SECSTEPBUI;
					InvalidateRect(hwnd, NULL, TRUE);
				}

				//��ǰ�μ��б���Ϊ����ʱ������Ӧ˫����һ��������һ����˫����Ϣ

				else if (iPaintState == PA_SECSTEPBUI
					|| iPaintState == PA_SECSTEPROOM)
				{
					//�ж�˫�����Ƿ�Ϊ��һ��
					if (iIndex != 0)
						break;

					//��������Ѱ������¥��
					for (pTailCom = pHead; pTailCom;
						pTailCom = pTailCom->nextCommunity)
						if (lstrcmp(pTailCom->name, comNow) == 0)
						{
							//�洢�������Ƶ�Com��ַ
							pComPaint = (Community*)malloc(sizeof(Community));
							*pComPaint = *pTailCom;

							//��������Ѱ�Ҹ�¥��
							for (pTailBui = pTailCom->buildings; pTailBui;
								pTailBui = pTailBui->nextBuilding)
								if (pTailBui->buildingNum == buiNow)
								{
									//�洢�������Ƶ�Bui��ַ
									pBuiPaint = (Building*)malloc(sizeof(Building));
									*pBuiPaint = *pTailBui;
									buiNow = pBuiPaint->buildingNum;
									break;
								}
							break;	//����ѭ��
						}

					//������һ��
					iPaintState = PA_FIRSTEPBUI;
					InvalidateRect(hwnd, NULL, TRUE);
				}
			}

			//���������˵����ı���ʾ����

			else if ((((NMHDR *)lParam)->code) == NM_CLICK)
			{
				//��ȡѡ�����ı�
				NM_LISTVIEW *pNMListView = (NM_LISTVIEW *)lParam;
				iIndex = pNMListView->iItem;

				//�жϵ������Ƿ�������
				if (iIndex<0 || iIndex>ListView_GetItemCount(hwndSubList))
					break;

				ListView_GetItemText(hwndSubList, iIndex, 0, szBuffer, 4096);

				//�����б��Ϊbuilding�����

				if (iPaintState == PA_FIRSTEPBUI
					|| iPaintState == PA_FIRSTEPCOM)
				{
					//��ȡǰ��λ¥����
					szBuffer[3] = '\0\0';
					iBuiNum = _ttoi(szBuffer);

					//��������Ѱ������¥��
					for (pTailCom = pHead; pTailCom;
						pTailCom = pTailCom->nextCommunity)
						if (lstrcmp(pTailCom->name, comNow) == 0)
						{
							//�洢�������Ƶ�Com��ַ
							pComPaint = (Community*)malloc(sizeof(Community));
							*pComPaint = *pTailCom;

							//��������Ѱ�Ҹ�¥��
							for (pTailBui = pTailCom->buildings; pTailBui;
								pTailBui = pTailBui->nextBuilding)
								if (pTailBui->buildingNum == iBuiNum)
								{
									//�洢�������Ƶ�Bui��ַ
									pBuiPaint = (Building*)malloc(sizeof(Building));
									*pBuiPaint = *pTailBui;
									buiNow = pBuiPaint->buildingNum;
									break;
								}
							break;	//����ѭ��
						}

					//ȷ����Ҫ���Ƶļ�����Ч����
					iPaintState = PA_FIRSTEPBUI;
					InvalidateRect(hwnd, NULL, TRUE);
				}

				//�����б��ΪRoom�����

				else if (iPaintState == PA_SECSTEPBUI
					|| iPaintState == PA_SECSTEPROOM)
				{

					//������¥��������ʾ¥����Ϣ

					if (iIndex == 0)
					{
						iPaintState = PA_SECSTEPBUI;
						InvalidateRect(hwnd, NULL, TRUE);
						break;
					}

					//��ȡǰ��λ�����
					szBuffer[3] = '\0\0';
					iRoomNum = _ttoi(szBuffer);

					//��������Ѱ������¥��
					for (pTailCom = pHead; pTailCom;
						pTailCom = pTailCom->nextCommunity)
						if (lstrcmp(pTailCom->name, comNow) == 0)
						{
							//�洢�������Ƶ�Com��ַ
							pComPaint = (Community*)malloc(sizeof(Community));
							*pComPaint = *pTailCom;

							//��������Ѱ�Ҹ�¥��
							for (pTailBui = pTailCom->buildings; pTailBui;
								pTailBui = pTailBui->nextBuilding)
								if (pTailBui->buildingNum == buiNow)
								{
									//�洢�������Ƶ�Bui��ַ
									pBuiPaint = (Building*)malloc(sizeof(Building));
									*pBuiPaint = *pTailBui;

									for (pTailRoom = pTailBui->rooms; pTailRoom;
										pTailRoom = pTailRoom->nextRoom)
									{
										if (pTailRoom->roomNum == iRoomNum)
										{
											//�洢�������Ƶ�Room��ַ
											pRoomPaint = (Room*)malloc(sizeof(Room));
											*pRoomPaint = *pTailRoom;
										}
									}

								}
							break;	//����ѭ��
						}

					//ȷ����Ҫ���Ƶļ�����Ч����
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

		//�����������
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
			//������Ϣ
			SendMessage(hwndMainList, LB_RESETCONTENT, 0, 0);
			ListView_DeleteAllItems(hwndSubList);

			SetWindowText(hwndMainTitle, TEXT("�������"));
			
			switch (iSearch)
			{
			case SEARCHCOM:

				//������������������Ϣ
				for (pTailCom = pComSearch; pTailCom;
					pTailCom = pTailCom->nextCommunity)
				{
					SendMessage(hwndMainList, LB_ADDSTRING,
						0, (LPARAM)pTailCom->name);
				}
				SendMessage(hwndMainList, LB_ADDSTRING,
					0, (LPARAM)TEXT("���˳�������"));

				if (pComPaint)
				{
					DrawComInf(hwnd, hdc, *pComPaint);
					//iIndex = 0;
				}
				break;
				
			case SEARCHBUI:

				//������������������Ϣ
				for (pTailBui = pBuiSearch; pTailBui;
					pTailBui = pTailBui->nextBuilding)
				{
					wsprintf(szBuffer, TEXT("%s%d��"), 
						pTailBui->inCom, pTailBui->buildingNum);
					SendMessage(hwndMainList, LB_ADDSTRING,
						0, (LPARAM)szBuffer);
				}
				SendMessage(hwndMainList, LB_ADDSTRING,
					0, (LPARAM)TEXT("���˳�������"));

				if (pBuiPaint)
				{
					DrawBuiInf(hwnd, hdc, *pBuiPaint);
					//iIndex = 0;
				}
				break;

			case SEARCHROOM:

				//������������������Ϣ
				for (pTailRoom = pRoomSearch; pTailRoom;
					pTailRoom = pTailRoom->nextRoom)
				{
					wsprintf(szBuffer, TEXT("%s%d��%d"),
						pTailRoom->inCom, pTailRoom->buildingNum,
						pTailRoom->roomNum);
					SendMessage(hwndMainList, LB_ADDSTRING,
						0, (LPARAM)szBuffer);
				}
				SendMessage(hwndMainList, LB_ADDSTRING,
					0, (LPARAM)TEXT("���˳�������"));

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
//�����ڡ�¥�̹���ϵͳ������About���Ի��򴰿ڹ���

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
//���ܣ����û��˳�ϵͳʱ�����ļ�δ���棬����ʾ���Ƿ���Ҫ����
//     ���ļ��Ѿ����棬��ֱ���˳�
//�������˳��Ĵ��ھ��������״̬����0Ϊ�ѱ��棬0Ϊδ���棩
//���أ�����Ҫ�˳�ϵͳ���򷵻�TRUE��������Ҫ���򷵻�FALSE

BOOL AskConfirm(HWND hwnd, int iSaveState)
{
	int iAns;
	if (!iSaveState)
	{
		iAns = MessageBox(hwnd, TEXT("�Ƿ�Ҫ�������ĸ���"), TEXT("��ʾ"), MB_YESNOCANCEL);
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
//���ڻ������ֹ��캯��
//���ܣ��������ڵĻ������֣������κ���Ϣ
//���������ھ��,�豸����,��Ļ����Ļ��
//���أ����ݲ��ֵ�RECT����

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
	SetDCBrushColor(hdc, RGB(137, 189, 255));   //����ɫΪ����ɫ

	Rectangle(hdc, rectData.left, rectData.top,
		rectData.right, rectData.bottom);

	SelectObject(hdc, GetStockObject(WHITE_BRUSH));

	return rectData;
}


///////////////////////////////////////////////////////
//create3DepthList
//���ܣ�����һ����������
//������1����һ��������
//     2��ÿ����һ�������ڶ�������
//     3��ÿ���ڶ�����������������
//�������أ����ظ����������ͷָ��

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
//���ܣ�����һ��room���Ƚ��ȳ��ĵ�������
//������������
//���أ�������ͷָ��

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
//���ܣ�����һ��building���Ƚ��ȳ��ĵ�������
//������������
//���أ�������ͷָ��

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
//���ܣ�����һ��room���Ƚ��ȳ��ĵ�������
//������������
//���أ�������ͷָ��

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
//�������¥�̡���AddCommunity���Ի��򴰿ڹ���

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

			//������OKʱ�������������ݴ���洢��	

			iLength[0] = SendMessage(GetDlgItem(hDlg, IDC_ADD_COM_NAME),
				EM_GETLINE, 0, (LPARAM)szBuffer[0]);
			iLength[1] = SendMessage(GetDlgItem(hDlg, IDC_ADD_COM_ADDR),
				EM_GETLINE, 2, (LPARAM)szBuffer[1]);
			iLength[2] = SendMessage(GetDlgItem(hDlg, IDC_ADD_COM_PHONE),
				EM_GETLINE, 3, (LPARAM)szBuffer[2]);
			iLength[3] = SendMessage(GetDlgItem(hDlg, IDC_ADD_COM_PERSON),
				EM_GETLINE, 4, (LPARAM)szBuffer[3]);

			//����ÿһ��ǿ�

			for (i = 0; i < 4; i++)
			{
				if (!iLength[i])
				{
					MessageBox(hDlg, TEXT("��������������Ϣ��"), TEXT("��ʾ"), MB_OK);
					return FALSE;
				}
				else {
					*(szBuffer[i] + iLength[i]) = '\0\0';
				}
			}

			//����ռ䣬�������ݷ���һ��community�ṹ��

			pNewCom = (Community*)malloc(sizeof(Community));

			lstrcpy(pNewCom->name, szBuffer[0]);
			lstrcpy(pNewCom->address, szBuffer[1]);
			lstrcpy(pNewCom->phone, szBuffer[2]);
			lstrcpy(pNewCom->host.name, szBuffer[3]);

			pNewCom->communityNum = iAutoComNum++;
			pNewCom->avgPrice = 0.0;

			//�����¥�̴��ȫ�ֻ�������֮���������
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
//�������¥������AddBuilding���Ի��򴰿ڹ���

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

		//���ñ༭���ʼ����
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

			//������OKʱ�������������ݴ���洢��	

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

			//����ÿһ��ǿ�

			for (i = 0; i < 5; i++)
			{
				if (!iLength[i])
				{
					MessageBox(hDlg, TEXT("��������������Ϣ��"), TEXT("��ʾ"), MB_OK);
					return FALSE;
				}
				else {
					*(szBuffer[i] + iLength[i]) = '\0\0';
				}
			}

			//����ռ䣬�������ݷ���һ��building�ṹ��

			pNewBui = (Building*)malloc(sizeof(Building));

			lstrcpy(pNewBui->inCom, szBuffer[0]);
			pNewBui->buildingNum = _ttoi(szBuffer[1]);
			pNewBui->numberOfFloors = _ttoi(szBuffer[2]);
			lstrcpy(pNewBui->host.phone, szBuffer[3]);
			lstrcpy(pNewBui->host.name, szBuffer[4]);

			pNewBui->avgPrice = 0.0;
			pNewBui->numberOfRooms = 0;

			//ȷ���Ƿ���������ַ
			for (ptailCom = pHead; ptailCom; ptailCom = ptailCom->nextCommunity)
				if (lstrcmp(pNewBui->inCom, ptailCom->name) == 0)
					fAddBuiState = SUCCEED;

			//�����Ƿ��ж�Ӧ¥��
			if (fAddBuiState == FAIL)
			{
				MessageBox(hDlg, TEXT("�������¥�̲�����"), TEXT("��ʾ"), MB_OK);
				break;
			}

			//������ṹ����¥��ȫ�ֻ�����
			pbBuffer[0] = *pNewBui;

			bRePaint = TRUE;	//��Ȩ���½���
			EndDialog(hDlg, TRUE);
			return TRUE;

		case IDCANCEL:
			bRePaint = FALSE;	//��ֹ���½���
			EndDialog(hDlg, FALSE);
			return TRUE;
		}

		break;
	}

	return FALSE;
}


///////////////////////////////////////////////////////
//������·��䡱��AddRoom���Ի��򴰿ڹ���

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

		//��ComboBox�����������
		for (i = 0; i < 34; i++)
			SendMessage(GetDlgItem(hDlg, IDC_ADD_ROOM_STYLE),
				CB_ADDSTRING, 0, (LPARAM)szComboBoxData[i]);

		//���ñ༭���ʼ����
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

			//������OKʱ�������������ݴ���洢��	

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

			//����ÿһ��ǿ�

			for (i = 0; i < 5; i++)
			{
				if (!iLength[i])
				{
					MessageBox(hDlg, TEXT("��������������Ϣ��"), TEXT("��ʾ"), MB_OK);
					return FALSE;
				}
				else {
					*(szBuffer[i] + iLength[i]) = '\0\0';
				}
			}

			//����ռ䣬�������ݷ���һ��Room�ṹ��

			pNewRoom = (Room*)malloc(sizeof(Room));

			lstrcpy(pNewRoom->inCom, szBuffer[0]);
			pNewRoom->buildingNum = _ttoi(szBuffer[1]);
			pNewRoom->roomNum = _ttoi(szBuffer[2]);
			pNewRoom->roomSize = _ttof(szBuffer[3]);
			pNewRoom->roomPrice = _ttof(szBuffer[4]);
			lstrcpy(pNewRoom->roomType, szBuffer[5]);
			pNewRoom->roomState = iSold;

			//ȷ���Ƿ���������ַ
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

			//�����Ƿ��ж�Ӧ¥�̺�¥��
			if (fAddRoomState == FAIL)
			{
				MessageBox(hDlg, TEXT("�������¥�̻�¥��������"), TEXT("��ʾ"), MB_OK);
				break;
			}

			//������ṹ����¥��ȫ�ֻ�����
			prBuffer[0] = *pNewRoom;

			bRePaint = TRUE;	//��Ȩ���½���
			EndDialog(hDlg, TRUE);
			return TRUE;

		case IDCANCEL:
			bRePaint = FALSE;	//��ֹ���½���
			EndDialog(hDlg, FALSE);
			return TRUE;
		}

		break;
	}

	return FALSE;
}


///////////////////////////////////////////////////////
//AddComToList
//���ܣ������������һ��community�ڵ�
//��������Ҫ��ӵ�Community�ṹ�壨��Ӷ��������ͷָ��.ȫ�ֱ����ṩ��
//���أ����ظ��º�����ͷָ��

BOOL AddComToList(Community newCom, Community **head)
{
	Community *begin, *end;

	//û��¥�̵����
	if (*head == NULL)
	{
		*head = (Community*)malloc(sizeof(Community));
		**head = newCom;

		//��ʼ������ָ�룬��Ҫ��
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

	//��ʼ������ָ�룬��Ҫ����
	(*begin->nextCommunity).nextCommunity = NULL;
	(*begin->nextCommunity).buildings = NULL;
	return TRUE;
}

///////////////////////////////////////////////////////
//AddBuiToList
//���ܣ������������һ��building�ڵ�
//��������Ҫ��ӵ�Building�ṹ�壬��Ӷ��������ͷָ�룬����¥�̺�
//���أ��ɹ��򷵻�TRUE��ʧ�ܷ���FALSE

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

			//��¥����û��¥�������
			if (tailCom->buildings == NULL)
			{
				tailCom->buildings =
					(Building*)malloc(sizeof(Building));
				*tailCom->buildings = newBui;

				//����δ��ʼ���Ľṹ����һ����ʼ������Ҫ����
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

			//��ʼ������ָ��
			begBui->nextBuilding->nextBuilding = NULL;
			begBui->nextBuilding->rooms = NULL;
			return TRUE;
		}
	}
	
	return FALSE;
}


///////////////////////////////////////////////////////
//AddRoomToList
//���ܣ������������һ��Room�ڵ�
//��������Ҫ��ӵ�Room�ṹ�壬��Ӷ��������ͷָ�룬����¥�̺ţ�����¥����
//���أ��ɹ��򷵻�TRUE��ʧ�ܷ���FALSE

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
			//��¥����û��¥�������
			if (tailCom->buildings == NULL)
				return FALSE;

			for (tailBui = tailCom->buildings; tailBui != NULL;
				tailBui = tailBui->nextBuilding)
			{
				if (tailBui->buildingNum == buiNum)
				{
					//��¥����û�з�������
					if (tailBui->rooms == NULL)
					{
						tailBui->rooms =
							(Room*)malloc(sizeof(Room));
						*tailBui->rooms = newRoom;

						//����δ��ʼ���Ľṹ����һ����ʼ������Ҫ����
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

					//��ʼ������ָ��
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
//���ܣ�����һ��ֻ��bui��ɵ�search
//��������Ҫ��ӵ�Building�ṹ�壬��Ӷ��������ͷָ��
//���أ��ɹ��򷵻�TRUE��ʧ�ܷ���FALSE

BOOL AddBuiToSearch(Building newCom, Building **head)
{
	Building *begin, *end;

	//û��¥�̵����
	if (*head == NULL)
	{
		*head = (Building*)malloc(sizeof(Building));
		**head = newCom;

		//��ʼ������ָ�룬��Ҫ��
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

	//��ʼ������ָ�룬��Ҫ����
	(*begin->nextBuilding).nextBuilding = NULL;
	(*begin->nextBuilding).rooms = NULL;
	return TRUE;
}


///////////////////////////////////////////////////////
//AddRoomToSearch
//���ܣ�����һ��ֻ��bui��ɵ�search
//��������Ҫ��ӵ�Room�ṹ�壬��Ӷ��������ͷָ��
//���أ��ɹ��򷵻�TRUE��ʧ�ܷ���FALSE

BOOL AddRoomToSearch(Room newCom, Room **head)
{
	Room *begin, *end;

	//û��¥�̵����
	if (*head == NULL)
	{
		*head = (Room*)malloc(sizeof(Room));
		**head = newCom;

		//��ʼ������ָ�룬��Ҫ��
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

	//��ʼ������ָ�룬��Ҫ����
	(*begin->nextRoom).nextRoom = NULL;
	return TRUE;
}


///////////////////////////////////////////////////////
//FileInitWnd
//���ܣ���ʼ�������ļ����ڵ�����
//�����������ھ��hwnd
//���أ��޷���ֵ

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
	ofn.lpstrFile = NULL;				 //�ڴ򿪡��ر��ļ�������
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL;           //�ڴ򿪡��ر��ļ�������
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = NULL;
	ofn.Flags = 0;						 //�ڴ򿪡��ر��ļ�������
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = TEXT("bin");
	ofn.lCustData = 0L;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;
}


///////////////////////////////////////////////////////
//FileImportDlg
//���ܣ����������ļ��Ի���
//�����������ھ��hwnd���ļ�����·����
//���أ����ɹ��򷵻�TRUE��ʧ���򷵻�FALSE

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
//���ܣ����������ļ��Ի���
//���������ھ��hwnd���ļ�����·����
//���أ����ɹ��򷵻�TRUE��ʧ���򷵻�FALSE

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
//���ܣ���ȡ�ļ�
//���������ھ��hwnd��·����
//���أ����ɹ��򷵻�TRUE��ʧ���򷵻�FALSE

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
		TEXT("%s - ¥��.bin"),
		TEXT("%s - ¥��.bin"),
		TEXT("%s - ����.bin")
	};
	TCHAR *szPrints[3], *szCompanyName;

	//��ȡ��˾��
	szCompanyName = (TCHAR*)malloc(sizeof(TCHAR) * 50);
	lstrcpy(szCompanyName, pstrLeadFileName);
	szCompanyName[lstrlen(szCompanyName) - 4] = '\0\0';

	//�༭�����ļ���
	for (i = 0; i < 3; i++)
	{
		szPrints[i] = (TCHAR*)malloc(sizeof(TCHAR) * 50);
		wsprintf(szPrints[i], szBuffers[i], szCompanyName);
	}

	//�������ļ�
	hLeadFile = CreateFile(pstrLeadFileName, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == hLeadFile)
		return FALSE;

	//�������ļ�
	for (i = 0; i < 3; i++)
		hFile[i] = CreateFile(szPrints[i], GENERIC_READ, FILE_SHARE_READ,
			NULL, OPEN_EXISTING, 0, NULL);

	//��ȡ�����ļ��Ĵ�С
	ReadFile(hLeadFile, &ldData, sizeof(LenData), &dwReadSize, NULL);

	//����ȡ�Ƿ�����
	if ((int)dwReadSize != sizeof(LenData))
	{
		for (i = 0; i < 3; i++)
			CloseHandle(hFile[i]);
		CloseHandle(hLeadFile);
		return FALSE;
	}

	//��������
	iComNum = ldData.iCom / sizeof(Community);
	iBuiNum = ldData.iBui / sizeof(Building);
	iRoomNum = ldData.iRoom / sizeof(Room);

	//ȷ����ſ�ʼ
	iAutoComNum = iComNum + 1;

	//����ռ�
	pcTmp = (Community*)malloc(sizeof(Community));
	pbTmp = (Building*)malloc(sizeof(Building));
	prTmp = (Room*)malloc(sizeof(Room));

	pHead = NULL;
	//��ȡ�����ļ���д������
	while (iComNum > 0)
	{
		iComNum--;
		ReadFile(hFile[0], pcTmp, sizeof(Community), &dwReadSize, NULL);
		SetFilePointer(hFile[0], sizeof(Community), NULL, FILE_CURRENT);

		//�����Ƿ�ȫ��д��ɹ�
		if ((int)dwReadSize != sizeof(Community))
		{
			CloseHandle(hFile[0]);
			return FALSE;
		}
		//��������
		AddComToList(*pcTmp, &pHead);

		while (iBuiNum > 0)
		{
			iBuiNum--;
			ReadFile(hFile[1], pbTmp, sizeof(Building), &dwReadSize, NULL);
			SetFilePointer(hFile[1], sizeof(Building), NULL, FILE_CURRENT);

			//�����Ƿ�ȫ��д��ɹ�
			if ((int)dwReadSize != sizeof(Building))
			{
				CloseHandle(hFile[1]);
				return FALSE;
			}

			//�����Ƿ�����¥���ָ���
			if (pbTmp->buildingNum == -1)
			{
				iBuiNum++;
				break;
			}

			//��¥����������
			AddBuiToList(*pbTmp, &pHead, pcTmp->communityNum);

			while (iRoomNum > 0)
			{
				iRoomNum--;
				ReadFile(hFile[2], prTmp, sizeof(Room), &dwReadSize, NULL);
				SetFilePointer(hFile[2], sizeof(Room), NULL, FILE_CURRENT);

				//�����Ƿ�ȫ��д��ɹ�
				if ((int)dwReadSize != sizeof(Room))
				{
					CloseHandle(hFile[2]);
					return FALSE;
				}

				//�����Ƿ���������ָ���
				if (prTmp->roomNum == -1)
				{
					iRoomNum++;
					break;
				}

				//�������������
				AddRoomToList(*prTmp, &pHead,
					pcTmp->communityNum, pbTmp->buildingNum);
			}
		}
	}

	//���������Ƿ�ȫ����ȡ
	if (iRoomNum || iBuiNum || iComNum)
	{
		for (i = 0; i < 3; i++)
			CloseHandle(hFile[i]);
		CloseHandle(hLeadFile);
		return FALSE;
	}

	//���������ڱ�������
	wsprintf(szPrint, TEXT("¥�̹���ϵͳ - %s"), szCompanyName);
	SetWindowText(hwnd, (PTSTR)szPrint);

	//���վ����ָ��
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
//���ܣ�д���ļ�
//���������ھ��hwnd�������ļ��ļ���
//���أ����ɹ��򷵻�TRUE��ʧ���򷵻�FALSE

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
		TEXT("%s - ¥��.bin"),
		TEXT("%s - ¥��.bin"),
		TEXT("%s - ����.bin")
	};
	TCHAR *szPrints[3];

	//��ȡ��˾��
	szCompanyName = (TCHAR*)malloc(sizeof(TCHAR) * 50);
	lstrcpy(szCompanyName, pstrLeadFileName);
	szCompanyName[lstrlen(szCompanyName) - 4] = '\0\0';

	//�༭�����ļ���
	for (i = 0; i < 3; i++)
	{
		szPrints[i] = (TCHAR*)malloc(sizeof(TCHAR) * 50);
		wsprintf(szPrints[i], szBuffers[i], szCompanyName);
	}

	//���ļ�
	for (i = 0; i < 3; i++)
		hFile[i] = CreateFile(szPrints[i], GENERIC_WRITE, 0,
			NULL, CREATE_ALWAYS, 0, NULL);
	hLeadFile = CreateFile(pstrLeadFileName, GENERIC_WRITE, 0,
		NULL, CREATE_ALWAYS, 0, NULL);

	if (INVALID_HANDLE_VALUE == hLeadFile)
		return FALSE;

	//�༭bui��room�ṹ���ļ��еķָ���

	pNullBui = (Building*)malloc(sizeof(Building));
	pNullRoom = (Room*)malloc(sizeof(Room));
	pNullBui->buildingNum = -1;
	pNullRoom->roomNum = -1;

	//��ȡ�������ݲ�д�������ļ���

	for (pTailCom = pHead; pTailCom;
		pTailCom = pTailCom->nextCommunity)
	{
		iComLen += sizeof(Community);
		WriteFile(hFile[0], pTailCom, sizeof(Community), &dwWriteSize, NULL);
		SetFilePointer(hFile[0], sizeof(Community), NULL, FILE_CURRENT);

		//�����Ƿ�ȫ��д��ɹ�
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
			
			//�����Ƿ�ȫ��д��ɹ�
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

				//�����Ƿ�ȫ��д��ɹ�
				if ((int)dwWriteSize != sizeof(Room))
				{
					CloseHandle(hFile[2]);
					return FALSE;
				}
			}

			//����Room�ָ���,�������Ƿ�ɹ�
			WriteFile(hFile[2], pNullRoom, sizeof(Room), &dwWriteSize, NULL);
			SetFilePointer(hFile[2], sizeof(Room), NULL, FILE_CURRENT);
			if ((int)dwWriteSize != sizeof(Room))
			{
				CloseHandle(hFile[2]);
				return FALSE;
			}
		}

		//����Building�ָ���,�������Ƿ�ɹ�
		WriteFile(hFile[1], pNullBui, sizeof(Building), &dwWriteSize, NULL);
		SetFilePointer(hFile[1], sizeof(Building), NULL, FILE_CURRENT);
		if ((int)dwWriteSize != sizeof(Building))
		{
			CloseHandle(hFile[1]);
			return FALSE;
		}
	}

	//�������ļ������ݴ�Сд�뵼���ļ���
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

	//�����ļ��Ƿ�Ϊ��
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
//���ܣ����ô�������
//�������༭����hwnd���ļ���
//���أ��޷���ֵ

void SetTitle(HWND hwnd, TCHAR * szTitleName)
{
	TCHAR szCaption[64 + MAX_PATH];

	wsprintf(szCaption, TEXT("%s - %s"), TEXT("¥�̹���ϵͳ"),
		szTitleName[0] ? szTitleName : TEXT("δ����"));

	SetWindowText(hwnd, szCaption);
}


///////////////////////////////////////////////////////
//DrawComInf
//���ܣ�����¥����Ϣ����
//�������༭����hwnd���豸����hdc��Community�ṹ
//���أ��ɹ�����TRUE��ʧ�ܷ���FALSE

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
		szBuffer, wsprintf(szBuffer, TEXT("¥�����ƣ�%s"), com.name));

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar + cyChar * 1.25, 
		szBuffer, wsprintf(szBuffer, TEXT("¥�̱�ţ�%d"), com.communityNum));

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar + 2 * cyChar * 1.25,
		szBuffer, wsprintf(szBuffer, TEXT("¥����ַ��%s"), com.address));

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar + 3 * cyChar * 1.25,
		szBuffer, wsprintf(szBuffer, TEXT("¥��������%d"), com.numberOfBuildings));

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar + 4 * cyChar * 1.25,
		szBuffer, wsprintf(szBuffer, TEXT("����������%d"), com.numberOfRooms));

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar + 5 * cyChar * 1.25,
		szBuffer, swprintf(szBuffer, TEXT("ƽ���۸�%.2f"), com.avgPrice));

	TextOut(hdc, 2.7 * XLINEPOS, 0.5*cyChar,
		szBuffer, wsprintf(szBuffer, TEXT("��ϵ��  ��%s"), com.host.name));

	TextOut(hdc, 2.7 * XLINEPOS, 0.5*cyChar + cyChar * 1.25,
		szBuffer, wsprintf(szBuffer, TEXT("��ϵ�绰��%s"), com.phone));

	DrawButton(hdc, cxClient, YLINEPOS);

	return TRUE;
}


///////////////////////////////////////////////////////
//DrawBuiInf
//���ܣ�����¥����Ϣ����
//�������༭����hwnd���豸����hdc��Building�ṹ
//���أ��ɹ�����TRUE��ʧ�ܷ���FALSE

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
		szBuffer, wsprintf(szBuffer, TEXT("¥����ţ�%d"), bui.buildingNum));

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar + cyChar * 1.25,
		szBuffer, wsprintf(szBuffer, TEXT("����¥�̣�%s"), bui.inCom));

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar + 2 * cyChar * 1.25,
		szBuffer, wsprintf(szBuffer, TEXT("¥����   ��%d"), bui.numberOfFloors));

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar + 3 * cyChar * 1.25,
		szBuffer, wsprintf(szBuffer, TEXT("������   ��%d"), bui.numberOfRooms));

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar + 4 * cyChar * 1.25,
		szBuffer, swprintf(szBuffer, TEXT("ƽ���۸�%.2f"), bui.avgPrice));

	TextOut(hdc, 2.7 * XLINEPOS, 0.5*cyChar,
		szBuffer, wsprintf(szBuffer, TEXT("��ϵ��   ��%s"), bui.host.name));

	TextOut(hdc, 2.7 * XLINEPOS, 0.5*cyChar + cyChar * 1.25,
		szBuffer, wsprintf(szBuffer, TEXT("��ϵ�绰��%s"), bui.host.phone));

	DrawButton(hdc, cxClient, YLINEPOS);

	return TRUE;
}


///////////////////////////////////////////////////////
//DrawRoomInf
//���ܣ�����¥����Ϣ����
//�������༭����hwnd���豸����hdc��Room�ṹ
//���أ��ɹ�����TRUE��ʧ�ܷ���FALSE

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
		szBuffer, wsprintf(szBuffer, TEXT("�����   ��%d"), room.roomNum));

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar + cyChar * 1.25,
		szBuffer, wsprintf(szBuffer, TEXT("����¥�̣�%s"), room.inCom));

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar + 2 * cyChar * 1.25,
		szBuffer, wsprintf(szBuffer, TEXT("����¥����%d"), room.buildingNum));

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar + 3 * cyChar * 1.25,
		szBuffer, wsprintf(szBuffer, TEXT("����¥�㣺%d"), room.floor));

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar + 4 * cyChar * 1.25,
		szBuffer, swprintf(szBuffer, TEXT("����۸�%.2f Ԫ / ƽ����"), room.roomPrice));

	TextOut(hdc, cxChar + XLINEPOS, 0.5*cyChar + 5 * cyChar * 1.25,
		szBuffer, swprintf(szBuffer, TEXT("�����ܼۣ�%d Ԫ"), room.allPrice));

	TextOut(hdc, 2.5 * XLINEPOS, 0.5*cyChar,
		szBuffer, wsprintf(szBuffer, TEXT("��ϵ��   ��%s"), room.host.name));

	TextOut(hdc, 2.5 * XLINEPOS, 0.5*cyChar + cyChar * 1.25,
		szBuffer, wsprintf(szBuffer, TEXT("��ϵ�绰��%s"), room.host.phone));

	TextOut(hdc, 2.5 * XLINEPOS, 0.5*cyChar + 2 * cyChar * 1.25,
		szBuffer, wsprintf(szBuffer, TEXT("%s"), room.roomType));

	if (room.roomState)
		lstrcpy(szBuffer, TEXT("���۳�"));
	else lstrcpy(szBuffer, TEXT("δ�۳�"));
	TextOut(hdc, 2.5 * XLINEPOS, 0.5*cyChar + 3 * cyChar * 1.25,
		szPrint, wsprintf(szPrint, TEXT("�۳������%s"), szBuffer));

	DrawButton(hdc, cxClient, YLINEPOS);
	roomNow = room.roomNum;

	return TRUE;
}


///////////////////////////////////////////////////////
//RenewData
//���ܣ����������ָ�Ϊ����ɫ
//�������豸����hdc��Building�ṹ
//���أ��޷���

void RenewData(HDC hdc, RECT rectData)
{
	SelectObject(hdc, GetStockObject(DC_BRUSH));
	SetDCBrushColor(hdc, RGB(137, 189, 255));   //����ɫΪ����ɫ

	Rectangle(hdc, rectData.left, rectData.top,
		rectData.right, rectData.bottom);

	SelectObject(hdc, GetStockObject(BLACK_BRUSH));
}


///////////////////////////////////////////////////////
//UpdataData
//���ܣ�����������������ݣ����������������۵�
//��������Ҫ���µ������ͷָ��ĵ�ַ
//���أ���������и����򷵻�TRUE�����򷵻�FALSE

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
		//��ʼ��¥�̼۸�ͷ�����
		sumComPrice = 0.0;
		comRoomNum = 0;
		comBuiNum = 0;

		for (pBui = pCom->buildings; pBui; pBui = pBui->nextBuilding)
		{
			//��ʼ��¥���۸�ͷ�������¥����
			sumBuiPrice = 0.0;
			buiRoomNum = 0;

			lstrcpy(pBui->inCom, pCom->name);

			for (pRoom = pBui->rooms; pRoom; pRoom = pRoom->nextRoom)
			{
				pRoom->buildingNum = pBui->buildingNum;

				//���㷿���ܼ�
				pRoom->allPrice = pRoom->roomSize * pRoom->roomPrice;
				pRoom->floor = pRoom->roomNum / 100;	//����¥����
				pRoom->host = pBui->host;	//������ϵ����¥��һ��
				
				//����¥���ܼ۸���ܷ�����
				sumBuiPrice += pRoom->roomPrice;
				buiRoomNum++;
			}

			//������д������
			pBui->numberOfRooms = buiRoomNum;
			if(buiRoomNum)
				pBui->avgPrice = sumBuiPrice / buiRoomNum;
			else pBui->avgPrice = 0.0;

			//����¥���ܼ۸��¥����
			sumComPrice += sumBuiPrice;
			comRoomNum += buiRoomNum;
			comBuiNum++;
		}
		
		//������д������
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
//���ܣ�WM_PAINT��Ϣ�л��Ƶ�һ��(����Com��Bui)������ʾ����
//�������豸����hdc����ʾ���ݵ�comָ�루NULL��ʾ��¥�̣�
//���أ��޷���

void Draw1Step(HDC hdc, Community *pChosenCom)
{
	HINSTANCE hInstance;
	Community *pCom;
	Building *pBui;
	TCHAR szPrint[100];
	TCHAR *pszSubTitle[] = {
		TEXT("¥����"),
		TEXT("����¥��"),
		TEXT("ƽ���۸�"),
		TEXT("��ϵ�绰")
	};
	int piColWid[] = { 80,250,170,115 };
	int iCol;
	LVCOLUMN lvc;
	LVITEM lvi;

	hInstance = (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);

	lvc.mask = LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
	
	//��������ͱ���
	SendMessage(hwndSubList, WM_SETFONT, (WPARAM)hFontSon, TRUE);
	SetWindowText(hwndMainTitle, TEXT("¥ ��"));

	//���listview��4��
	for (iCol = 0; iCol < 4; iCol++)
	{
		lvc.iSubItem = iCol;
		lvc.pszText = pszSubTitle[iCol];
		lvc.cx = piColWid[iCol];
		lvc.fmt = LVCFMT_CENTER;

		ListView_DeleteColumn(hwndSubList, iCol);
		ListView_InsertColumn(hwndSubList, iCol, &lvc);
	}

	//�������˵�����
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

	//�ձ�ʾĿǰ��¥�̣�ֱ���˳�
	if (!pChosenCom)
		return;

	//�����
	lvi.mask = LVIF_TEXT | LVIF_STATE;
	lvi.iSubItem = 0;	//��ʼ������
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
				//���һ�б��
				ListView_InsertItem(hwndSubList, &lvi);

				//��һ������ı�
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
//���ܣ�WM_PAINT��Ϣ�л��Ƶڶ���������ʾ����
//�������豸����hdc,ָ����ѡ¥����ָ��
//���أ��޷���

void Draw2Step(HDC hdc,Building *pBui)  
{
	Room *pRoom;
	Community *pCom;
	HINSTANCE hInstance;
	TCHAR szPrint[100];
	TCHAR *pszSubTitle[] = {
		TEXT("�����"),
		TEXT("���ڲ���"),
		TEXT("����۸�"),
		TEXT("�������")
	};
	int piColWid[] = { 80,250,170,115 };
	int iCol;
	LVCOLUMN lvc;
	LVITEM lvi;

	hInstance = (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);

	lvc.mask = LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;

	//��������ͱ���
	SendMessage(hwndSubList, WM_SETFONT, (WPARAM)hFontSon, TRUE);
	SetWindowText(hwndMainTitle, TEXT("¥ ��"));

	SendMessage(hwndMainList, LB_RESETCONTENT, 0, 0);
	for (pCom = pHead; pCom; pCom = pCom->nextCommunity)
		SendMessage(hwndMainList, LB_ADDSTRING, 0, (LPARAM)pCom->name);

	//���listview��4��
	for (iCol = 0; iCol < 4; iCol++)
	{
		lvc.iSubItem = iCol;
		lvc.pszText = pszSubTitle[iCol];
		lvc.cx = piColWid[iCol];
		lvc.fmt = LVCFMT_CENTER;

		ListView_DeleteColumn(hwndSubList, iCol);
		ListView_InsertColumn(hwndSubList, iCol, &lvc);
	}

	//�������˵�����
	ListView_DeleteAllItems(hwndSubList);

	if (!pBui)
		return;

	//�����
	lvi.mask = LVIF_TEXT | LVIF_STATE;
	lvi.iSubItem = 0;	//��ʼ������
	lvi.pszText = LPSTR_TEXTCALLBACK;
	lvi.state = 0;
	lvi.stateMask = 0;
	lvi.iItem = 0;

	//�����Ӳ˵�

		//��һ��Ϊ��¥�����Թ��鿴¥����Ϣ�ͷ�����һ��
	wsprintf(szPrint, TEXT("%s��%d��"), pBui->inCom, pBui->buildingNum);
	
		//���һ�б��
	ListView_InsertItem(hwndSubList, &lvi);
	ListView_SetItemText(hwndSubList, lvi.iItem, 1, szPrint);
	lvi.iItem++;

		//֮��Ϊ���������Ϣ
	for (pRoom = pBui->rooms; pRoom; pRoom = pRoom->nextRoom)
	{
		//���һ�б��
		ListView_InsertItem(hwndSubList, &lvi);

		//��һ������ı�
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
//���ܣ�ɾ�������е�һ��Community�ڵ�
//��������Ҫɾ����Community��ţ���Ӷ��������ͷָ��ĵ�ַ
//���أ����ظ��º�����ͷָ��

BOOL DelComInList(int comNum, Community **head)
{
	Community *comTail, *comLastTail;

	//û��¥�̵����
	if (*head == NULL)
		return FALSE;

	//ֻ��һ���ڵ�����
	if ((*head)->nextCommunity == NULL)
	{
		//����һ���ڵ��ǲ�������ֵ
		if ((*head)->communityNum == comNum)
		{
			//ֻ��һ���ڵ㣬ֱ��ָ��NULL
			*head = NULL;
			return TRUE;
		}
		else return FALSE;
	}

	//����һ���ڵ��ǲ�������ֵ
	if ((*head)->communityNum == comNum)
	{
		//�ж���ڵ㣬������һ��
		*head = (*head)->nextCommunity;
		return TRUE;
	}

	comTail = (*head)->nextCommunity;
	comLastTail = *head;

	//ѭ��ɾ����¥��
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
//���ܣ�ɾ�������е�һ��Building�ڵ�
//��������Ҫɾ����Building��ţ���Ӷ��������ͷָ��ĵ�ַ
//���أ����ظ��º�����ͷָ��

BOOL DelBuiInList(int comNum, int buiNum, Community **head)

{
	Community *comTail;
	Building *buiTail, *buiLastTail;

	//û��¥�̵����
	if (*head == NULL)
		return FALSE;

	for (comTail = *head; comTail != NULL;
		comTail = comTail->nextCommunity)
	{
		if (comNum == comTail->communityNum)
		{
			//û��¥�������
			if (comTail->buildings == NULL)
				return FALSE;

			//ֻ��һ���ڵ�����
			if (comTail->buildings->nextBuilding == NULL)
			{
				//����һ���ڵ��ǲ�������ֵ
				if (comTail->buildings->buildingNum == buiNum)
				{
					//ֻ��һ���ڵ㣬ֱ��ָ��NULL
					comTail->buildings = NULL;
					return TRUE;
				}
				else return FALSE;
			}

			//����һ���ڵ��ǲ�������ֵ
			if (comTail->buildings->buildingNum == buiNum)
			{
				//ָ����һ���ڵ�
				comTail->buildings = comTail->buildings->nextBuilding;
				return TRUE;
			}

			//��ʼ������ָ��
			buiLastTail = comTail->buildings;
			buiTail = comTail->buildings->nextBuilding;

			//ѭ��ɾ��
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
//���ܣ�ɾ�������е�һ��Room�ڵ�
//��������Ҫɾ����Room��ţ���Ӷ��������ͷָ��ĵ�ַ
//���أ����ظ��º�����ͷָ��

BOOL DelRoomInList(int comNum, int buiNum, int roomNum, Community **head)
{
	Community *comTail;
	Building *buiTail;
	Room *roomTail, *roomLastTail;

	//û��¥�̵����
	if (*head == NULL)
		return FALSE;

	for (comTail = *head; comTail != NULL;
		comTail = comTail->nextCommunity)
	{
		if (comNum == comTail->communityNum)
		{
			//û��¥�������
			if (comTail->buildings == NULL)
				return FALSE;

			for (buiTail = comTail->buildings; buiTail != NULL;
				buiTail = buiTail->nextBuilding)
			{
				if (buiTail->buildingNum == buiNum)
				{
					//û�з�������
					if (buiTail->rooms == NULL)
						return FALSE;

					//ֻ��һ���ڵ�����
					if (buiTail->rooms->nextRoom == NULL)
					{
						//����һ���ڵ��ǲ�������ֵ
						if (buiTail->rooms->roomNum == roomNum)
						{
							//ֻ��һ���ڵ㣬ֱ��ָ��NULL
							buiTail->rooms = NULL;
							return TRUE;
						}
						else return FALSE;
					}

					//����һ���ڵ��ǲ�������ֵ
					if (buiTail->rooms->roomNum == roomNum)
					{
						//ָ����һ��
						buiTail->rooms = buiTail->rooms->nextRoom;
						return TRUE;
					}

					//��ʼ������ָ��
					roomLastTail = buiTail->rooms;
					roomTail = buiTail->rooms->nextRoom;

					//ѭ��ɾ��
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
//��ɾ��¥�̡���DeleteCommunity���Ի��򴰿ڹ���

BOOL CALLBACK DelComDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	TCHAR szBuffer[100][100];
	Community *comTail;
	int iComDelNum = -1;
	static int iStateEdit;	//1��ʾ����������֣�2��ʾ�������¥�̺�
	int iLength[5];

	switch (message)
	{
	case WM_INITDIALOG:

		SendMessage(GetDlgItem(hDlg, IDC_RADIO3), 
			BM_SETCHECK, BST_CHECKED, 0);
		//Disable�ڶ����༭��
		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNUM), FALSE);
		iStateEdit = 1;
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_RADIO3:

			if (HIWORD(wParam) == BN_CLICKED)
			{
				//����enable�ı༭��
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNAME), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNUM), FALSE);
				SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNUM), TEXT(""));
				iStateEdit = 1;
			}
			break;

		case IDC_RADIO4:

			if (HIWORD(wParam) == BN_CLICKED)
			{
				//����enable�ı༭��
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNUM), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNAME), FALSE);
				SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNAME), TEXT(""));
				iStateEdit = 2;
			}
			break;

		case IDOK:

			//������OKʱ�������������ݴ���洢��	

			if (iStateEdit == 1)
				iLength[0] = SendMessage(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNAME),
					EM_GETLINE, 0, (LPARAM)szBuffer[0]);
			else if (iStateEdit == 2)
				iLength[0] = SendMessage(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNUM),
					EM_GETLINE, 1, (LPARAM)szBuffer[0]);

			//��������ǿ�

			if (!iLength[0])
			{
				MessageBox(hDlg, TEXT("��������������Ϣ��"), TEXT("��ʾ"), MB_OK);
				return FALSE;
			}
			else *(szBuffer[0] + iLength[0]) = '\0\0';
			
			//�洢��Ѱ����Ҫɾ����¥����

			if (iStateEdit == 1)
			{
				comTail = pHead;

				//ѭ������¥����
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

			//���¥�����Ƿ���ȷ
			if (iComDelNum == -1)
			{
				MessageBox(hDlg, TEXT("��¥�����Ʋ����ڣ�"), TEXT("��ʾ"), MB_OK);
				return FALSE;
			}

			//ɾ����������¥�̺�
			if (!DelComInList(iComDelNum, &pHead))
			{
				MessageBox(hDlg, TEXT("��¥�̺Ų����ڣ�"), TEXT("��ʾ"), MB_OK);
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
//��ɾ��¥�̡���DeleteBui���Ի��򴰿ڹ���

BOOL CALLBACK DelBuiDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	TCHAR szBuffer[100][100];
	Community *comTail;
	int i, iComDelNum = -1, iBuiDelNum = -1;
	static int iStateEdit;	//1��ʾ����������֣�2��ʾ�������¥�̺�
	int iLength[5];

	switch (message)
	{
	case WM_INITDIALOG:

		SendMessage(GetDlgItem(hDlg, IDC_RADIO3),
			BM_SETCHECK, BST_CHECKED, 0);
		//Disable�ڶ����༭��
		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNUM), FALSE);
		iStateEdit = 1;
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_RADIO3:

			if (HIWORD(wParam) == BN_CLICKED)
			{
				//����enable�ı༭��
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNAME), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNUM), FALSE);
				SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNUM), TEXT(""));
				iStateEdit = 1;
			}
			break;

		case IDC_RADIO4:

			if (HIWORD(wParam) == BN_CLICKED)
			{
				//����enable�ı༭��
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNUM), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNAME), FALSE);
				SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNAME), TEXT(""));
				iStateEdit = 2;
			}
			break;

		case IDOK:

			//������OKʱ�������������ݴ���洢��	

			if (iStateEdit == 1)
				iLength[0] = SendMessage(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNAME),
					EM_GETLINE, 0, (LPARAM)szBuffer[0]);
			else if (iStateEdit == 2)
				iLength[0] = SendMessage(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNUM),
					EM_GETLINE, 1, (LPARAM)szBuffer[0]);
			
			iLength[1] = SendMessage(GetDlgItem(hDlg, IDC_EDIT_DEL_BUINUM),
				EM_GETLINE, 3, (LPARAM)szBuffer[1]);

			//����ÿһ��ǿ�

			for (i = 0; i < 2; i++)
			{
				if (!iLength[i])
				{
					MessageBox(hDlg, TEXT("��������������Ϣ��"), TEXT("��ʾ"), MB_OK);
					return FALSE;
				}
				else {
					*(szBuffer[i] + iLength[i]) = '\0\0';
				}
			}

			//�洢��Ѱ����Ҫɾ����¥����

			if (iStateEdit == 1)
			{
				comTail = pHead;

				//ѭ������¥����
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

			//���¥�����Ƿ���ȷ
			if (iComDelNum == -1)
			{
				MessageBox(hDlg, TEXT("��¥�����Ʋ����ڣ�"), TEXT("��ʾ"), MB_OK);
				return FALSE;
			}

			//ɾ����������¥����
			if (!DelBuiInList(iComDelNum, iBuiDelNum, &pHead))
			{
				MessageBox(hDlg, TEXT("�Ҳ�����¥����"), TEXT("��ʾ"), MB_OK);
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
//��ɾ��¥�̡���DeleteRoom���Ի��򴰿ڹ���

BOOL CALLBACK DelRoomDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	TCHAR szBuffer[100][100];
	Community *comTail;
	int i;
	int iComDelNum = -1, iBuiDelNum = -1, iRoomDelNum = -1;
	static int iStateEdit;	//1��ʾ����������֣�2��ʾ�������¥�̺�
	int iLength[5];

	switch (message)
	{
	case WM_INITDIALOG:

		SendMessage(GetDlgItem(hDlg, IDC_RADIO3),
			BM_SETCHECK, BST_CHECKED, 0);
		//Disable�ڶ����༭��
		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNUM), FALSE);
		iStateEdit = 1;
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_RADIO3:

			if (HIWORD(wParam) == BN_CLICKED)
			{
				//����enable�ı༭��
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNAME), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNUM), FALSE);
				SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNUM), TEXT(""));
				iStateEdit = 1;
			}
			break;

		case IDC_RADIO4:

			if (HIWORD(wParam) == BN_CLICKED)
			{
				//����enable�ı༭��
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNUM), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNAME), FALSE);
				SetWindowText(GetDlgItem(hDlg, IDC_EDIT_DEL_COMNAME), TEXT(""));
				iStateEdit = 2;
			}
			break;

		case IDOK:

			//������OKʱ�������������ݴ���洢��	

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

			//����ÿһ��ǿ�

			for (i = 0; i < 3; i++)
			{
				if (!iLength[i])
				{
					MessageBox(hDlg, TEXT("��������������Ϣ��"), TEXT("��ʾ"), MB_OK);
					return FALSE;
				}
				else {
					*(szBuffer[i] + iLength[i]) = '\0\0';
				}
			}

			//������¥����ת��Ϊ¥�̺�

			if (iStateEdit == 1)
			{
				comTail = pHead;

				//ѭ������¥����
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

			//���¥�����Ƿ���ȷ
			if (iComDelNum == -1)
			{
				MessageBox(hDlg, TEXT("��¥�����Ʋ����ڣ�"), TEXT("��ʾ"), MB_OK);
				return FALSE;
			}

			//ɾ���������鷿���
			if (!DelRoomInList(iComDelNum, iBuiDelNum, iRoomDelNum, &pHead))
			{
				MessageBox(hDlg, TEXT("�Ҳ����÷��䣡"), TEXT("��ʾ"), MB_OK);
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
//���ư�ť
//������hdc����Ϣ�������½ǵ�����

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
//���༭¥����Ϣ����EditComDlgProc���Ի��򴰿ڹ���

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

		//���ñ༭���ʼ����
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

			//������OKʱ�������������ݴ���洢��	

			iLength[0] = SendMessage(GetDlgItem(hDlg, IDC_ADD_COM_NAME),
				EM_GETLINE, 0, (LPARAM)szBuffer[0]);
			iLength[1] = SendMessage(GetDlgItem(hDlg, IDC_ADD_COM_ADDR),
				EM_GETLINE, 1, (LPARAM)szBuffer[1]);
			iLength[2] = SendMessage(GetDlgItem(hDlg, IDC_ADD_COM_PHONE),
				EM_GETLINE, 2, (LPARAM)szBuffer[2]);
			iLength[3] = SendMessage(GetDlgItem(hDlg, IDC_ADD_COM_PERSON),
				EM_GETLINE, 3, (LPARAM)szBuffer[3]);

			//����ÿһ��ǿ�

			for (i = 0; i < 4; i++)
			{
				if (!iLength[i])
				{
					MessageBox(hDlg, TEXT("��������������Ϣ��"), TEXT("��ʾ"), MB_OK);
					return FALSE;
				}
				else {
					*(szBuffer[i] + iLength[i]) = '\0\0';
				}
			}

			//�޸�����������

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
						MessageBox(hwnd, TEXT("��¥�̺�����ϵͳ��"),
							TEXT("����"), MB_OK | MB_ICONWARNING);
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
//���༭¥����Ϣ����EditBuiDlgProc���Ի��򴰿ڹ���

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

		//���ñ༭���ʼ����
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

			//������OKʱ�������������ݴ���洢��

			iLength[0] = SendMessage(GetDlgItem(hDlg, IDC_ADD_BUI_NUM),
				EM_GETLINE, 0, (LPARAM)szBuffer[0]);
			iLength[1] = SendMessage(GetDlgItem(hDlg, IDC_ADD_BUI_FLOORS),
				EM_GETLINE, 1, (LPARAM)szBuffer[1]);
			iLength[2] = SendMessage(GetDlgItem(hDlg, IDC_ADD_BUI_PHONE),
				EM_GETLINE, 2, (LPARAM)szBuffer[2]);
			iLength[3] = SendMessage(GetDlgItem(hDlg, IDC_ADD_BUI_PERSON),
				EM_GETLINE, 3, (LPARAM)szBuffer[3]);

			//����ÿһ��ǿ�

			for (i = 0; i < 4; i++)
			{
				if (!iLength[i])
				{
					MessageBox(hDlg, TEXT("��������������Ϣ��"), TEXT("��ʾ"), MB_OK);
					return FALSE;
				}
				else {
					*(szBuffer[i] + iLength[i]) = '\0\0';
				}
			}

			//�޸�����������

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
								MessageBox(hwnd, TEXT("��¥��������ϵͳ��"),
									TEXT("����"), MB_OK | MB_ICONWARNING);
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
//���༭������Ϣ����EditRoomDlgProc���Ի��򴰿ڹ���

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

		//��ComboBox�����������
		for (i = 0; i < 34; i++)
			SendMessage(GetDlgItem(hDlg, IDC_ADD_ROOM_STYLE),
				CB_ADDSTRING, 0, (LPARAM)szComboBoxData[i]);

		//���ñ༭���ʼ����
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

								//����Ĭ�ϻ���
								for (i = 0; i < 34; i++)
									if (!lstrcmp(szComboBoxData[i], roomTail->roomType))
										break;
								SendMessage(GetDlgItem(hDlg, IDC_ADD_ROOM_STYLE),
									CB_SETCURSEL, i, 0);

								//����Ĭ���۳�״̬
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

			//������OKʱ�������������ݴ���洢��	

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

			//����ÿһ��ǿ�

			for (i = 0; i < 3; i++)
			{
				if (!iLength[i])
				{
					MessageBox(hDlg, TEXT("��������������Ϣ��"), TEXT("��ʾ"), MB_OK);
					return FALSE;
				}
				else {
					*(szBuffer[i] + iLength[i]) = '\0\0';
				}
			}

			//�޸�����������

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
										MessageBox(hwnd, TEXT("�÷���������ϵͳ��"),
											TEXT("����"), MB_OK | MB_ICONWARNING);
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
//�����ÿһ��¥�̵����ƶ�
//��������׼SearchData�ṹ���Աȵ�¥��
//���أ����������Ϣ���¥�̵����ƶ�

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
//�����ÿһ��¥�������ƶ�
//��������׼SearchData�ṹ���Աȵ�¥������¥������¥��
//���أ����������Ϣ���¥�������ƶ�

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
//�����ÿһ����������ƶ�
//��������׼SearchData�ṹ���Աȵķ��䣬�÷�������¥����¥��
//���أ����������Ϣ��÷�������ƶ�

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
//���������ܡ���Search���Ի��򴰿ڹ���

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
	static int iSearchPos = 0;	//1��ʾcom��2��ʾbui��3��ʾroom��0��ʾδѡ��
	int iLength[20];

	switch (message)
	{
	case WM_INITDIALOG:

		//��ComboBox�����������
		for (i = 0; i < 34; i++)
			SendMessage(GetDlgItem(hDlg, IDC_S_ROOMTYPE),
				CB_ADDSTRING, 0, (LPARAM)szComboBoxData[i]);

		//����Ĭ��ѡ��¥��
		SendMessage(GetDlgItem(hDlg, IDC_SCOM),
			BM_SETCHECK, BST_CHECKED, 0);
		//Disable¥���ͷ���
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
				//Disable¥���ͷ���
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
				//Disable����

				EnableWindow(GetDlgItem(hDlg, IDC_S_ROOMNUM), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_ROOMSIZE), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_ROOMFLOOR), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_ALLPRICE), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_LOPRICE), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_HIPRICE), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_ROOMTYPE), FALSE);

				//Enable¥��

				EnableWindow(GetDlgItem(hDlg, IDC_S_BUINUM), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_BUIPERSON), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_S_BUIPHONE), TRUE);

				iSearchPos = 2;
			}
			break;

		case IDC_SROOM:

			if (HIWORD(wParam) == BN_CLICKED)
			{
				//Enable�����¥��

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
			case 0:		//����������Ϊ�գ����������ܳ��֣�
				MessageBox(hDlg, TEXT("��ѡ��Ҫ�����Ķ���"), TEXT("��ʾ"), MB_OK);
				break;

			case 1:		//����������Ϊ¥��

				sData.mask = SD_COM;
				
				iLength[0] = SendMessage(GetDlgItem(hDlg, IDC_S_COMNAME),
					EM_GETLINE, 0, (LPARAM)szBuffer[0]);
				iLength[1] = SendMessage(GetDlgItem(hDlg, IDC_S_COMNUM),
					EM_GETLINE, 0, (LPARAM)szBuffer[1]);
				iLength[2] = SendMessage(GetDlgItem(hDlg, IDC_S_COMPERSON),
					EM_GETLINE, 0, (LPARAM)szBuffer[2]);
				iLength[3] = SendMessage(GetDlgItem(hDlg, IDC_S_COMPHONE),
					EM_GETLINE, 0, (LPARAM)szBuffer[3]);

				//��ÿһ���ַ�����ӽ�β
				for (i = 0; i < 4; i++)
					*(szBuffer[i] + iLength[i]) = '\0\0';

				//¼����������Ϣ
				lstrcpy(sData.comName, szBuffer[0]);
				sData.comNum = _ttoi(szBuffer[1]);
				lstrcpy(sData.comPerson, szBuffer[2]);
				lstrcpy(sData.comPhone, szBuffer[3]);
				
				//�������Ƶ���������������
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
					MessageBox(hwnd, TEXT("δ�ҵ��������"), TEXT("��ʾ"), MB_OK);
					return FALSE;
				}

				//����������������������
				pComSearch = RankCom(sData, pComSearch);

				iSearch = SEARCHCOM;
				break;

			case 2:		//����������Ϊ¥��

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

				//��ÿһ���ַ�����ӽ�β
				for (i = 0; i < 7; i++)
					*(szBuffer[i] + iLength[i]) = '\0\0';

				//¼����������Ϣ
				lstrcpy(sData.comName, szBuffer[0]);
				sData.comNum = _ttoi(szBuffer[1]);
				lstrcpy(sData.comPerson, szBuffer[2]);
				lstrcpy(sData.comPhone, szBuffer[3]);
				sData.buiNum = _ttoi(szBuffer[4]);
				lstrcpy(sData.buiPerson, szBuffer[5]);
				lstrcpy(sData.buiPhone, szBuffer[6]);

				//�������Ƶ���������������
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
					MessageBox(hwnd, TEXT("δ�ҵ��������"), TEXT("��ʾ"), MB_OK);
					return FALSE;
				}

				//����������������������
				pBuiSearch = RankBui(sData, pBuiSearch);

				iSearch = SEARCHBUI;
				break;

			case 3:		//����������Ϊ����

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

				//��ÿһ���ַ�����ӽ�β
				for (i = 0; i < 13; i++)
					*(szBuffer[i] + iLength[i]) = '\0\0';

				//¼����������Ϣ
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

				//�������Ƶ���������������
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
					MessageBox(hwnd, TEXT("δ�ҵ��������"), TEXT("��ʾ"), MB_OK);
					return FALSE;
				}

				//����������������������
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
//��Com������Ȩ�ؽ�������
//��������׼�Ƚ��������ݣ����������ͷָ��
//���أ�����Ϊ������������ͷָ��

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
				//���� p->next �� p->next->next ��λ��
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
//��Bui������Ȩ�ؽ�������
//��������׼�Ƚ��������ݣ����������ͷָ��
//���أ�����Ϊ������������ͷָ��

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
			//�ҵ��������ڵ�¥��
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
				//���� p->next �� p->next->next ��λ��
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
//��Room������Ȩ�ؽ�������
//��������׼�Ƚ��������ݣ����������ͷָ��
//���أ�����Ϊ������������ͷָ��

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
			//�ҵ��������ڵ�¥��
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
				//���� p->next �� p->next->next ��λ��
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
//����Ƿ����ظ��ķ���
//����������¥����������¥���ţ�����ţ����������ͷָ��
//���أ���û�з���TRUE���з���FALSE

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
//����Ƿ����ظ���¥��
//����������¥������¥���ţ����������ͷָ��
//���أ���û�з���TRUE���з���FALSE

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
//����Ƿ����ظ���¥��
//������¥�̺ţ����������ͷָ��
//���أ���û�з���TRUE���з���FALSE

BOOL CheckSameCom(TCHAR comName[], Community **head)
{
	Community *comTail;

	for (comTail = *head; comTail; comTail = comTail->nextCommunity)
		if (!lstrcmp(comTail->name, comName))
			return FALSE;

	return TRUE;
}