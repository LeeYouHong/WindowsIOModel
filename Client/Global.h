
#define NAME_LEN            33
#define PASS_LEN            33

//����Ԫ����
#define				CountArray(Array)			(sizeof(Array)/sizeof(Array[0]))

//�������ݶ���
#define SOCKET_VER						0x66								//����汾
#define SOCKET_BUFFER					8192								//���绺��
#define SOCKET_PACKET					(SOCKET_BUFFER-sizeof(CMD_Head)-2*sizeof(DWORD))
#define MESSAGE_BUFFER                  2000                                //������Ϣ

//�ں�������
#define MDM_KN_COMMAND					0									//�ں�����
#define SUB_KN_DETECT_SOCKET			1									//�������
#define SUB_KN_SHUT_DOWN_SOCKET			2									//�ж�����



//�û�����������
#define MDM_GP_USER						4								//�û���Ϣ

#define SUB_GP_MODIFY_INDIVIDUAL		105								//��������
#define SUB_GP_MODIFY_INDIVIDUAL_RESULT	106								//�޸Ľ��

//�����ں�
struct CMD_Info
{
	BYTE								cbVersion;							//�汾��ʶ
	BYTE								cbCheckCode;						//Ч���ֶ�
	WORD								wPacketSize;						//���ݴ�С
};

//��������
struct CMD_Command
{
	WORD								wMainCmdID;							//��������
	WORD								wSubCmdID;							//��������
};

//�����ͷ
struct CMD_Head
{
	CMD_Info							CmdInfo;							//�����ṹ
	CMD_Command							CommandInfo;						//������Ϣ
};

//���������
struct CMD_Buffer
{
	CMD_Head							Head;								//���ݰ�ͷ
	BYTE								cbBuffer[SOCKET_PACKET];			//���ݻ���
};

//���ṹ��Ϣ
struct CMD_KN_DetectSocket
{
	DWORD								dwSendTickCount;					//����ʱ��
	DWORD								dwRecvTickCount;					//����ʱ��
};

//��¼������

#define MDM_GP_LOGON					1								//�㳡��¼

#define SUB_GP_LOGON_ACCOUNTS			1								//�ʺŵ�¼
#define SUB_GP_LOGON_USERID				2								//I D ��¼
#define SUB_GP_REGISTER_ACCOUNTS		3								//ע���ʺ�

#define SUB_GP_LOGON_SUCCESS			100								//��½�ɹ�
#define SUB_GP_LOGON_ERROR				101								//��½ʧ��
#define SUB_GP_LOGON_FINISH				102								//��½���

//�ʺŵ�¼
struct CMD_GP_LogonByAccounts
{
	DWORD								dwPlazaVersion;					//�㳡�汾
	char								szAccounts[NAME_LEN];			//��¼�ʺ�
	char								szPassWord[PASS_LEN];			//��¼����
};

//I D ��¼
struct CMD_GP_LogonByUserID
{
	DWORD								dwPlazaVersion;					//�㳡�汾
	DWORD								dwUserID;						//�û� I D
	char								szPassWord[PASS_LEN];			//��¼����
};

//ע���ʺ�
struct CMD_GP_RegisterAccounts
{
	BYTE								cbGender;						//�û��Ա�
	DWORD								dwPlazaVersion;					//�㳡�汾
	char								szAccounts[NAME_LEN];			//��¼�ʺ�
	char								szPassWord[PASS_LEN];			//��¼����
};

//��½�ɹ�
struct CMD_GP_LogonSuccess
{
	BYTE								cbGender;						//�û��Ա�
	DWORD								dwUserID;						//�û� I D
};

//��½ʧ��
struct CMD_GP_LogonError
{
	long								lErrorCode;						//�������
	char								szErrorDescribe[128];			//������Ϣ
};
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//���ݿ������

//��ѯ���ݿ������
#define DB_REGISTER_SUCCESS         1                                   //�û������ݿ���ע��ɹ�
#define DB_REGISTER_USER_EXISTE     2                                   //�û��Ѿ�����
#define DB_REGISTER_ERROR           3                                   //���ݿ�ע���û�����
//���ݿ������ʶ
#define	DBR_GP_LOGON_BY_ACCOUNTS	1									//�ʺŵ�¼
#define	DBR_GP_LOGON_BY_USERID		2									//I D ��¼
#define DBR_GP_REGISTER_ACCOUNTS	3									//ע���ʺ�
#define DBR_GP_USER_LEAVE			5									//����뿪
#define DBR_GP_MODIFY_INDIVIDUAL	8									//�޸�����


//���ݿ������ʶ
#define DBR_GP_LOGON_SUCCESS		100									//��¼�ɹ�
#define DBR_GP_LOGON_ERROR			101									//��¼ʧ��
#define DBR_GP_MODIFY_RESULT		105									//�޸Ľ��

//�ʺŵ�¼
struct DBR_GP_LogonByAccounts
{
	char							szAccounts[NAME_LEN];				//��¼�ʺ�
	char							szPassWord[PASS_LEN];				//��¼����
};

//ID ��¼
struct DBR_GP_LogonByUserID
{
	DWORD							dwUserID;							//�û� I D
	char							szPassWord[PASS_LEN];				//��¼����
};

//�ʺ�ע��
struct DBR_GP_RegisterAccounts
{
	BYTE							cbGender;							//�û��Ա�
	char							szAccounts[NAME_LEN];				//��¼�ʺ�
	char							szPassWord[PASS_LEN];				//��¼����
};

//��Ϣ����
#define  MDM_GP_CHAT                 2                                  //��������Ϣ
#define  SUB_GP_CHAT_SINGLE          1                                  //��������
#define  SUB_GP_CHAT_ALL             2                                  //Ⱥ��

//����
struct CMD_GP_ChatBySingle
{
	char								szSender[NAME_LEN];			//�����û�
	char								szRecver[NAME_LEN];			//�����û�
	char                                szMessage[MESSAGE_BUFFER];   //������Ϣ
};