#include <Windows.h>
#include <commdlg.h>
#include <tchar.h>
#include <stdlib.h>
#include <CommCtrl.h>

#define ID_MAINLIST			10
#define ID_MAINLISTTITLE	11
#define ID_SUBLIST			12
#define ID_SUBLISTTITLE		13
#define ID_FILEEDIT			14
#define TP_BUI				30000
#define TP_COM				30001
#define PA_FIRSTEPCOM		500
#define PA_SECSTEPBUI		501
#define PA_FIRSTEPBUI		502
#define PA_SECSTEPROOM		503
#define ID_BUTTONDEL		15
#define ID_BUTTONEDIT		16
#define SD_COM				600
#define SD_BUI				601
#define SD_ROOM				602
#define ID_BUTTONENTER		18

#define SEARCHCOM			701
#define NOSEARCH			702
#define SEARCHBUI			703
#define SEARCHROOM			704

#define SEARCHING	(iSearch==SEARCHCOM||iSearch==SEARCHBUI||iSearch==SEARCHROOM)

#define XLINEPOS (1.0 / 4 * cxClient)			//主界面竖直分界线
#define YLINEPOS ((1 - 0.618) * cyClient)		//主界面水平分界线

#define Swap(x,y) (x)=(y)-(x);(x)=(x)+(y);(y)=(x)-(y);	//交换x、y的值

///////////////////////////////////////////////////////
//主窗口窗口过程

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


///////////////////////////////////////////////////////
//AskConfirm 
//功能：当用户退出系统时，若文件未保存，则提示其是否需要保存
//     若文件已经保存，则直接退出
//参数：退出的窗口句柄，保存状态（非0为已保存，0为未保存）
//返回：若需要退出系统，则返回TRUE；若不需要，则返回FALSE

BOOL AskConfirm(HWND hwnd, int iSaveState);


///////////////////////////////////////////////////////
//DrawBasicBk
//窗口基本布局构造函数
//功能：画出窗口的基本布局，不含任何信息
//参数：窗口句柄,设备环境,屏幕宽，屏幕高
//返回：数据窗口的RECT矩形

RECT DrawBasicBk(HWND hwnd, HDC hdc, int cxClient, int cyClient);


///////////////////////////////////////////////////////
//“关于‘楼盘管理系统’”（About）对话框窗口过程

BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);


///////////////////////////////////////////////////////
//“添加新楼盘”（AddCommunity）对话框窗口过程

BOOL CALLBACK AddComDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);


///////////////////////////////////////////////////////
//“添加新楼栋”（AddBuilding）对话框窗口过程

BOOL CALLBACK AddBuiDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);


///////////////////////////////////////////////////////
//“添加新房间”（AddRoom）对话框窗口过程

BOOL CALLBACK AddRoomDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);


///////////////////////////////////////////////////////
//联系人结构体
//功能：用于存储个人信息
typedef struct _Person {
	TCHAR name[100];		//姓名
	TCHAR phone[50];		//电话
	TCHAR idNumber[50];		//证件ID号
}Person;


///////////////////////////////////////////////////////
//房间结构体
//功能：储存房间相关信息，并可用于构造链表
typedef struct _Room {
	int roomNum;				//房间号
	int buildingNum;			//所在楼栋号
	int communityNum;			//所在楼盘号
	int floor;					//所在楼层
	float roomSize;				//房间面积（平方米）
	float roomPrice;			//房间价格（人民币元/平方米）
	long allPrice;				//房间总价（人民币元）
	TCHAR roomType[100];		//户型：室，厅，厕，厨数量
	TCHAR inCom[100];			//所在楼盘名
	BOOL roomState;				//售出状态：0代表未售出，1代表已售出
	TCHAR note[1000];			//备注
	Person host;				//联系人结构
	struct _Room *nextRoom;		//指向链表中下一个房间
}Room;


///////////////////////////////////////////////////////
//楼栋结构体
//功能：储存楼栋相关信息，并可用于构造链表
typedef struct _Building {
	int buildingNum;					//楼栋号
	int communityNum;					//所在楼盘号
	int numberOfRooms;					//所有房间数
	int numberOfFloors;					//楼层数
	float avgPrice;						//平均价格
	TCHAR inCom[100];					//所在楼盘名称
	Person host;						//联系人结构
	struct _Building *nextBuilding;		//指向下一个楼栋
	struct _Room *rooms;				//指向该楼栋第一个房间
}Building;


///////////////////////////////////////////////////////
//楼盘结构体
//功能：储存楼盘相关信息，并可用于构造链表
typedef struct _Community {
	int communityNum;								//楼盘号
	TCHAR name[100];								//楼盘名
	TCHAR address[100];								//楼盘地址
	TCHAR phone[30];								//联系电话
	int numberOfBuildings;							//所有楼栋数
	int numberOfRooms;								//所有房间数
	float avgPrice;									//平均价格
	Person host;									//联系人结构
	struct _Community *nextCommunity = NULL;		//指向下一个楼盘
	struct _Building *buildings = NULL;				//指向该楼盘第一个楼栋
}Community;

/////////////////////////////////////////////////////////
//*.led文件中的结构体
//储存com，bui，room三个文件里面所包含的数据数
typedef struct _LenData {
	int iCom;
	int iBui;
	int iRoom;
}LenData;


/////////////////////////////////////////////////////////
//搜索中使用的结构体，容纳搜索的所有数据
typedef struct _SearchData {

	int mask;			//确定搜索的对象
						//值为SD_COM,SD_BUI,SD_ROOM中的一个
	//Community
	TCHAR comName[100];	
	int comNum;
	TCHAR comPerson[100];
	TCHAR comPhone[100];

	//Building
	int buiNum;
	TCHAR buiPhone[100];
	TCHAR buiPerson[100];

	//Room
	int roomNum;
	int roomFloor;
	float roomSize;
	long allPrice;
	float roomLoPrice;
	float roomHiPrice;
	BOOL roomSold;
	TCHAR roomType[100];
}SearchData;


///////////////////////////////////////////////////////
//createComList
//功能：创建一个community的先进先出的单向链表
//参数：链表长度
//返回：该链表头指针

Community *createComList(int len);


///////////////////////////////////////////////////////
//createBuiList
//功能：创建一个building的先进先出的单向链表
//参数：链表长度
//返回：该链表头指针

Building *createBuiList(int len);


///////////////////////////////////////////////////////
//createRoomList
//功能：创建一个room的先进先出的单向链表
//参数：链表长度
//返回：该链表头指针

Room *createRoomList(int len);

///////////////////////////////////////////////////////
//create3DepthList
//功能：创建一个三级链表
//参数：1、第一级个数；
//     2、每个第一级所含第二级个数
//     3、每个第二级所含第三级个数
//返回：返回该三级链表的头指针

Community *create3DepthList(int len1, int len2, int len3);


///////////////////////////////////////////////////////
//AddComToList
//功能：向链表中添加一个community节点
//参数：将要添加的Community结构体（添加对象链表的头指针.全局变量提供）
//返回：返回更新后链表头指针

BOOL AddComToList(Community newCom, Community **head);


///////////////////////////////////////////////////////
//AddBuiToList
//功能：向链表中添加一个building节点
//参数：将要添加的Building结构体，添加对象链表的头指针，所属楼盘号
//返回：成功则返回TRUE，失败返回FALSE

BOOL AddBuiToList(Building newBui, Community **head, int comNum);


///////////////////////////////////////////////////////
//AddRoomToList
//功能：向链表中添加一个Room节点
//参数：将要添加的Room结构体，添加对象链表的头指针，所属楼盘号，所属楼栋号
//返回：成功则返回TRUE，失败返回FALSE

BOOL AddRoomToList(Room newRoom, Community **head,
	int comNum, int buiNum);


///////////////////////////////////////////////////////
//FileInitWnd
//功能：初始化弹出文件窗口的数据
//参数：父窗口句柄hwnd
//返回：无返回值

void FileInitWnd(HWND hwnd);


///////////////////////////////////////////////////////
//FileImportDlg
//功能：创建导入文件对话框
//参数：父窗口句柄hwnd，文件名，路径名
//返回：若成功则返回TRUE，失败则返回FALSE

BOOL FileImportDlg(HWND hwnd, PTSTR pstrFileName, PTSTR pstrTitleName);


///////////////////////////////////////////////////////
//FileSaveDlg
//功能：创建保存文件对话框
//参数：窗口句柄hwnd，文件名，路径名
//返回：若成功则返回TRUE，失败则返回FALSE

BOOL FileSaveDlg(HWND hwnd, PTSTR pstrFileName, PTSTR pstrTitleName);


///////////////////////////////////////////////////////
//FileReadWnd
//功能：创建保存文件对话框
//参数：窗口句柄hwnd，文件名，路径名
//返回：若成功则返回TRUE，失败则返回FALSE

BOOL FileReadWnd(HWND hEdit, PTSTR pstrFileName);


///////////////////////////////////////////////////////
//FileWriteWnd
//功能：写入文件
//参数：编辑框句柄hwnd，文件名
//返回：若成功则返回TRUE，失败则返回FALSE

BOOL FileWriteWnd(HWND hEdit, PTSTR pstrFileName);


///////////////////////////////////////////////////////
//SetTitle
//功能：设置窗口内容
//参数：编辑框句柄hwnd，文件名
//返回：无返回值

void SetTitle(HWND hwnd, TCHAR * szTitleName);


///////////////////////////////////////////////////////
//DrawComInf
//功能：绘制楼盘信息窗口
//参数：编辑框句柄hwnd，设备环境hdc，Community结构
//返回：成功返回TRUE，失败返回FALSE

BOOL DrawComInf(HWND hwnd, HDC hdc, Community com);


///////////////////////////////////////////////////////
//DrawBuiInf
//功能：绘制楼栋信息窗口
//参数：编辑框句柄hwnd，设备环境hdc，Building结构
//返回：成功返回TRUE，失败返回FALSE

BOOL DrawBuiInf(HWND hwnd, HDC hdc, Building bui);


///////////////////////////////////////////////////////
//DrawRoomInf
//功能：绘制楼栋信息窗口
//参数：编辑框句柄hwnd，设备环境hdc，Room结构
//返回：成功返回TRUE，失败返回FALSE

BOOL DrawRoomInf(HWND hwnd, HDC hdc, Room room);


///////////////////////////////////////////////////////
//RenewData
//功能：将数据区恢复为背景色
//参数：设备环境hdc，Building结构
//返回：无返回

void RenewData(HDC hdc, RECT rectData);


///////////////////////////////////////////////////////
//Draw1Step
//功能：WM_PAINT消息中绘制第一级(包含Com与Bui)，不显示数据
//参数：设备环境hdc，显示数据的com指针
//返回：无返回

void Draw1Step(HDC hdc, Community *pChosenCom);



///////////////////////////////////////////////////////
//Draw2Step
//功能：WM_PAINT消息中绘制第二级，不显示数据
//参数：设备环境hdc,指向所选楼栋的指针
//返回：无返回

void Draw2Step(HDC hdc, Building *pBui);


///////////////////////////////////////////////////////
//UpdataData
//功能：更新整个链表的数据，包括房间数，均价等
//参数：将要更新的链表的头指针的地址
//返回：若完成所有更新则返回TRUE，否则返回FALSE

BOOL UpdateData(Community **ppHead);


///////////////////////////////////////////////////////
//DelComInList
//功能：删除链表中的一个Community节点
//参数：将要删除的Community编号，添加对象链表的头指针的地址
//返回：返回更新后链表头指针

BOOL DelComInList(int comNum, Community **head);


///////////////////////////////////////////////////////
//DelBuiInList
//功能：删除链表中的一个Building节点
//参数：将要删除的Building编号，添加对象链表的头指针的地址
//返回：返回更新后链表头指针

BOOL DelBuiInList(int comNum,int buiNum, Community **head);


///////////////////////////////////////////////////////
//DelRoomInList
//功能：删除链表中的一个Room节点
//参数：将要删除的Room编号，添加对象链表的头指针的地址
//返回：返回更新后链表头指针

BOOL DelRoomInList(int comNum, int buiNum, int roomNum, Community **head);


///////////////////////////////////////////////////////
//“删除楼盘”（DeleteCommunity）对话框窗口过程

BOOL CALLBACK DelComDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

///////////////////////////////////////////////////////
//“删除楼盘”（DeleteBui）对话框窗口过程

BOOL CALLBACK DelBuiDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);


///////////////////////////////////////////////////////
//“删除楼盘”（DeleteRoom）对话框窗口过程

BOOL CALLBACK DelRoomDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);


///////////////////////////////////////////////////////
//绘制按钮
//参数：hdc，信息窗口右下角的坐标

void DrawButton(HDC hdc, int x, int y);


///////////////////////////////////////////////////////
//“编辑楼盘信息”（EditComDlgProc）对话框窗口过程

BOOL CALLBACK EditComDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);


///////////////////////////////////////////////////////
//“编辑楼栋信息”（EditBuiDlgProc）对话框窗口过程

BOOL CALLBACK EditBuiDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);


///////////////////////////////////////////////////////
//“编辑房间信息”（EditRoomDlgProc）对话框窗口过程

BOOL CALLBACK EditRoomDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);


///////////////////////////////////////////////////////
//计算出每一个楼盘的相似度
//参数：标准SearchData结构，对比的楼盘
//返回：这个输入信息与该楼盘的相似度

int GetComSim(SearchData data, Community com);


///////////////////////////////////////////////////////
//计算出每一个楼栋的相似度
//参数：标准SearchData结构，对比的楼栋，该楼栋所在楼盘
//返回：这个输入信息与该楼栋的相似度

int GetBuiSim(SearchData data, Community com, Building bui);


///////////////////////////////////////////////////////
//计算出每一个房间的相似度
//参数：标准SearchData结构，对比的房间，该房间所在楼栋、楼盘
//返回：这个输入信息与该房间的相似度

int GetRoomSim(SearchData data, Community com, Building bui, Room room);


///////////////////////////////////////////////////////
//“搜索功能”（Search）对话框窗口过程

BOOL CALLBACK SearchDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);


///////////////////////////////////////////////////////
//对Com链表按照权重进行排序
//参数：标准比较输入数据，排序的链表头指针
//返回：返回为排完序的链表的头指针

Community *RankCom(SearchData sData, Community *pComSearch);

///////////////////////////////////////////////////////
//对Bui链表按照权重进行排序
//参数：标准比较输入数据，排序的链表头指针
//返回：返回为排完序的链表的头指针

Building *RankBui(SearchData sData, Building *pBuiSearch);


///////////////////////////////////////////////////////
//AddBuiToSearch
//功能：创建一个只由bui组成的search
//参数：将要添加的Building结构体，添加对象链表的头指针
//返回：成功则返回TRUE，失败返回FALSE

BOOL AddBuiToSearch(Building newCom, Building **head);


///////////////////////////////////////////////////////
//对Room链表按照权重进行排序
//参数：标准比较输入数据，排序的链表头指针
//返回：返回为排完序的链表的头指针

Room *RankRoom(SearchData sData, Room *pBuiSearch);


///////////////////////////////////////////////////////
//检查是否有重复的楼栋
//参数：楼盘号，排序的链表头指针
//返回：如没有返回TRUE，有返回FALSE

BOOL CheckSameCom(TCHAR comName[], Community **head);


///////////////////////////////////////////////////////
//检查是否有重复的楼栋
//参数：所在楼盘名，楼栋号，排序的链表头指针
//返回：如没有返回TRUE，有返回FALSE

BOOL CheckSameBui(TCHAR comName[], int buiNum, Community **head);


///////////////////////////////////////////////////////
//检查是否有重复的房间
//参数：所在楼盘名，所在楼栋号，房间号，排序的链表头指针
//返回：如没有返回TRUE，有返回FALSE

BOOL CheckSameRoom(TCHAR comName[], int buiNum, int roomNum, Community **head);