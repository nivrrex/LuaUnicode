#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include <windows.h>

#define BUFF_SIZE 1024
#define CHAR_SCALE (sizeof(wchar_t)/sizeof(char))
#define CMD_BUFF_SIZE 4096

wchar_t * AToU (const char *str)
{
	int textlen;
	wchar_t *result;

	textlen = MultiByteToWideChar (CP_ACP, 0, str, -1, NULL, 0);
	result = (wchar_t *) malloc ((textlen + 1) * sizeof (wchar_t));
	memset (result, 0, (textlen + 1) * sizeof (wchar_t));
	MultiByteToWideChar (CP_ACP, 0, str, -1, (LPWSTR) result, textlen);
	return result;
}

char * UToA (const wchar_t * str)
{
	char *result;
	int textlen;

	// wide char to multi char
	textlen = WideCharToMultiByte (CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
	result = (char *) malloc ((textlen + 1) * sizeof (char));
	memset (result, 0, sizeof (char) * (textlen + 1));
	WideCharToMultiByte (CP_ACP, 0, str, -1, result, textlen, NULL, NULL);
	return result;
}

wchar_t * U8ToU (const char *str)
{
	int textlen;
	wchar_t *result;

	textlen = MultiByteToWideChar (CP_UTF8, 0, str, -1, NULL, 0);
	result = (wchar_t *) malloc ((textlen + 1) * sizeof (wchar_t));
	memset (result, 0, (textlen + 1) * sizeof (wchar_t));
	MultiByteToWideChar (CP_UTF8, 0, str, -1, (LPWSTR) result, textlen);
	return result;
}

char * UToU8 (const wchar_t * str)
{
	char *result;
	int textlen;

	// wide char to multi char
	textlen = WideCharToMultiByte (CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
	result = (char *) malloc ((textlen + 1) * sizeof (char));
	memset (result, 0, sizeof (char) * (textlen + 1));
	WideCharToMultiByte (CP_UTF8, 0, str, -1, result, textlen, NULL, NULL);
	return result;
}

static int Unicode_a2u (lua_State * L)
{
	const char *str;
	wchar_t *result;

	/*���ݵ�һ������ */
	str = lua_tostring (L, -1);
	/*��ʼת�� */
	result = AToU (str);
	/*����ֵ�� */
	lua_pushlstring (L, (char *) result, wcslen (result) * CHAR_SCALE);
    free(result);
	return 1;
}

static int Unicode_u2a (lua_State * L)
{
	const wchar_t *str;
	char *result;

	/*���ݵ�һ������ */
	str = (wchar_t *) lua_tostring (L, -1);
	/*��ʼת�� */
	result = UToA (str);
	/*����ֵ�� */
	lua_pushstring (L, result);
    free(result);
	return 1;
}

static int Unicode_u2u8 (lua_State * L)
{
	const wchar_t *str;
	char *result;

	/*���ݵ�һ������ */
	str = (wchar_t *) lua_tostring (L, -1);
	/*��ʼת�� */
	result = UToU8 (str);
	/*����ֵ�� */
	lua_pushstring (L, result);
    free(result);
	return 1;
}

static int Unicode_u82u (lua_State * L)
{
	const char *str;
	wchar_t *result;

	/*���ݵ�һ������ */
	str = lua_tostring (L, -1);
	/*��ʼת�� */
	result = U8ToU (str);
	/*����ֵ�� */
	lua_pushlstring (L, (char *) result, wcslen (result) * CHAR_SCALE);
    free(result);
	return 1;
}

static int Unicode_a2u8 (lua_State * L)
{
	const char *str;
	wchar_t *temp;
	char *result;

	/*���ݵ�һ������ */
	str = lua_tostring (L, -1);
	/*��ʼת�� */
	temp = AToU (str);
	result = UToU8 (temp);
	/*����ֵ�� */
	lua_pushstring (L, result);
    free(result);
	return 1;
}

static int Unicode_u82a (lua_State * L)
{
	const char *str;
	wchar_t *temp;
	char *result;

	/*���ݵ�һ������ */
	str = lua_tostring (L, -1);
	/*��ʼת�� */
	temp = U8ToU (str);
	result = UToA (temp);
	/*����ֵ�� */
	lua_pushstring (L, result);
    free(result);
	return 1;
}

/*��ȡһ���ļ���С*/
static int _GetFileSize (const char *filename)
{
	long len;
	FILE *fp;

	/*��ֻ�����ļ���seek���ļ�ĩβ�ķ�ʽ��ȡ�ļ���С */
	if ((fp = fopen (filename, "r")) == NULL)
	{
		printf ("%s is not invalid\n", filename);
		return 0;
	}
	fseek (fp, 0, SEEK_END);
	len = ftell (fp);
	fclose (fp);
	return len;
}

/*Lua ��ȡ �ļ���С*/
static int GetFileSizeW (lua_State * L)
{
	/*���ݵ�һ���������ļ��� */
	const char *filename = lua_tostring (L, -1);
	lua_pushinteger (L, _GetFileSize (filename));
	return 1;
}

/*��ȡһ��Unicode�ļ���ʹ��fgetwc������IO�����ٶȽ���*/
static int GetAllFileWC (lua_State * L)
{
	/*���ݵ�һ���������ļ��� */
	const char *filename = lua_tostring (L, -1);
	/*��ȡ�ļ���С */
	int len;
	len = _GetFileSize (filename);
	/*���û����С */
	wchar_t *buf;
	/*����wchar_t����Ϊchar����һ��������buf�ռ��СΪ�ļ�����һ�룬�ټ�ĩβ�� '\0' */
	buf = (wchar_t *) malloc (sizeof (wchar_t) * (len / CHAR_SCALE + 1));

	int i = 0;
	FILE *input = fopen (filename, "rb");
	while (!feof (input))
	{
		buf[i++] = fgetwc (input);
	}
	/*�ַ���ĩβ���� */
	buf[i - 1] = L'\0';
	lua_pushlstring (L, (char *) buf, wcslen (buf) * CHAR_SCALE);
	free (buf);
	return 1;
}

/*��ȡһ��Unicode�ļ���ʹ��fgetws�������ٶȽϿ�*/
static int GetAllFileWS (lua_State * L)
{
	/*���ݵ�һ���������ļ��� */
	const char *filename = lua_tostring (L, -1);
	/*��ȡ�ļ���С */
	FILE *input = fopen (filename, "rb");
	int len = _GetFileSize (filename);
	/*��ʼ������ */
	wchar_t *all;
	all = (wchar_t *) malloc (sizeof (wchar_t) * (len / CHAR_SCALE + 1));
	memset (all, 0, sizeof (wchar_t) * (len / CHAR_SCALE + 1));

	int i = 0;
	wchar_t *buf;

	buf = (wchar_t *) malloc (sizeof (wchar_t) * BUFF_SIZE);
	memset (buf, 0, sizeof (wchar_t) * BUFF_SIZE);
	while (!feof (input))
	{
		int j = 0;

		fgetws (buf, BUFF_SIZE, input);
		/*���ӻ���ռ䣬ԭ��ʹ��wcscat���ӣ����Ǻ����������⣬ʹ��ָ����ٶȺܿ� */
		while (buf[j] != L'\0')
		{
			all[i++] = buf[j++];
		}
		/*�����ʱbuf */
		memset (buf, 0, sizeof (wchar_t) * BUFF_SIZE);
	}
	all[len / CHAR_SCALE] = L'\0';
	lua_pushlstring (L, (char *) all, wcslen (all) * CHAR_SCALE);
	/*ɾ����ʱ���� */
	free (buf);
	free (all);
	return 1;
}

//��ʼ���ַ���ָ��
char * StringInit ()
{
	char *str = (char *) malloc (1 * sizeof (char));
	memset (str, 0, 1 * sizeof (char));
	return str;
}

//�����ַ���Ϊָ���ַ�
char * StringSet (char *str, const char *toset)
{
	int len = strlen (toset) + 1;
	str = (char *) realloc (str, len * sizeof (char));
	strcpy (str, toset);
	return str;
}

//���ַ���ĩβ���ָ���ַ���
char * StringAppent (char *first, const char *last)
{
	int len = strlen (first) + strlen (last) + 1;

	first = (char *) realloc (first, len * sizeof (char));
	strcat (first, last);
	return first;
}

//���ַ���ָ��λ�����ָ���ַ���
char * StringInsert (char *str, const char *insert, int start)
{
	//�Կ�ʼ��������ֽ�����֤
	if (start > strlen (str))
	{
		start = strlen (str);
	}
	else if (start < 0)
	{
		start = 0;
	}
	//���·�������
	int pos = strlen (str);
	int len = strlen (str) + strlen (insert) + 1;
	str = (char *) realloc (str, len * sizeof (char));

	//ʹ��ָ��ķ�ʽ���б���
	int i;
	//ͨ������ʽ��׼���������ݺ�λ�õ����ݸ��Ƶ����
	for (i = 1; i <= pos - start; i++)
	{
		str[len - i - 1] = str[len - i - strlen (insert) - 1];
	}
	//��׼����������ݲ��뵽��ʼ����ĵط�
	for (i = 1; i <= strlen (insert); i++)
	{
		str[i + start - 1] = insert[i - 1];
	}
	//ĩβ��0
	str[len - 1] = '\0';
	return str;
}


static int RunCmd (lua_State * L)
{
    const char * cmd;
    const char * workdir;
    //���ݶ�ջ���ݣ���������
    int top = lua_gettop(L);
    if (top==1) {
        workdir=NULL;
        cmd=lua_tostring (L,-1);
    }
    else if (top==2){
        workdir=lua_tostring (L,-1);
        cmd=lua_tostring (L,-2);
    }
    
    SECURITY_ATTRIBUTES sa;
    HANDLE hRead,hWrite;

    sa.nLength=sizeof (SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor=NULL;
    sa.bInheritHandle=TRUE;
    if (!CreatePipe (&hRead,&hWrite,&sa,0))
    {
        printf ("error");
        return;
    }
    
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    si.cb=sizeof (STARTUPINFO);
    GetStartupInfo (&si);
    si.hStdError=hWrite;
    si.hStdOutput=hWrite;
    si.wShowWindow=SW_HIDE;
    si.dwFlags=STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

    /* Quick-and-dirty��ʵ��
       char * runcmd;
       int len = strlen("c://windows//system32//cmd.exe /c ") + strlen(cmd) + 1;
       runcmd = (char*)malloc(len*sizeof(char));
       memset(runcmd,0,len*sizeof(char));
       sprintf(runcmd,"c://windows//system32//cmd.exe /c %s",cmd);
     */
    char * runcmd=StringInit ();

    runcmd=StringSet (runcmd,cmd);
    runcmd=StringInsert (runcmd,"c://windows//system32//cmd.exe /c ",0);

    //printf("%s\n",runcmd);
    if (!CreateProcess
        (NULL,runcmd,NULL,NULL,TRUE,0,NULL,workdir,&si,&pi))
    {
        printf ("error");
        return;
    }
    CloseHandle (hWrite);

    char buffer[128]={0};
    char * result=StringInit ();

    DWORD bytesRead;

    while (1)
    {
        if (ReadFile (hRead,buffer,127,&bytesRead,NULL) ==FALSE)
            break;
        result=StringAppent (result,buffer);
        memset (buffer,0,127);
        //sleep(1);
    }
    //���ؽ��
    lua_pushstring (L,result);
    free (result);
    free (runcmd);
    return 1;
}

static int OpenDialog(lua_State * L)
{
    /*���ݵ�һ��������cmd */
	const char *filter = lua_tostring (L, -1);

    OPENFILENAME ofn;
    TCHAR szOpenFileNames[80*MAX_PATH];
    TCHAR szPath[MAX_PATH];
    TCHAR szFileName[80*MAX_PATH];
    TCHAR* p;
    int nLen = 0;

    ZeroMemory( &ofn, sizeof(ofn) );
    ofn.Flags = OFN_EXPLORER | OFN_ALLOWMULTISELECT;
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szOpenFileNames;
    ofn.nMaxFile = sizeof(szOpenFileNames);
    ofn.lpstrFile[0] = '\0';
    ofn.lpstrFilter = filter;

    if( GetOpenFileName( &ofn ) )
    {  
      //�ѵ�һ���ļ���ǰ�ĸ��Ƶ�szPath,��:
      //���ֻѡ��һ���ļ�,�͸��Ƶ����һ��'\'
      //���ѡ�˶���ļ�,�͸��Ƶ���һ��NULL�ַ�
      lstrcpyn(szPath, szOpenFileNames, ofn.nFileOffset );
      //��ֻѡ��һ���ļ�ʱ,�������NULL�ַ��Ǳ����.
      //���ﲻ����Դ�ѡ��һ���Ͷ���ļ������
      szPath[ ofn.nFileOffset ] = '\0';
      nLen = lstrlen(szPath);
      
      if( szPath[nLen-1] != '\\' )   //���ѡ�˶���ļ�,��������'\\'
      {
        lstrcat(szPath, TEXT("\\"));
      }
      
      p = szOpenFileNames + ofn.nFileOffset; //��ָ���Ƶ���һ���ļ�
      
      ZeroMemory(szFileName, sizeof(szFileName));
      while( *p )
      {   
        lstrcat(szFileName, szPath);  //���ļ�������·��  
        lstrcat(szFileName, p);    //�����ļ���  
        lstrcat(szFileName, TEXT("\n")); //����   
        p += lstrlen(p) +1;     //������һ���ļ�
      }
      lua_pushstring (L, szFileName);
    }
    return 1;
}

static const luaL_reg PearFunctions[] = {
	{"a2u", Unicode_a2u},
	{"u2a", Unicode_u2a},
	{"u2u8", Unicode_u2u8},
	{"u82u", Unicode_u82u},
	{"a2u8", Unicode_a2u8},
	{"u82a", Unicode_u82a},
	{"getfilesizew", GetFileSizeW},
	{"getallfilewc", GetAllFileWC},
	{"getallfilews", GetAllFileWS},
	{"runcmd", RunCmd},
    {"opendialog",OpenDialog},
	{NULL, NULL}
};

int __cdecl __declspec (dllexport) luaopen_Pear (lua_State * L)
{
	luaL_openlib (L, "Pear", PearFunctions, 0);
	return 1;
}


/*
gcc Pear.c -I"C:\MinGW\include\lua" -L"C:\MinGW\lib" -lmingw32 -llua -lcomdlg32 -shared -o C:\Pear.dll
*/