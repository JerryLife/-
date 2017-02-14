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

#define XLINEPOS (1.0 / 4 * cxClient)			//��������ֱ�ֽ���
#define YLINEPOS ((1 - 0.618) * cyClient)		//������ˮƽ�ֽ���

#define Swap(x,y) (x)=(y)-(x);(x)=(x)+(y);(y)=(x)-(y);	//����x��y��ֵ

///////////////////////////////////////////////////////
//�����ڴ��ڹ���

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


///////////////////////////////////////////////////////
//AskConfirm 
//���ܣ����û��˳�ϵͳʱ�����ļ�δ���棬����ʾ���Ƿ���Ҫ����
//     ���ļ��Ѿ����棬��ֱ���˳�
//�������˳��Ĵ��ھ��������״̬����0Ϊ�ѱ��棬0Ϊδ���棩
//���أ�����Ҫ�˳�ϵͳ���򷵻�TRUE��������Ҫ���򷵻�FALSE

BOOL AskConfirm(HWND hwnd, int iSaveState);


///////////////////////////////////////////////////////
//DrawBasicBk
//���ڻ������ֹ��캯��
//���ܣ��������ڵĻ������֣������κ���Ϣ
//���������ھ��,�豸����,��Ļ����Ļ��
//���أ����ݴ��ڵ�RECT����

RECT DrawBasicBk(HWND hwnd, HDC hdc, int cxClient, int cyClient);


///////////////////////////////////////////////////////
//�����ڡ�¥�̹���ϵͳ������About���Ի��򴰿ڹ���

BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);


///////////////////////////////////////////////////////
//�������¥�̡���AddCommunity���Ի��򴰿ڹ���

BOOL CALLBACK AddComDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);


///////////////////////////////////////////////////////
//�������¥������AddBuilding���Ի��򴰿ڹ���

BOOL CALLBACK AddBuiDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);


///////////////////////////////////////////////////////
//������·��䡱��AddRoom���Ի��򴰿ڹ���

BOOL CALLBACK AddRoomDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);


///////////////////////////////////////////////////////
//��ϵ�˽ṹ��
//���ܣ����ڴ洢������Ϣ
typedef struct _Person {
	TCHAR name[100];		//����
	TCHAR phone[50];		//�绰
	TCHAR idNumber[50];		//֤��ID��
}Person;


///////////////////////////////////////////////////////
//����ṹ��
//���ܣ����淿�������Ϣ���������ڹ�������
typedef struct _Room {
	int roomNum;				//�����
	int buildingNum;			//����¥����
	int communityNum;			//����¥�̺�
	int floor;					//����¥��
	float roomSize;				//���������ƽ���ף�
	float roomPrice;			//����۸������Ԫ/ƽ���ף�
	long allPrice;				//�����ܼۣ������Ԫ��
	TCHAR roomType[100];		//���ͣ��ң������ޣ�������
	TCHAR inCom[100];			//����¥����
	BOOL roomState;				//�۳�״̬��0����δ�۳���1�������۳�
	TCHAR note[1000];			//��ע
	Person host;				//��ϵ�˽ṹ
	struct _Room *nextRoom;		//ָ����������һ������
}Room;


///////////////////////////////////////////////////////
//¥���ṹ��
//���ܣ�����¥�������Ϣ���������ڹ�������
typedef struct _Building {
	int buildingNum;					//¥����
	int communityNum;					//����¥�̺�
	int numberOfRooms;					//���з�����
	int numberOfFloors;					//¥����
	float avgPrice;						//ƽ���۸�
	TCHAR inCom[100];					//����¥������
	Person host;						//��ϵ�˽ṹ
	struct _Building *nextBuilding;		//ָ����һ��¥��
	struct _Room *rooms;				//ָ���¥����һ������
}Building;


///////////////////////////////////////////////////////
//¥�̽ṹ��
//���ܣ�����¥�������Ϣ���������ڹ�������
typedef struct _Community {
	int communityNum;								//¥�̺�
	TCHAR name[100];								//¥����
	TCHAR address[100];								//¥�̵�ַ
	TCHAR phone[30];								//��ϵ�绰
	int numberOfBuildings;							//����¥����
	int numberOfRooms;								//���з�����
	float avgPrice;									//ƽ���۸�
	Person host;									//��ϵ�˽ṹ
	struct _Community *nextCommunity = NULL;		//ָ����һ��¥��
	struct _Building *buildings = NULL;				//ָ���¥�̵�һ��¥��
}Community;

/////////////////////////////////////////////////////////
//*.led�ļ��еĽṹ��
//����com��bui��room�����ļ�������������������
typedef struct _LenData {
	int iCom;
	int iBui;
	int iRoom;
}LenData;


/////////////////////////////////////////////////////////
//������ʹ�õĽṹ�壬������������������
typedef struct _SearchData {

	int mask;			//ȷ�������Ķ���
						//ֵΪSD_COM,SD_BUI,SD_ROOM�е�һ��
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
//���ܣ�����һ��community���Ƚ��ȳ��ĵ�������
//������������
//���أ�������ͷָ��

Community *createComList(int len);


///////////////////////////////////////////////////////
//createBuiList
//���ܣ�����һ��building���Ƚ��ȳ��ĵ�������
//������������
//���أ�������ͷָ��

Building *createBuiList(int len);


///////////////////////////////////////////////////////
//createRoomList
//���ܣ�����һ��room���Ƚ��ȳ��ĵ�������
//������������
//���أ�������ͷָ��

Room *createRoomList(int len);

///////////////////////////////////////////////////////
//create3DepthList
//���ܣ�����һ����������
//������1����һ��������
//     2��ÿ����һ�������ڶ�������
//     3��ÿ���ڶ�����������������
//���أ����ظ����������ͷָ��

Community *create3DepthList(int len1, int len2, int len3);


///////////////////////////////////////////////////////
//AddComToList
//���ܣ������������һ��community�ڵ�
//��������Ҫ��ӵ�Community�ṹ�壨��Ӷ��������ͷָ��.ȫ�ֱ����ṩ��
//���أ����ظ��º�����ͷָ��

BOOL AddComToList(Community newCom, Community **head);


///////////////////////////////////////////////////////
//AddBuiToList
//���ܣ������������һ��building�ڵ�
//��������Ҫ��ӵ�Building�ṹ�壬��Ӷ��������ͷָ�룬����¥�̺�
//���أ��ɹ��򷵻�TRUE��ʧ�ܷ���FALSE

BOOL AddBuiToList(Building newBui, Community **head, int comNum);


///////////////////////////////////////////////////////
//AddRoomToList
//���ܣ������������һ��Room�ڵ�
//��������Ҫ��ӵ�Room�ṹ�壬��Ӷ��������ͷָ�룬����¥�̺ţ�����¥����
//���أ��ɹ��򷵻�TRUE��ʧ�ܷ���FALSE

BOOL AddRoomToList(Room newRoom, Community **head,
	int comNum, int buiNum);


///////////////////////////////////////////////////////
//FileInitWnd
//���ܣ���ʼ�������ļ����ڵ�����
//�����������ھ��hwnd
//���أ��޷���ֵ

void FileInitWnd(HWND hwnd);


///////////////////////////////////////////////////////
//FileImportDlg
//���ܣ����������ļ��Ի���
//�����������ھ��hwnd���ļ�����·����
//���أ����ɹ��򷵻�TRUE��ʧ���򷵻�FALSE

BOOL FileImportDlg(HWND hwnd, PTSTR pstrFileName, PTSTR pstrTitleName);


///////////////////////////////////////////////////////
//FileSaveDlg
//���ܣ����������ļ��Ի���
//���������ھ��hwnd���ļ�����·����
//���أ����ɹ��򷵻�TRUE��ʧ���򷵻�FALSE

BOOL FileSaveDlg(HWND hwnd, PTSTR pstrFileName, PTSTR pstrTitleName);


///////////////////////////////////////////////////////
//FileReadWnd
//���ܣ����������ļ��Ի���
//���������ھ��hwnd���ļ�����·����
//���أ����ɹ��򷵻�TRUE��ʧ���򷵻�FALSE

BOOL FileReadWnd(HWND hEdit, PTSTR pstrFileName);


///////////////////////////////////////////////////////
//FileWriteWnd
//���ܣ�д���ļ�
//�������༭����hwnd���ļ���
//���أ����ɹ��򷵻�TRUE��ʧ���򷵻�FALSE

BOOL FileWriteWnd(HWND hEdit, PTSTR pstrFileName);


///////////////////////////////////////////////////////
//SetTitle
//���ܣ����ô�������
//�������༭����hwnd���ļ���
//���أ��޷���ֵ

void SetTitle(HWND hwnd, TCHAR * szTitleName);


///////////////////////////////////////////////////////
//DrawComInf
//���ܣ�����¥����Ϣ����
//�������༭����hwnd���豸����hdc��Community�ṹ
//���أ��ɹ�����TRUE��ʧ�ܷ���FALSE

BOOL DrawComInf(HWND hwnd, HDC hdc, Community com);


///////////////////////////////////////////////////////
//DrawBuiInf
//���ܣ�����¥����Ϣ����
//�������༭����hwnd���豸����hdc��Building�ṹ
//���أ��ɹ�����TRUE��ʧ�ܷ���FALSE

BOOL DrawBuiInf(HWND hwnd, HDC hdc, Building bui);


///////////////////////////////////////////////////////
//DrawRoomInf
//���ܣ�����¥����Ϣ����
//�������༭����hwnd���豸����hdc��Room�ṹ
//���أ��ɹ�����TRUE��ʧ�ܷ���FALSE

BOOL DrawRoomInf(HWND hwnd, HDC hdc, Room room);


///////////////////////////////////////////////////////
//RenewData
//���ܣ����������ָ�Ϊ����ɫ
//�������豸����hdc��Building�ṹ
//���أ��޷���

void RenewData(HDC hdc, RECT rectData);


///////////////////////////////////////////////////////
//Draw1Step
//���ܣ�WM_PAINT��Ϣ�л��Ƶ�һ��(����Com��Bui)������ʾ����
//�������豸����hdc����ʾ���ݵ�comָ��
//���أ��޷���

void Draw1Step(HDC hdc, Community *pChosenCom);



///////////////////////////////////////////////////////
//Draw2Step
//���ܣ�WM_PAINT��Ϣ�л��Ƶڶ���������ʾ����
//�������豸����hdc,ָ����ѡ¥����ָ��
//���أ��޷���

void Draw2Step(HDC hdc, Building *pBui);


///////////////////////////////////////////////////////
//UpdataData
//���ܣ�����������������ݣ����������������۵�
//��������Ҫ���µ������ͷָ��ĵ�ַ
//���أ���������и����򷵻�TRUE�����򷵻�FALSE

BOOL UpdateData(Community **ppHead);


///////////////////////////////////////////////////////
//DelComInList
//���ܣ�ɾ�������е�һ��Community�ڵ�
//��������Ҫɾ����Community��ţ���Ӷ��������ͷָ��ĵ�ַ
//���أ����ظ��º�����ͷָ��

BOOL DelComInList(int comNum, Community **head);


///////////////////////////////////////////////////////
//DelBuiInList
//���ܣ�ɾ�������е�һ��Building�ڵ�
//��������Ҫɾ����Building��ţ���Ӷ��������ͷָ��ĵ�ַ
//���أ����ظ��º�����ͷָ��

BOOL DelBuiInList(int comNum,int buiNum, Community **head);


///////////////////////////////////////////////////////
//DelRoomInList
//���ܣ�ɾ�������е�һ��Room�ڵ�
//��������Ҫɾ����Room��ţ���Ӷ��������ͷָ��ĵ�ַ
//���أ����ظ��º�����ͷָ��

BOOL DelRoomInList(int comNum, int buiNum, int roomNum, Community **head);


///////////////////////////////////////////////////////
//��ɾ��¥�̡���DeleteCommunity���Ի��򴰿ڹ���

BOOL CALLBACK DelComDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

///////////////////////////////////////////////////////
//��ɾ��¥�̡���DeleteBui���Ի��򴰿ڹ���

BOOL CALLBACK DelBuiDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);


///////////////////////////////////////////////////////
//��ɾ��¥�̡���DeleteRoom���Ի��򴰿ڹ���

BOOL CALLBACK DelRoomDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);


///////////////////////////////////////////////////////
//���ư�ť
//������hdc����Ϣ�������½ǵ�����

void DrawButton(HDC hdc, int x, int y);


///////////////////////////////////////////////////////
//���༭¥����Ϣ����EditComDlgProc���Ի��򴰿ڹ���

BOOL CALLBACK EditComDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);


///////////////////////////////////////////////////////
//���༭¥����Ϣ����EditBuiDlgProc���Ի��򴰿ڹ���

BOOL CALLBACK EditBuiDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);


///////////////////////////////////////////////////////
//���༭������Ϣ����EditRoomDlgProc���Ի��򴰿ڹ���

BOOL CALLBACK EditRoomDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);


///////////////////////////////////////////////////////
//�����ÿһ��¥�̵����ƶ�
//��������׼SearchData�ṹ���Աȵ�¥��
//���أ����������Ϣ���¥�̵����ƶ�

int GetComSim(SearchData data, Community com);


///////////////////////////////////////////////////////
//�����ÿһ��¥�������ƶ�
//��������׼SearchData�ṹ���Աȵ�¥������¥������¥��
//���أ����������Ϣ���¥�������ƶ�

int GetBuiSim(SearchData data, Community com, Building bui);


///////////////////////////////////////////////////////
//�����ÿһ����������ƶ�
//��������׼SearchData�ṹ���Աȵķ��䣬�÷�������¥����¥��
//���أ����������Ϣ��÷�������ƶ�

int GetRoomSim(SearchData data, Community com, Building bui, Room room);


///////////////////////////////////////////////////////
//���������ܡ���Search���Ի��򴰿ڹ���

BOOL CALLBACK SearchDlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);


///////////////////////////////////////////////////////
//��Com������Ȩ�ؽ�������
//��������׼�Ƚ��������ݣ����������ͷָ��
//���أ�����Ϊ������������ͷָ��

Community *RankCom(SearchData sData, Community *pComSearch);

///////////////////////////////////////////////////////
//��Bui������Ȩ�ؽ�������
//��������׼�Ƚ��������ݣ����������ͷָ��
//���أ�����Ϊ������������ͷָ��

Building *RankBui(SearchData sData, Building *pBuiSearch);


///////////////////////////////////////////////////////
//AddBuiToSearch
//���ܣ�����һ��ֻ��bui��ɵ�search
//��������Ҫ��ӵ�Building�ṹ�壬��Ӷ��������ͷָ��
//���أ��ɹ��򷵻�TRUE��ʧ�ܷ���FALSE

BOOL AddBuiToSearch(Building newCom, Building **head);


///////////////////////////////////////////////////////
//��Room������Ȩ�ؽ�������
//��������׼�Ƚ��������ݣ����������ͷָ��
//���أ�����Ϊ������������ͷָ��

Room *RankRoom(SearchData sData, Room *pBuiSearch);


///////////////////////////////////////////////////////
//����Ƿ����ظ���¥��
//������¥�̺ţ����������ͷָ��
//���أ���û�з���TRUE���з���FALSE

BOOL CheckSameCom(TCHAR comName[], Community **head);


///////////////////////////////////////////////////////
//����Ƿ����ظ���¥��
//����������¥������¥���ţ����������ͷָ��
//���أ���û�з���TRUE���з���FALSE

BOOL CheckSameBui(TCHAR comName[], int buiNum, Community **head);


///////////////////////////////////////////////////////
//����Ƿ����ظ��ķ���
//����������¥����������¥���ţ�����ţ����������ͷָ��
//���أ���û�з���TRUE���з���FALSE

BOOL CheckSameRoom(TCHAR comName[], int buiNum, int roomNum, Community **head);