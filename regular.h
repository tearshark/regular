// regular.h
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
// More author :
// Create time : 2004-6-2	9:12
// Modify time : 2004-6-14   15:08
// ��ʷ: �ο�re_history.txt
//-----------------------------------------------------------------------------
#ifndef __regular_h__2004_6_14_14_14
#define __regular_h__2004_6_14_14_14
#include <vector>
#include <algorithm>
#include <cassert>

#pragma once

// traits
namespace VFX
{

struct re_traits_a;
struct re_traits_mb;
struct re_traits_w;

enum re_error;
enum re_flags;

template<class TRAITS>
struct re_match;
template<class TRAITS>
class re_result;
template<class TRAITS>
class re_pattern;

}

#include "re_traits.inl"

namespace VFX
{

// enumerate
enum re_flags
{
	rf_zero				= 0x0000,
	rf_return			= 0x0001,		//\rת��Ϊ'\r'������ת��Ϊr
	rf_multi_line		= 0x0002,		//ȫ����Ϊ����ƥ�䡣^ƥ������ʼ��$��Ϊ�н���
										//������Ϊ����ƥ�䡣��^ƥ����ʼ��ַ��$ƥ�������ַ��.����ƥ��\n
	rf_nocase			= 0x0004,		//�����ִ�Сд
	rf_endasenter		= 0x0008,		//������Ϊ\nƥ��
	rt_optimize			= 0x0010,		//�Ż�������һЩ������ĺϷ��ı��ʽ
};

enum re_error
{
	ree_ok		= 0,		//û�д���
	ree_syntex,				//�﷨����
	ree_noparse,			//û�е���parse�����ʧ��
	ree_nomatch,			//û��ƥ��
	ree_address,			//ָ���ַ�����߼�
	ree_memory,				//�ڴ�����
	ree_interior,			//�ڲ�����
	ree_recursion_always,	//����ݹ�
};

// helper functions to make it easier to combine the regex flags.
inline re_flags operator|( re_flags f1, re_flags f2 )
{
    return ( re_flags ) ( ( unsigned )f1 | ( unsigned )f2 );
}
inline re_flags & operator|=( re_flags & f1, re_flags f2 )
{
    return f1 = ( f1 | f2 );
}
inline re_flags operator&( re_flags f1, re_flags f2 )
{
    return ( re_flags ) ( ( unsigned )f1 & ( unsigned )f2 );
}
inline re_flags & operator&=( re_flags & f1, re_flags f2 )
{
    return f1 = ( f1 & f2 );
}

//-----------------------------------------------------------------------------

// structs and classes

// matched description
template<class TRAITS = re_traits_a >
struct re_match
{
	typedef TRAITS				traits;
	typedef typename traits::re_char		re_char;
	size_t			id;
	const re_char *	begin;
	const re_char * end;

	re_match():id(-1){}
	re_match(size_t _i,const re_char * _b,const re_char * _e)
		:id(_i),begin(_b),end(_e){}
};

// matched result if succeeded
template<class TRAITS = re_traits_a >
class re_result
{
public:

	typedef TRAITS				traits;
	typedef typename traits::re_char		re_char;
	typedef re_match<TRAITS>	match_type;

	match_type	m_Match;
	std::vector<match_type>	m_Groups;
	std::vector<match_type>	m_GroupBegin;

	re_result(){
		m_Match.begin = m_Match.end = NULL;
	}

	const match_type Match() const{
		return m_Match;
	}
	size_t GroupCount() const{
		return (size_t)m_Groups.size();
	}
	const match_type GetGroup(size_t __n) const{
		return m_Groups[__n];
	}
	const match_type operator[] (size_t __n) const{
		return m_Groups[__n];
	}

	friend class re_pattern<TRAITS>;
private:
	void _repair(intptr_t offset){
		m_Match.begin = reinterpret_cast<const re_char *>(reinterpret_cast<const char *>(m_Match.begin) + offset);
		m_Match.end = reinterpret_cast<const re_char *>(reinterpret_cast<const char *>(m_Match.end) + offset);
		
		for(typename std::vector<match_type>::iterator i=m_Groups.begin(); i!=m_Groups.end(); ++i)
		{
			(*i).begin = reinterpret_cast<const re_char *>(reinterpret_cast<const char *>((*i).begin) + offset);
			(*i).end = reinterpret_cast<const re_char *>(reinterpret_cast<const char *>((*i).end) + offset);
		}
	}
};

// pattern for parser and matcher
template<class TRAITS = re_traits_a >
class re_pattern
{
public:

	typedef TRAITS				traits;
	typedef typename traits::re_char		re_char;
	typedef re_match<traits>	match_type;
	typedef re_result<traits>	result_type;
	typedef re_pattern<traits>	this_type;

	re_pattern()
	{
		m_Syntex = NULL;
	}
	~re_pattern()
	{
		clear();
	}

#if 1
#include "re_detail_vc6.inl"
#else
	// interface

	re_error	parse(const re_char * szStart,const re_char * szEnd = NULL,re_flags flags = rf_zero);
	re_error	match(const re_char * szStart,result_type & result,const re_char * szEnd = NULL) const;
	template<class STRING>
	re_error	replace(const re_char * szFind,const re_char * szReplaceWith,const re_char * pszReplace,STRING & strResult,re_flags flags = rf_zero);
	void		clear();

#endif

	re_error	error() const{
		return m_eError;}
	re_flags	option() const{
		return m_eFlags;}

private:
#include "re_syntex.inl"

private:
	re_pattern(const this_type & );
	re_pattern & operator = (const this_type &);
};

}

#if defined(_UNICODE) || defined(UNICODE)
typedef VFX::re_traits_w					re_traits;
#else
typedef VFX::re_traits_mb					re_traits;
#endif
typedef VFX::re_match<re_traits>			re_match;
typedef VFX::re_result<re_traits>			re_result;
typedef VFX::re_pattern<re_traits>			re_pattern;

typedef VFX::re_match<VFX::re_traits_a>		re_match_a;
typedef VFX::re_result<VFX::re_traits_a>	re_result_a;
typedef VFX::re_pattern<VFX::re_traits_a>	re_pattern_a;

typedef VFX::re_match<VFX::re_traits_mb>	re_match_mb;
typedef VFX::re_result<VFX::re_traits_mb>	re_result_mb;
typedef VFX::re_pattern<VFX::re_traits_mb>	re_pattern_mb;

typedef VFX::re_match<VFX::re_traits_w>		re_match_w;
typedef VFX::re_result<VFX::re_traits_w>	re_result_w;
typedef VFX::re_pattern<VFX::re_traits_w>	re_pattern_w;

#endif // end __regular_h__2004_6_14_14_14
