// example.cpp
// 
// 正则表达式解析库演示
// 
// 与2004年在北京写于创意鹰翔公司
// 版权申明:
// 作者放弃除著作署名权之外的任何权益。你可以在保留此申明的前提下，自由的，无限制的
// 分发、修改和使用本函数库。
// 如果你有任何建议和BUG汇报，欢迎你联系原始作者:tearshark@163.com
//
// 原始作者 : lanzhengpeng(兰征鹏)
// Modify time : 2005-03-12   16:58
// 历史: 参看re_history.txt
//-----------------------------------------------------------------------------

#include <iostream>
#include <strstream>
#include <string>

#pragma warning(disable : 4786)

#include "regular.h"

//抽取出C/C++中的字符串
std::string _get_cpp_string_with_comment(const unsigned char * psz)
{
	re_pattern_mb	p;
	p.parse((unsigned char *)"({\\s}([ \\t\\r\\n]*(\\o|\\p)[ \\t\\r\\n]*)*)+",NULL,VFX::rf_return);

	re_result_mb	r;
	p.match((unsigned char *)psz,r);

	std::strstream strio;
	strio << "\"";
	for(size_t i=0; i<r.GroupCount(); ++i)
	{
		re_match_mb mch = r[i];
		if(mch.id == 0)
		{
			for(const unsigned char * sz=mch.begin+1; sz<mch.end-1; ++sz)
				strio << *sz;
		}
	}
	strio << "\"";
	return std::string(strio.str(), (size_t)strio.pcount());
}

template<class CharT>
struct select_re_traits{};
template<> struct select_re_traits<char>{
	typedef VFX::re_traits_a	re_traits;
};
template<> struct select_re_traits<unsigned char>{
	typedef VFX::re_traits_mb	re_traits;
};
template<> struct select_re_traits<wchar_t>{
	typedef VFX::re_traits_w	re_traits;
};

//替换函数
//pszFind:条件表达式。正则表达式
//pszReplaceWith:替换内容表达式。
//				\0表示替换成查找到的内容,
//				\1,\2,...表示替换成条件表达式中第0个，第1个，...组
//				其余部分原封不动的替换
//pszReplace:将要被替换的文本
//strResult:替换的结果文本
//bCase:区分大小写否
template<class CharT>
bool replace(const CharT * pszFind,
			const CharT * pszReplaceWith,
			const CharT * pszReplace,
			std::basic_string<CharT> & strResult,
			bool bCase)
{
	typedef select_re_traits<CharT>::re_traits	re_traits;

	VFX::re_pattern<re_traits>	rpf;
	if(rpf.parse(pszFind,NULL,bCase?VFX::rf_zero:VFX::rf_nocase) != VFX::ree_ok)
		return false;
	VFX::re_result<re_traits> rrw;
	{
		VFX::re_pattern<re_traits>	rpw;
		rpw.parse((CharT *)"({(\\\\\\z)!*}{\\\\\\z}*)*");
		rpw.match(pszReplaceWith,rrw);
	}

	const CharT * pend = pszReplace + strlen((CharT *)pszReplace);
	VFX::re_result<re_traits>	rr;
	for(const CharT * p = pszReplace; p < pend; )
	{
		if(VFX::ree_ok == rpf.match(p,rr,pend))
		{
			//匹配到表达式
			//输出被跳过的字符
			strResult.append((CharT *)p,rr.m_Match.begin - p);
			for(size_t i=0; i<rrw.GroupCount(); ++i)
			{
				VFX::re_match<re_traits> mch = rrw[i];
				if(mch.id == 0)
				{
					strResult.append((CharT *)mch.begin,mch.end - mch.begin);
				}
				else 
				{
					size_t nGroup = _ttoi((CharT *)mch.begin + 1);
					if(nGroup == 0)
					{
						strResult.append((CharT *)rr.m_Match.begin,rr.m_Match.end - rr.m_Match.begin);
					}
					else if(nGroup <= rr.GroupCount())
					{
						mch = rr[nGroup - 1];
						strResult.append((CharT *)mch.begin,mch.end - mch.begin);
					}
				}
			}
			//移动指针
			p = (CharT *)rr.m_Match.end;
		}
		else
		{
			strResult.append((CharT *)p,pend - p);
			//结束
			p = pend;
		}
	}
	
	return true;
}

inline bool __IsCharEq(TCHAR c1,TCHAR c2)
{
	return _totlower(c1) == _totlower(c2);
}

int main()
{
	const char * pszReplace = 
"	m_fmCBI.createdate = sc->createdate;\r\n"
"	m_fmCBI.money = sc->money;\r\n"
"	m_fmCBI.fame = sc->fame;\r\n"
"	m_fmCBI.fans_number = sc->fans_number;\r\n"
"	m_fmCBI.fans_intimate = sc->fans_intimate;\r\n"
"	m_fmCBI.shirt_index = sc->shirt_index;\r\n"
"	m_fmCBI.shirt_host_color1 = sc->shirt_host_color1;\r\n"
"	m_fmCBI.shirt_host_color2 = sc->shirt_host_color2;\r\n"
"	m_fmCBI.shirt_visiting_color1 = sc->shirt_visiting_color1;\r\n"
"	m_fmCBI.shirt_visiting_color2 = sc->shirt_visiting_color2;\r\n"
"	m_fmCBI.flag_index = sc->flag_index;\r\n"
"	m_fmCBI.flag_color1 = sc->flag_color1;\r\n"
"	m_fmCBI.flag_color2 = sc->flag_color2;\r\n"
"	m_fmCBI.logo_index = sc->logo_index;\r\n";

	std::string strResult;
	VFX::re_pattern<> rep;
	rep.parse("{m_fmCBI[^ =]+} *= *{[^;]+}");
	rep.replace(NULL,
			"\\2 = \\1",
			pszReplace,
			strResult,
			VFX::rf_zero);
	std::cout << strResult;

	return 0;
}

//int main()
//{
//
//	std::string str(
//			"\"\\r\"							// return\r\n"
//			"\"\\t\"							// tab\r\n"
//			"\"\\r?\\n\"							// newline\r\n"
//			"\"[a-zA-Z0-9]\"						// alpha numeric\r\n"
//			"\"[ \\t]\"							// white space (blank)\r\n"
//			"\"[a-zA-Z]\"						// alpha\r\n"
//			"\"[0-9]\"							// digit\r\n"
//			"\"[0-9a-fA-F]\"						// hex digit\r\n"
//			"\"(\\\"[^\\\"]*\\\")|(\'[^\']*\')\"				// quoted string\r\n"
//			"/*no implement*/\"test\"					// all right\r\n"
//			"\"[a-zA-Z]+\"						// simple word\r\n"
//			"\"[0-9]+\"						// integer\r\n"
//		);
//	std::cout << str << std::endl;
//	std::cout << _get_cpp_string_with_comment((unsigned char *)str.c_str());
//	std::cout << std::endl;
//
//    return 0;
//}
//
////去掉C++中的注释
//int main3(int argc,char * argv[])
//{
//	//参数要多于一个
//	if(argc < 2)
//		return 0;
//	//分析表达式
//	re_pattern_mb	rp;
//	rp.parse((unsigned char *)"{\\s}|{(\\o|\\p)}");
//	//打开文件
//	std::ifstream	file(argv[1],std::ios::in|std::ios::binary);
//	//得到文件长度
//	file.seekg(0,std::ios::end);
//	size_t nlength = file.tellg();
//	file.seekg(0,std::ios::beg);
//	//根据文件长度分配内存
//	unsigned char * const pbuff = new unsigned char[nlength + 1];
//	//读文件
//	file.read((char *)pbuff,nlength);
//	pbuff[nlength] = 0;
//
//	//循环去掉注释
//	unsigned char * pend = pbuff + nlength;
//	re_result_mb	rr;
//	for(unsigned char * p = pbuff; p < pend; )
//	{
//		if(VFX::ree_ok == rp.match(p,rr,pend))
//		{
//			//匹配到表达式
//			//输出被跳过的字符
//			std::cout.write((char *)p,rr.m_Match.begin - p);
//			//输出字符串(匹配\s的组)
//			if(rr[0].id == 0)
//				std::cout.write((char *)rr[0].begin,rr[0].end - rr[0].begin);
//			//移动指针
//			p = (unsigned char *)rr.m_Match.end;
//		}
//		else
//		{
//			//无匹配，输出剩余的字符
//			std::cout.write((char *)p,pend - p);
//			//结束
//			p = pend;
//		}
//	}
//	//清理
//	delete [] pbuff;
//	return 0;
//}
