// re_traits.inl
// 
// ������ʽ������
// 
// ��2004���ڱ���д�ڴ���ӥ�蹫˾
// ��Ȩ����:
// ���߷�������������Ȩ֮����κ�Ȩ�档������ڱ�����������ǰ���£����ɵģ������Ƶ�
// �ַ����޸ĺ�ʹ�ñ������⡣
// ��������κν����BUG�㱨����ӭ����ϵԭʼ����:tearshark@163.com
//
// ԭʼ���� : lanzhengpeng(������)
// Modify time : 2004-6-15 23:34
// ��ʷ: �ο�re_history.txt
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
	rt_root				= 0,		//������
	rt_escape			= '\\',		//ת�����

	rt_begin_line		= '^',		//��Ϊ����ƥ���ʱ��ƥ���еĿ�ʼ
	rt_begin_all		= '^',		//��Ϊ����ƥ���ʱ��ƥ����ʼ��ַ
	rt_end_line			= '$',		//��Ϊ����ƥ���ʱ��ƥ���еĽ�����
									//�еĽ�����'\r\n'��'\n'��Ϊ����������һ��ѡ��������ƥ��
	rt_end_all			= '$',		//��Ϊ����ƥ���ʱ��ƥ�������ַ

	rt_not_range		= '^',		//��Χ֮��
	rt_range_begin		= '[',		//��Χ֮�� ��ʼ
	rt_range_end		= ']',		//��Χ֮�� ����
	rt_range_sign		= '-',		//��Χ

	rt_return			= 'r',		//�س�
	rt_enter			= 'n',		//����
	rt_tab				= 't',		//�Ʊ�

	rt_any				= '.',		//�����ַ�

	rt_not_regular		= '!',		//��������ʽ
	rt_and_regular		= '&',		//��������ʽ
	rt_or_regular		= '|',		//��������ʽ

	rt_more_zero		= '*',		//�ظ��������
	rt_more_one			= '+',		//�ظ�һ������
	rt_zero_one			= '?',		//��λ�һ��

	rt_new_begin		= '(',		//�ֿ� ��ʼ
	rt_new_end			= ')',		//�ֿ� ����

	rt_group_begin		= '{',		//���� ��ʼ
	rt_group_end		= '}',		//���� ����

	rt_count_begin		= '<',		//�ظ������޶� ��ʼ
	rt_count_end		= '>',		//�ظ������޶� ����
	rt_colon			= ':',		//<m:n>

	rt_hex_prefix		= 'x',		//ʮ������ǰ׺
	rt_hex_prefix2		= 'X',		//ʮ������ǰ׺
	rt_oct_prefix		= 'o',		//�˽���ǰ׺
	rt_oct_prefix2		= 'O',		//��������ǰ׺
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