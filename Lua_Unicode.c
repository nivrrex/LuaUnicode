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

	/*传递第一个参数 */
	str = lua_tostring (L, -1);
	/*开始转换 */
	result = AToU (str);
	/*返回值， */
	lua_pushlstring (L, (char *) result, wcslen (result) * CHAR_SCALE);
    free(result);
	return 1;
}

static int Unicode_u2a (lua_State * L)
{
	const wchar_t *str;
	char *result;

	/*传递第一个参数 */
	str = (wchar_t *) lua_tostring (L, -1);
	/*开始转换 */
	result = UToA (str);
	/*返回值， */
	lua_pushstring (L, result);
    free(result);
	return 1;
}

static int Unicode_u2u8 (lua_State * L)
{
	const wchar_t *str;
	char *result;

	/*传递第一个参数 */
	str = (wchar_t *) lua_tostring (L, -1);
	/*开始转换 */
	result = UToU8 (str);
	/*返回值， */
	lua_pushstring (L, result);
    free(result);
	return 1;
}

static int Unicode_u82u (lua_State * L)
{
	const char *str;
	wchar_t *result;

	/*传递第一个参数 */
	str = lua_tostring (L, -1);
	/*开始转换 */
	result = U8ToU (str);
	/*返回值， */
	lua_pushlstring (L, (char *) result, wcslen (result) * CHAR_SCALE);
    free(result);
	return 1;
}

static int Unicode_a2u8 (lua_State * L)
{
	const char *str;
	wchar_t *temp;
	char *result;

	/*传递第一个参数 */
	str = lua_tostring (L, -1);
	/*开始转换 */
	temp = AToU (str);
	result = UToU8 (temp);
	/*返回值， */
	lua_pushstring (L, result);
    free(result);
	return 1;
}

static int Unicode_u82a (lua_State * L)
{
	const char *str;
	wchar_t *temp;
	char *result;

	/*传递第一个参数 */
	str = lua_tostring (L, -1);
	/*开始转换 */
	temp = U8ToU (str);
	result = UToA (temp);
	/*返回值， */
	lua_pushstring (L, result);
    free(result);
	return 1;
}

/*获取一个文件大小*/
static int _GetFileSize (const char *filename)
{
	long len;
	FILE *fp;

	/*用只读打开文件并seek到文件末尾的方式获取文件大小 */
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

/*Lua 获取 文件大小*/
static int GetFileSizeW (lua_State * L)
{
	/*传递第一个参数，文件名 */
	const char *filename = lua_tostring (L, -1);
	lua_pushinteger (L, _GetFileSize (filename));
	return 1;
}

/*读取一个Unicode文件，使用fgetwc函数，IO导致速度较慢*/
static int GetAllFileWC (lua_State * L)
{
	/*传递第一个参数，文件名 */
	const char *filename = lua_tostring (L, -1);
	/*获取文件大小 */
	int len;
	len = _GetFileSize (filename);
	/*设置缓存大小 */
	wchar_t *buf;
	/*由于wchar_t长度为char长度一倍，所以buf空间大小为文件长度一半，再加末尾的 '\0' */
	buf = (wchar_t *) malloc (sizeof (wchar_t) * (len / CHAR_SCALE + 1));

	int i = 0;
	FILE *input = fopen (filename, "rb");
	while (!feof (input))
	{
		buf[i++] = fgetwc (input);
	}
	/*字符串末尾置零 */
	buf[i - 1] = L'\0';
	lua_pushlstring (L, (char *) buf, wcslen (buf) * CHAR_SCALE);
	free (buf);
	return 1;
}

/*读取一个Unicode文件，使用fgetws函数，速度较快*/
static int GetAllFileWS (lua_State * L)
{
	/*传递第一个参数，文件名 */
	const char *filename = lua_tostring (L, -1);
	/*获取文件大小 */
	FILE *input = fopen (filename, "rb");
	int len = _GetFileSize (filename);
	/*初始化变量 */
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
		/*连接缓存空间，原先使用wcscat连接，但是很慢且有问题，使用指针后速度很快 */
		while (buf[j] != L'\0')
		{
			all[i++] = buf[j++];
		}
		/*清空临时buf */
		memset (buf, 0, sizeof (wchar_t) * BUFF_SIZE);
	}
	all[len / CHAR_SCALE] = L'\0';
	lua_pushlstring (L, (char *) all, wcslen (all) * CHAR_SCALE);
	/*删除临时变量 */
	free (buf);
	free (all);
	return 1;
}

//初始化字符串指针
char * StringInit ()
{
	char *str = (char *) malloc (1 * sizeof (char));
	memset (str, 0, 1 * sizeof (char));
	return str;
}

//设置字符串为指定字符
char * StringSet (char *str, const char *toset)
{
	int len = strlen (toset) + 1;
	str = (char *) realloc (str, len * sizeof (char));
	strcpy (str, toset);
	return str;
}

//在字符串末尾添加指定字符串
char * StringAppent (char *first, const char *last)
{
	int len = strlen (first) + strlen (last) + 1;

	first = (char *) realloc (first, len * sizeof (char));
	strcat (first, last);
	return first;
}

//在字符串指定位置添加指定字符串
char * StringInsert (char *str, const char *insert, int start)
{
	//对开始插入的数字进行验证
	if (start > strlen (str))
	{
		start = strlen (str);
	}
	else if (start < 0)
	{
		start = 0;
	}
	//重新分配内容
	int pos = strlen (str);
	int len = strlen (str) + strlen (insert) + 1;
	str = (char *) realloc (str, len * sizeof (char));

	//使用指针的方式进行遍历
	int i;
	//通过倒序方式将准备插入数据后位置的数据复制到最后
	for (i = 1; i <= pos - start; i++)
	{
		str[len - i - 1] = str[len - i - strlen (insert) - 1];
	}
	//将准备插入的数据插入到开始插入的地方
	for (i = 1; i <= strlen (insert); i++)
	{
		str[i + start - 1] = insert[i - 1];
	}
	//末尾置0
	str[len - 1] = '\0';
	return str;
}


static int RunCmd (lua_State * L)
{
    const char * cmd;
    const char * workdir;
    //根据堆栈内容，决定工作
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

    /* Quick-and-dirty的实现
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
    //返回结果
    lua_pushstring (L,result);
    free (result);
    free (runcmd);
    return 1;
}

static int OpenDialog(lua_State * L)
{
    /*传递第一个参数，cmd */
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
      //把第一个文件名前的复制到szPath,即:
      //如果只选了一个文件,就复制到最后一个'\'
      //如果选了多个文件,就复制到第一个NULL字符
      lstrcpyn(szPath, szOpenFileNames, ofn.nFileOffset );
      //当只选了一个文件时,下面这个NULL字符是必需的.
      //这里不区别对待选了一个和多个文件的情况
      szPath[ ofn.nFileOffset ] = '\0';
      nLen = lstrlen(szPath);
      
      if( szPath[nLen-1] != '\\' )   //如果选了多个文件,则必须加上'\\'
      {
        lstrcat(szPath, TEXT("\\"));
      }
      
      p = szOpenFileNames + ofn.nFileOffset; //把指针移到第一个文件
      
      ZeroMemory(szFileName, sizeof(szFileName));
      while( *p )
      {   
        lstrcat(szFileName, szPath);  //给文件名加上路径  
        lstrcat(szFileName, p);    //加上文件名  
        lstrcat(szFileName, TEXT("\n")); //换行   
        p += lstrlen(p) +1;     //移至下一个文件
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