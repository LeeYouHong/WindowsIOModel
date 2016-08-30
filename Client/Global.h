
#define NAME_LEN            33
#define PASS_LEN            33

//数组元素数
#define				CountArray(Array)			(sizeof(Array)/sizeof(Array[0]))

//网络数据定义
#define SOCKET_VER						0x66								//网络版本
#define SOCKET_BUFFER					8192								//网络缓冲
#define SOCKET_PACKET					(SOCKET_BUFFER-sizeof(CMD_Head)-2*sizeof(DWORD))
#define MESSAGE_BUFFER                  2000                                //聊天消息

//内核命令码
#define MDM_KN_COMMAND					0									//内核命令
#define SUB_KN_DETECT_SOCKET			1									//检测命令
#define SUB_KN_SHUT_DOWN_SOCKET			2									//中断网络



//用户操作命令码
#define MDM_GP_USER						4								//用户信息

#define SUB_GP_MODIFY_INDIVIDUAL		105								//个人资料
#define SUB_GP_MODIFY_INDIVIDUAL_RESULT	106								//修改结果

//网络内核
struct CMD_Info
{
	BYTE								cbVersion;							//版本标识
	BYTE								cbCheckCode;						//效验字段
	WORD								wPacketSize;						//数据大小
};

//网络命令
struct CMD_Command
{
	WORD								wMainCmdID;							//主命令码
	WORD								wSubCmdID;							//子命令码
};

//网络包头
struct CMD_Head
{
	CMD_Info							CmdInfo;							//基础结构
	CMD_Command							CommandInfo;						//命令信息
};

//网络包缓冲
struct CMD_Buffer
{
	CMD_Head							Head;								//数据包头
	BYTE								cbBuffer[SOCKET_PACKET];			//数据缓冲
};

//检测结构信息
struct CMD_KN_DetectSocket
{
	DWORD								dwSendTickCount;					//发送时间
	DWORD								dwRecvTickCount;					//接收时间
};

//登录命令码

#define MDM_GP_LOGON					1								//广场登录

#define SUB_GP_LOGON_ACCOUNTS			1								//帐号登录
#define SUB_GP_LOGON_USERID				2								//I D 登录
#define SUB_GP_REGISTER_ACCOUNTS		3								//注册帐号

#define SUB_GP_LOGON_SUCCESS			100								//登陆成功
#define SUB_GP_LOGON_ERROR				101								//登陆失败
#define SUB_GP_LOGON_FINISH				102								//登陆完成

//帐号登录
struct CMD_GP_LogonByAccounts
{
	DWORD								dwPlazaVersion;					//广场版本
	char								szAccounts[NAME_LEN];			//登录帐号
	char								szPassWord[PASS_LEN];			//登录密码
};

//I D 登录
struct CMD_GP_LogonByUserID
{
	DWORD								dwPlazaVersion;					//广场版本
	DWORD								dwUserID;						//用户 I D
	char								szPassWord[PASS_LEN];			//登录密码
};

//注册帐号
struct CMD_GP_RegisterAccounts
{
	BYTE								cbGender;						//用户性别
	DWORD								dwPlazaVersion;					//广场版本
	char								szAccounts[NAME_LEN];			//登录帐号
	char								szPassWord[PASS_LEN];			//登录密码
};

//登陆成功
struct CMD_GP_LogonSuccess
{
	BYTE								cbGender;						//用户性别
	DWORD								dwUserID;						//用户 I D
};

//登陆失败
struct CMD_GP_LogonError
{
	long								lErrorCode;						//错误代码
	char								szErrorDescribe[128];			//错误消息
};
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//数据库对象定义

//查询数据库错误码
#define DB_REGISTER_SUCCESS         1                                   //用户在数据库中注册成功
#define DB_REGISTER_USER_EXISTE     2                                   //用户已经存在
#define DB_REGISTER_ERROR           3                                   //数据库注册用户出错
//数据库请求标识
#define	DBR_GP_LOGON_BY_ACCOUNTS	1									//帐号登录
#define	DBR_GP_LOGON_BY_USERID		2									//I D 登录
#define DBR_GP_REGISTER_ACCOUNTS	3									//注册帐号
#define DBR_GP_USER_LEAVE			5									//玩家离开
#define DBR_GP_MODIFY_INDIVIDUAL	8									//修改资料


//数据库输出标识
#define DBR_GP_LOGON_SUCCESS		100									//登录成功
#define DBR_GP_LOGON_ERROR			101									//登录失败
#define DBR_GP_MODIFY_RESULT		105									//修改结果

//帐号登录
struct DBR_GP_LogonByAccounts
{
	char							szAccounts[NAME_LEN];				//登录帐号
	char							szPassWord[PASS_LEN];				//登录密码
};

//ID 登录
struct DBR_GP_LogonByUserID
{
	DWORD							dwUserID;							//用户 I D
	char							szPassWord[PASS_LEN];				//登录密码
};

//帐号注册
struct DBR_GP_RegisterAccounts
{
	BYTE							cbGender;							//用户性别
	char							szAccounts[NAME_LEN];				//登录帐号
	char							szPassWord[PASS_LEN];				//登录密码
};

//消息传送
#define  MDM_GP_CHAT                 2                                  //聊天主信息
#define  SUB_GP_CHAT_SINGLE          1                                  //单个聊天
#define  SUB_GP_CHAT_ALL             2                                  //群发

//单聊
struct CMD_GP_ChatBySingle
{
	char								szSender[NAME_LEN];			//发送用户
	char								szRecver[NAME_LEN];			//接受用户
	char                                szMessage[MESSAGE_BUFFER];   //聊天消息
};