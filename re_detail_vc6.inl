// re_detail_vc6.inl
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
// Modify time : 2004-6-15 22:57
// ��ʷ: �ο�re_history.txt
//-----------------------------------------------------------------------------

//template<class TRAITS>
re_error /*re_pattern<TRAITS>::*/parse(const typename TRAITS::re_char * szStart,
										const typename TRAITS::re_char * szEnd = NULL,
										re_flags flags = rf_zero)
{
	clear();
	_push_syntex(new rp_array);
	
	if(szEnd == NULL)
		szEnd = reinterpret_cast<const re_char *>(reinterpret_cast<const char *>(szStart) + traits::ByteLen(szStart));
	
	assert(szEnd >= szStart);
	if(szEnd < szStart)
		return ree_address;

	m_eFlags = flags;
	if(flags & rf_nocase)
	{
		size_t nByteLen = reinterpret_cast<const char *>(szEnd) - reinterpret_cast<const char *>(szStart);
		re_char * sz = reinterpret_cast<re_char *>(new char[nByteLen]);
		
		ToLower(szStart,szEnd,sz);
		szStart = sz;
		szEnd = reinterpret_cast<const re_char *>(reinterpret_cast<const char *>(szStart) + nByteLen);
		
		m_eError = _parse(szStart,szEnd);
		m_Syntexes.clear();
		delete [] reinterpret_cast<char *>(sz);
		if(m_eError == ree_ok)
		{
			if(flags & rt_optimize)
				_optimize();
		}
		else
		{
			clear();
		}

		return m_eError;
	}
	else
	{
		m_eError = _parse(szStart,szEnd);
		m_Syntexes.clear();
		if(m_eError == ree_ok)
		{
			if(flags & rt_optimize)
				_optimize();
		}
		else
		{
			re_error Err = m_eError;
			clear();
			return Err;
		}

		return ree_ok;
	}
}

//template<class TRAITS>
re_error /*re_pattern<TRAITS>::*/match(const typename TRAITS::re_char * szStart,
								   re_result<TRAITS> & result,
								   const typename TRAITS::re_char * szEnd = NULL) const
{
	if(m_Syntex == NULL)
		return ree_noparse;
	if(szEnd == NULL)
		szEnd = reinterpret_cast<const re_char *>(reinterpret_cast<const char *>(szStart) + traits::ByteLen(szStart));

	assert(szEnd >= szStart);
	if(szEnd < szStart)
		return ree_address;

	result.m_Groups.clear();
	if(option() & rf_nocase)
	{
		size_t nByteLen = reinterpret_cast<const char *>(szEnd) - reinterpret_cast<const char *>(szStart);
		re_char * szNew = reinterpret_cast<re_char *>(new char[nByteLen]);
		ToLower(szStart,szEnd,szNew);
		szEnd = reinterpret_cast<const re_char *>(reinterpret_cast<char *>(szNew) + nByteLen);

		re_match_temp pattern_temp(szNew, szEnd, option());
		re_error e = _match(pattern_temp,szNew,result);
		if(e == ree_ok)
			result._repair(reinterpret_cast<const char *>(szStart) - reinterpret_cast<const char *>(szNew));
		delete [] reinterpret_cast<char *>(szNew);

		return e;
	}
	else
	{
		re_match_temp pattern_temp(szStart, szEnd, option());
		return _match(pattern_temp,szStart,result);
	}
}

//template<class TRAITS>
void /*re_pattern<TRAITS>::*/clear()
{
	m_eError = ree_ok;
	m_eFlags = rf_zero;
	m_nGroupId = 0;
	if(m_Syntex != NULL)
	{
		delete m_Syntex;
		m_Syntex = NULL;
	}
}

//�滻����
//szFind:�������ʽ,������ʽ.ΪNULL��ʾʹ��֮ǰ����parse�Ľ��
//szReplaceWith:�滻���ݱ��ʽ��
//				\0��ʾ�滻�ɲ��ҵ�������,
//				\1,\2,...��ʾ�滻���������ʽ�е�0������1����...��
//				���ಿ��ԭ�ⲻ�����滻
//szReplace:��Ҫ���滻���ı�
//strResult:�滻�Ľ���ı�.ģ�����STRING���͡�Ҫ��֧��STRING(CharT *,size_t)��ʽ����һ��String����
//			֧��+=�������š��ƺ�һ�����Ƶ�String�඼Ӧ��֧��

//template<class TRAITS>
template<class STRING>
re_error /*re_pattern<TRAITS>::*/replace(const typename TRAITS::re_char * szFind,
										 const typename TRAITS::re_char * szReplaceWith,
										 const typename TRAITS::re_char * szReplace,
										 STRING & strResult,
										 re_flags flags/* = rf_zero*/)
{
	if(NULL != szFind)
		parse(szFind,NULL,flags);
	if(m_Syntex == NULL)
		return ree_noparse;

	const this_type & rpf = *this;
	result_type rrw;
	{
		this_type	rpw;
		rpw.parse((re_char *)ReplaceRegular());
		rpw.match(szReplaceWith,rrw);
	}

	const re_char * pend = reinterpret_cast<const re_char *>(reinterpret_cast<const char *>(szReplace) + ByteLen((re_char *)szReplace));
	result_type	rr;
	for(const re_char * p = szReplace; p < pend; )
	{
		if(ree_ok == rpf.match(p,rr,pend))
		{
			//ƥ�䵽���ʽ
			//������������ַ�
			strResult += STRING((re_char *)p,rr.m_Match.begin - p);
//			strResult.append((re_char *)p,rr.m_Match.begin - p);
			for(size_t i=0; i<rrw.GroupCount(); ++i)
			{
				match_type mch = rrw[i];
				if(mch.id == 0)
				{
					strResult += STRING((re_char *)mch.begin,mch.end - mch.begin);
//					strResult.append((CharT *)mch.begin,mch.end - mch.begin);
				}
				else 
				{
					size_t nGroup = typename TRAITS::toi((re_char *)mch.begin + 1);
					if(nGroup == 0)
					{
						strResult += STRING((re_char *)rr.m_Match.begin,rr.m_Match.end - rr.m_Match.begin);
//						strResult.append((CharT *)rr.m_Match.begin,rr.m_Match.end - rr.m_Match.begin);
					}
					else if(nGroup <= rr.GroupCount())
					{
						mch = rr[nGroup - 1];
						strResult += STRING((re_char *)mch.begin,mch.end - mch.begin);
//						strResult.append((CharT *)mch.begin,mch.end - mch.begin);
					}
				}
			}
			//�ƶ�ָ��
			p = (re_char *)rr.m_Match.end;
		}
		else
		{
			strResult += STRING((re_char *)p,pend - p);
//			strResult.append((CharT *)p,pend - p);
			//����
			p = pend;
		}
	}
	
	return ree_ok;
}
