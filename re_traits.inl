// re_traits.inl
// 
// 正则表达式解析库
// 
// 与2004年在北京写于创意鹰翔公司
// 版权申明:
// 作者放弃除著作署名权之外的任何权益。你可以在保留此申明的前提下，自由的，无限制的
// 分发、修改和使用本函数库。
// 如果你有任何建议和BUG汇报，欢迎你联系原始作者:tearshark@163.com
//
// 原始作者 : lanzhengpeng(兰征鹏)
// Modify time : 2004-6-15 23:34
// 历史: 参看re_history.txt
//-----------------------------------------------------------------------------

#ifndef isdbcsleader
#	include <windows.h>
#	define isdbcsleader(c)	IsDBCSLeadByte(c)
#endif
#include <tchar.h>
#include <mbstring.h>

namespace VFX
{

enum re_types
{
	rt_root				= 0,		//跟块标记
	rt_escape			= '\\',		//转义符号

	rt_begin_line		= '^',		//作为多行匹配的时候，匹配行的开始
	rt_begin_all		= '^',		//作为单行匹配的时候，匹配起始地址
	rt_end_line			= '$',		//作为多行匹配的时候，匹配行的结束。
									//行的结束以'\r\n'或'\n'作为结束。根据一个选项决定如何匹配
	rt_end_all			= '$',		//作为单行匹配的时候，匹配结束地址

	rt_not_range		= '^',		//范围之外
	rt_range_begin		= '[',		//范围之内 起始
	rt_range_end		= ']',		//范围之内 结束
	rt_range_sign		= '-',		//范围

	rt_return			= 'r',		//回车
	rt_enter			= 'n',		//换行
	rt_tab				= 't',		//制表

	rt_any				= '.',		//任意字符

	rt_not_regular		= '!',		//否定正则表达式
	rt_and_regular		= '&',		//与正则表达式
	rt_or_regular		= '|',		//或正则表达式

	rt_more_zero		= '*',		//重复零次以上
	rt_more_one			= '+',		//重复一次以上
	rt_zero_one			= '?',		//零次或一次

	rt_new_begin		= '(',		//分块 开始
	rt_new_end			= ')',		//分块 结束

	rt_group_begin		= '{',		//成组 开始
	rt_group_end		= '}',		//成组 结束

	rt_count_begin		= '<',		//重复次数限定 开始
	rt_count_end		= '>',		//重复次数限定 结束
	rt_colon			= ':',		//<m:n>

	rt_hex_prefix		= 'x',		//十六进制前缀
	rt_hex_prefix2		= 'X',		//十六进制前缀
	rt_oct_prefix		= 'o',		//八进制前缀
	rt_oct_prefix2		= 'O',		//八六进制前缀
};

struct re_traits_a
{
	typedef char re_char;

	static inline const re_char * Next(const re_char * sz){
		return sz + 1;
	}
	static inline re_char * Next(re_char * sz){
		return sz + 1;
	}
	static inline int Cmp(const re_char *szLeft, const re_char *szRight, size_t nByteCount){
		return strncmp(szLeft, szRight, nByteCount);
	}
	static inline int CmpNoCase(const re_char *szLeft, const re_char *szRight, size_t nByteCount){
		return _strnicmp(szLeft, szRight, nByteCount);
	}
	static inline void ToLower(const re_char *sz, const re_char * szEnd,re_char * szOut){
		for(;sz < szEnd; sz = Next(sz),szOut = Next(szOut))
			*szOut = tolower(*sz);
	}
	static inline int IsDigit(const re_char * sz){
		return isdigit(*sz);
	}
	static inline int toi(const re_char* szStart) {
		return atoi(szStart);
	}
	static inline long to_int(const re_char *szStart, const re_char * szEnd, re_char **szOut){
		long l = 0;
		for(;szStart < szEnd; szStart = Next(szStart))
		{
			if(IsDigit(szStart))
				l = 10 * l + *szStart - '0';
			else
				break;
		}
		*szOut = const_cast<re_char *>(szStart);
		return l;
	}
	static inline long to_hex(const re_char *szStart, const re_char * szEnd, re_char **szOut){
		long l = 0;
		for(;szStart < szEnd; szStart = Next(szStart))
		{
			if('0' <= *szStart && *szStart <= '9')
			{
				l = 16 * l + *szStart - '0';
			}
			else if('a' <= *szStart && *szStart <= 'f')
			{
				l = 16 * l + *szStart - 'a' + 10;
			}
			else if('A' <= *szStart && *szStart <= 'F')
			{
				l = 16 * l + *szStart - 'A' + 10;
			}
			else
				break;
		}
		*szOut = const_cast<re_char *>(szStart);
		return l;
	}
	static inline long to_oct(const re_char *szStart, const re_char * szEnd, re_char **szOut){
		long l = 0;
		for(;szStart < szEnd; szStart = Next(szStart))
		{
			if('0' <= *szStart && *szStart <= '7')
				l = 8 * l + *szStart - '0';
			else
				break;
		}
		*szOut = const_cast<re_char *>(szStart);
		return l;
	}
	static inline const re_char** Abbrevs()
	{
		static const re_char * s_szAbbrevs[] = 
		{
			"a[a-zA-Z0-9]",							// alpha numeric
			"b[ \\t]",								// white space (blank)
			"c[a-zA-Z]",							// alpha
			"d[0-9]",								// digit
			"f[+\\-]?([0-9]+\\.[0-9]*)|([0-9]*\\.[0-9]+)",	// Float
			"h[0-9a-fA-F]",							// hex digit
			"n\r?\n",								// newline
			"o/\\*.*?\\*/",							// c cOmment
			"p//.*?\\n|$",							// cPp comment
			"q(\"[^\"]*\")|(\'[^\']*\')",			// quoted string
			"s\"([^\\\\\"]*(\\\\.)*)*?\"",			// c string
			"w[a-zA-Z]+",							// simple word
			"z[0-9]+",								// integer
			NULL
		};

		return s_szAbbrevs;
	}
	static inline const re_char * ReplaceRegular()
	{
		return "({(\\\\\\z)!*}{\\\\\\z}*)*";
	}
	static inline size_t ByteLen(const re_char *sz){
		return int(strlen(sz));
	}
	static inline unsigned char ToUnsigned(re_char c){
		return (unsigned char)c;
	}
	static inline size_t ToSize_t(const re_char * sz){
		return (unsigned char)*sz;
	}
};

struct re_traits_mb
{
	typedef unsigned char		re_char;
	typedef re_traits_a			base;
	
	static inline const re_char * Next(const re_char * sz){
		return _mbsinc(sz);
	}
	static inline re_char * Next(re_char * sz){
		return _mbsinc(sz);
	}
	static inline int Cmp(const re_char *szLeft, const re_char *szRight, size_t nByteCount){
		return _mbsncmp(szLeft, szRight, nByteCount);
	}
	static inline int CmpNoCase(const re_char *szLeft, const re_char *szRight, size_t nByteCount){
		return _mbsnicmp(szLeft, szRight, nByteCount);
	}
	static inline void ToLower(const re_char *sz, const re_char * szEnd,re_char * szOut){
		base::ToLower(reinterpret_cast<const base::re_char*>(sz),
					reinterpret_cast<const base::re_char *>(szEnd),
					reinterpret_cast<base::re_char *>(szOut));
	}
	static inline int IsDigit(const re_char * sz){
		return _ismbcdigit((unsigned int)*sz);
	}
	static inline int toi(const re_char* szStart) {
		return atoi((const char *)szStart);
	}
	static inline long to_int(const re_char *szStart, const re_char * szEnd, re_char **szOut){
		return base::to_int(	reinterpret_cast<const base::re_char *>(szStart),
							reinterpret_cast<const base::re_char *>(szEnd),
							reinterpret_cast<base::re_char **>(szOut));
	}
	static inline long to_hex(const re_char *szStart, const re_char * szEnd, re_char **szOut){
		return base::to_hex(	reinterpret_cast<const base::re_char *>(szStart),
							reinterpret_cast<const base::re_char *>(szEnd),
							reinterpret_cast<base::re_char **>(szOut));
	}
	static inline long to_oct(const re_char *szStart, const re_char * szEnd, re_char **szOut){
		return base::to_oct(	reinterpret_cast<const base::re_char *>(szStart),
							reinterpret_cast<const base::re_char *>(szEnd),
							reinterpret_cast<base::re_char **>(szOut));
	}
	static inline const re_char** Abbrevs(){
		return reinterpret_cast<const re_char**>(base::Abbrevs());
	}
	static inline const re_char * ReplaceRegular(){
		return reinterpret_cast<const re_char *>(base::ReplaceRegular());
	}
	static inline size_t ByteLen(const re_char *sz){
		return base::ByteLen(reinterpret_cast<const base::re_char*>(sz));
	}
	static inline unsigned char ToUnsigned(re_char c){
		return (unsigned char)c;
	}
	static inline size_t ToSize_t(const re_char * sz){
		if(isdbcsleader(*sz))
			return *(unsigned short*)sz;
		return *(unsigned char *)sz;
	}
};


struct re_traits_w
{
	typedef wchar_t				re_char;
	
	static inline const re_char * Next(const re_char * sz){
		return sz + 1;
	}
	static inline re_char * Next(re_char * sz){
		return sz + 1;
	}
	static inline int Cmp(const re_char *szLeft, const re_char *szRight, size_t nByteCount){
		return wcsncmp(szLeft, szRight, nByteCount);
	}
	static inline int CmpNoCase(const re_char *szLeft, const re_char *szRight, size_t nByteCount){
		return _wcsnicmp(szLeft, szRight, nByteCount);
	}
	static inline void ToLower(const re_char *sz, const re_char * szEnd,re_char * szOut){
		for(;sz < szEnd; sz = Next(sz),szOut = Next(szOut))
			*szOut = towlower(*sz);
	}
	static inline int IsDigit(const re_char * sz){
		return iswdigit((unsigned int)*sz);
	}
	static inline int toi(const re_char* szStart) {
		return _wtoi(szStart);
	}
	static inline long to_int(const re_char *szStart, const re_char * szEnd, re_char **szOut){
		long l = 0;
		for(;szStart < szEnd; szStart = Next(szStart))
		{
			if(IsDigit(szStart))
				l = 10 * l + *szStart - L'0';
			else
				break;
		}
		*szOut = const_cast<re_char *>(szStart);
		return l;
	}
	static inline long to_hex(const re_char *szStart, const re_char * szEnd, re_char **szOut){
		long l = 0;
		for(;szStart < szEnd; szStart = Next(szStart))
		{
			if(L'0' <= *szStart && *szStart <= L'9')
			{
				l = 16 * l + *szStart - L'0';
			}
			else if(L'a' <= *szStart && *szStart <= L'f')
			{
				l = 16 * l + *szStart - L'a' + 10;
			}
			else if(L'A' <= *szStart && *szStart <= L'F')
			{
				l = 16 * l + *szStart - L'A' + 10;
			}
			else
				break;
		}
		*szOut = const_cast<re_char *>(szStart);
		return l;
	}
	static inline long to_oct(const re_char *szStart, const re_char * szEnd, re_char **szOut){
		long l = 0;
		for(;szStart < szEnd; szStart = Next(szStart))
		{
			if(L'0' <= *szStart && *szStart <= L'7')
				l = 8 * l + *szStart - L'0';
			else
				break;
		}
		*szOut = const_cast<re_char *>(szStart);
		return l;
	}
	static inline const re_char** Abbrevs(){
		static const re_char * s_szAbbrevs[] = 
		{
			L"a[a-zA-Z0-9]",							// alpha numeric
			L"b[ \\t]",									// white space (blank)
			L"c[a-zA-Z]",								// alpha
			L"d[0-9]",									// digit
			L"f[+\\-]?([0-9]+\\.[0-9]*)|([0-9]*\\.[0-9]+)",	// Float
			L"h[0-9a-fA-F]",							// hex digit
			L"n\r?\n",									// newline
			L"o/\\*.*?\\*/",							// c cOmment
			L"p//.*?\\n|$",								// cPp comment
			L"q(\"[^\"]*\")|(\'[^\']*\')",				// quoted string
			L"s\"([^\\\\\"]*(\\\\.)*[^\\\\\"]*)*?\"",	// c string
			L"w[a-zA-Z]+",								// simple word
			L"z[0-9]+",									// integer
			NULL
		};

		return s_szAbbrevs;
	}
	static inline const re_char * ReplaceRegular()
	{
		return L"({(\\\\\\z)!*}{\\\\\\z}*)*";
	}
	static inline size_t ByteLen(const re_char *sz){
		return sizeof(re_char) * wcslen(sz);
	}
	static inline unsigned char ToUnsigned(re_char c){
		return (unsigned char)c;
	}
	static inline size_t ToSize_t(const re_char * sz){
		return *sz;
	}
};

}