#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include <windows.h>

#define BUFF_SIZE 1024
#define CHAR_SCALE (sizeof(wchar_t)/sizeof(char))

wchar_t * AToU( const char* str )
{
	int      textlen ;
	wchar_t * result;
	textlen = MultiByteToWideChar( CP_ACP, 0, str,-1,      NULL,0 );
	result = (wchar_t *)malloc((textlen+1)*sizeof(wchar_t));
	memset(result,0,(textlen+1)*sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0,str,-1,(LPWSTR)result,textlen );
	return      result;
}
char * UToA( const wchar_t *str )
{
	char * result;
	int textlen;
	// wide char to multi char
	textlen = WideCharToMultiByte( CP_ACP,      0,      str,      -1,      NULL, 0, NULL, NULL   );
	result =(char *)malloc((textlen+1)*sizeof(char));
	memset( result, 0, sizeof(char) * ( textlen + 1 ) );
	WideCharToMultiByte( CP_ACP, 0, str, -1, result, textlen, NULL, NULL );
	return result;
}
wchar_t * U8ToU( const char* str )
{
	int      textlen ;
	wchar_t * result;
	textlen = MultiByteToWideChar( CP_UTF8, 0, str,-1,      NULL,0 );
	result = (wchar_t *)malloc((textlen+1)*sizeof(wchar_t));
	memset(result,0,(textlen+1)*sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0,str,-1,(LPWSTR)result,textlen );
	return      result;
}
char * UToU8( const wchar_t *str )
{
	char * result;
	int textlen;
	// wide char to multi char
	textlen = WideCharToMultiByte( CP_UTF8,      0,      str,      -1,      NULL, 0, NULL, NULL   );
	result =(char *)malloc((textlen+1)*sizeof(char));
	memset(result, 0, sizeof(char) * ( textlen + 1 ) );
	WideCharToMultiByte( CP_UTF8, 0, str, -1, result, textlen, NULL, NULL );
	return result;
}

static int Unicode_a2u(lua_State *L)
{
	const char* str;
	wchar_t* result;
	/*传递第一个参数*/
	str = lua_tostring(L, -1);
	/*开始转换*/
	result = AToU(str);
	/*返回值，*/
	lua_pushlstring(L, (char*)result,wcslen(result)*CHAR_SCALE);
	return 1;
}

static int Unicode_u2a(lua_State *L)
{
	const wchar_t* str;
	char* result;
	/*传递第一个参数*/
	str = (wchar_t *)lua_tostring(L, -1);
	/*开始转换*/
	result = UToA(str);
	/*返回值，*/
	lua_pushstring(L, result);
	return 1;
}

static int Unicode_u2u8(lua_State *L)
{
	const wchar_t* str;
	char* result;
	/*传递第一个参数*/
	str = (wchar_t*)lua_tostring(L, -1);
	/*开始转换*/
	result = UToU8(str);
	/*返回值，*/
	lua_pushstring(L, result);
	return 1;
}

static int Unicode_u82u(lua_State *L)
{
	const char* str;
	wchar_t * result;
	/*传递第一个参数*/
	str = lua_tostring(L, -1);
	/*开始转换*/
	result = U8ToU(str);
	/*返回值，*/
	lua_pushlstring(L, (char*)result,wcslen(result)*CHAR_SCALE);
	return 1;
}

static int Unicode_a2u8(lua_State *L)
{
	const char* str;
	wchar_t * temp;
	char* result;
	/*传递第一个参数*/
	str = lua_tostring(L, -1);
	/*开始转换*/
	temp = AToU(str);
	result = UToU8(temp);
	/*返回值，*/
	lua_pushstring(L, result);
	return 1;
}

static int Unicode_u82a(lua_State *L)
{
	const char* str;
	wchar_t * temp;
	char* result;
	/*传递第一个参数*/
	str = lua_tostring(L, -1);
	/*开始转换*/
	temp = U8ToU(str);
	result = UToA(temp);
	/*返回值，*/
	lua_pushstring(L, result);
	return 1;
}
/*获取一个文件大小*/
static int _GetFileSize(const char* filename)
{
	long len;
	FILE * fp;
	/*用只读打开文件并seek到文件末尾的方式获取文件大小*/
	if ((fp = fopen(filename, "r")) == NULL)
	{
		printf("%s is not invalid\n",filename);
		return 0;
	}
	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	fclose(fp);
	return len;
}
/*Lua 获取 文件大小*/
static int GetFileSizeW(lua_State *L)
{
	/*传递第一个参数，文件名*/
	const char* filename = lua_tostring(L, -1);
	lua_pushinteger(L, _GetFileSize(filename));
	return 1;
}

/*读取一个Unicode文件，使用fgetwc函数，IO导致速度较慢*/
static int GetAllFileWC(lua_State *L)
{
	/*传递第一个参数，文件名*/
	const char* filename = lua_tostring(L, -1);
	/*获取文件大小*/
	int len;
	len = _GetFileSize(filename);
	/*设置缓存大小*/
	wchar_t * buf;
	/*由于wchar_t长度为char长度一倍，所以buf空间大小为文件长度一半，再加末尾的 '\0'*/
	buf = (wchar_t *) malloc (sizeof(wchar_t) * (len/CHAR_SCALE+1));

	int i = 0 ;
	FILE*     input=fopen( filename, "rb");
	while(!feof(input))
	{
		buf[i++]=fgetwc(input);
	}
	/*字符串末尾置零*/
	buf[i-1]=L'\0';
	lua_pushlstring(L, (char*)buf,wcslen(buf)*CHAR_SCALE);
	return 1;
}
/*读取一个Unicode文件，使用fgetws函数，速度较快*/
static int GetAllFileWS(lua_State *L)
{
	/*传递第一个参数，文件名*/
	const char* filename = lua_tostring(L, -1);
	/*获取文件大小*/
	FILE*    input=fopen( filename, "rb");
	int len = _GetFileSize(filename);
	/*初始化变量*/
	wchar_t *all ;
	all = (wchar_t *)malloc(sizeof(wchar_t)*(len/CHAR_SCALE+1));
	memset(all, 0, sizeof(wchar_t) * (len/CHAR_SCALE+1) );

	int i=0;
	while(!feof(input))
	{
		wchar_t *   buf;
		buf = (wchar_t *)malloc(sizeof(wchar_t)*BUFF_SIZE)         ;
		memset(buf,   0, sizeof(wchar_t) * BUFF_SIZE );

		int j=0;
		fgetws(buf,   BUFF_SIZE,   input);
		/*连接缓存空间，原先使用wcscat连接，但是很慢且有问题，使用指针后速度很快*/
		while(buf[j] != L'\0')
		{
			all[i++]=buf[j++];
		}
		free(buf);
	}
	all[len/CHAR_SCALE]=L'\0';
	lua_pushlstring(L, (char*)all,wcslen(all)*CHAR_SCALE);
	return 1;
}

static const luaL_reg UnicodeFunctions [] =
{
	{"a2u",Unicode_a2u},
	{"u2a",Unicode_u2a},
	{"u2u8",Unicode_u2u8},
	{"u82u",Unicode_u82u},
	{"a2u8",Unicode_a2u8},
	{"u82a",Unicode_u82a},
	{"getfilesizew",GetFileSizeW},
	{"getallfilewc",GetAllFileWC},
	{"getallfilews",GetAllFileWS},
	{NULL, NULL}
};

int __cdecl __declspec(dllexport) luaopen_Unicode(lua_State* L)
{
	luaL_openlib(L, "Unicode", UnicodeFunctions, 0);
	return 1;
}
