// re_syntex.inl
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
// Modify time : 2005-03-12   16:58
// 历史: 参看re_history.txt
//-----------------------------------------------------------------------------
public:
	// helper
	static inline const re_char * Next(const re_char * sz){
		return traits::Next(sz);}
	static inline int Cmp(const re_char *szLeft, const re_char *szRight, size_t nByteCount){
		return traits::Cmp(szLeft,szRight,nByteCount);}
	static inline int CmpNoCase(const re_char *szLeft, const re_char *szRight, size_t nByteCount){
		return traits::CmpNoCase(szLeft,szRight,nByteCount);}
	static inline void ToLower(const re_char *sz,const re_char * szEnd,re_char * szOut){
		traits::ToLower(sz,szEnd,szOut);}
	static inline int IsDigit(const re_char * sz){
		return traits::IsDigit(sz);}
	static inline const re_char** Abbrevs(){
		return traits::Abbrevs();}
	static inline const re_char * ReplaceRegular(){
		return traits::ReplaceRegular();}
	static inline size_t ByteLen(const re_char *sz){
		return traits::ByteLen(sz);	}
	static inline unsigned char ToUnsigned(re_char c){
		return traits::ToUnsigned(c);}
	static inline unsigned char ToUnsigned(const re_char * sz){
		return traits::ToUnsigned(*sz);}
	static inline size_t ToSize_t(const re_char * sz){
		return traits::ToSize_t(sz);}
	
	inline long to_int(const re_char *szStart,const re_char * szEnd,re_char **szOut){
		return traits::to_int(szStart,szEnd,szOut);}
	inline long to_hex(const re_char *szStart,const re_char * szEnd,re_char **szOut){
		return traits::to_hex(szStart,szEnd,szOut);}
	inline long to_oct(const re_char *szStart,const re_char * szEnd,re_char **szOut){
		return traits::to_oct(szStart,szEnd,szOut);}

private:
	struct re_block
	{
		typedef std::vector<re_block *>	myvector;

		re_types			eType;
		const re_char *		szStart;
		const re_char *		szEnd;
		myvector			m_Block;
		
		bool _is_end(const re_char * sz) const{
			return sz >= szEnd;}

		~re_block(){
			for(typename myvector::iterator i=m_Block.begin(); i!=m_Block.end(); ++i)
				delete *i;
		}
	};

	struct re_match_temp
	{
		bool				bBreak;
		const re_char *		szStart;
		const re_char *		szEnd;
		re_flags			eFlags;
		re_error			eError;

		re_match_temp(const re_char * _s,const re_char * _e,re_flags _f)
			:bBreak(false),szStart(_s),szEnd(_e),eFlags(_f),eError(ree_ok){}

		bool _is_start(const re_char * sz) const{
			return sz == szStart;}
		bool _is_end(const re_char * sz) const{
			return sz >= szEnd;}
		bool _is_break() const{
			return bBreak != false;
		}
		void _break(){
			bBreak = true;
		}
		re_flags option() const{
			return eFlags;
		}
	};
	//syntex
	//规则一：匹配则将指针移动到下一个位置
	//规则二：不匹配不要移动指针
	struct rp_base
	{
		virtual ~rp_base(){}
		virtual bool match(re_match_temp & pattern,const re_char * & sz,result_type & result) = 0;
	};
	struct rp_array : public rp_base
	{
		typedef std::vector<rp_base *>	myvector;
		myvector	m_Syntex;

		virtual ~rp_array(){
			for(typename myvector::iterator i=m_Syntex.begin(); i!=m_Syntex.end(); ++i)
				delete *i;
		}
		virtual bool match(re_match_temp & pattern,const re_char * & sz,result_type & result)
		{
			for(typename myvector::iterator i=m_Syntex.begin(); i!=m_Syntex.end(); ++i)
			{
				if((*i)->match(pattern,sz,result) == false)
					return false;
			}
			return true;
		}
	};
	struct rp_group : public rp_base
	{
		size_t		m_GroupId;
		rp_array	m_Array;
		virtual bool match(re_match_temp & pattern,const re_char * & sz,result_type & result)
		{
			const re_char * begin = sz;
			if(m_Array.match(pattern,sz,result))
			{
				result.m_Groups.push_back(match_type(m_GroupId,begin,sz));
				return true;
			}
			return false;
		}
	};
	struct rp_exactness : public rp_base
	{
		size_t	m_Coding;
		virtual bool match(re_match_temp & pattern,const re_char * & sz,result_type & result)
		{
			if(!pattern._is_end(sz) && ToSize_t(sz) == m_Coding)
			{
				sz = Next(sz);
				return true;
			}
			return false;
		}
	};
	struct rp_any_single_line : public rp_base
	{
		virtual bool match(re_match_temp & pattern,const re_char * & sz,result_type & result)
		{
			if(pattern._is_end(sz))
				return false;
			sz = Next(sz);
			return true;
		}
	};
	struct rp_any_multi_line : public rp_base
	{
		virtual bool match(re_match_temp & pattern,const re_char * & sz,result_type & result)
		{
			if(pattern._is_end(sz))
				return false;
			if('\n' != ToSize_t(sz))
			{
				sz = Next(sz);
				return true;
			}
			return false;
		}
	};
	struct range_range{
		size_t begin;
		size_t end;
	};
	struct rp_range : public rp_base
	{
		std::vector<range_range>	m_Range;
		std::vector<size_t>			m_In;
		virtual bool match(re_match_temp & pattern,const re_char * & sz,result_type & result)
		{
			if(pattern._is_end(sz))
				return false;
			size_t c = ToSize_t(sz);
			if(std::find(m_In.begin(),m_In.end(),c) != m_In.end())
			{
				sz = Next(sz);
				return true;
			}
			typedef std::vector<range_range>::iterator	iterator;
			for(iterator i=m_Range.begin(); i<m_Range.end(); ++i)
			{
				if((*i).begin <= c && c <= (*i).end)
				{
					sz = Next(sz);
					return true;
				}
			}
			return false;
		}
	};
	struct rp_not_range : public rp_range
	{
		using rp_range::m_In;
		using rp_range::m_Range;
		virtual bool match(re_match_temp & pattern,const re_char * & sz,result_type & result)
		{
			if(pattern._is_end(sz))
				return false;
			size_t c = ToSize_t(sz);
			if(std::find(m_In.begin(),m_In.end(),c) != m_In.end())
				return false;
			typedef std::vector<range_range>::iterator	iterator;
			for(iterator i=m_Range.begin(); i<m_Range.end(); ++i)
			{
				if((*i).begin <= c && c <= (*i).end)
					return false;
			}

			sz = Next(sz);
			return true;
		}
	};
	struct rp_begin_line : public rp_base
	{
		virtual bool match(re_match_temp & pattern,const re_char * & sz,result_type & result){
			if(pattern._is_start(sz))
				return true;
			const re_char * szTemp = reinterpret_cast<const re_char *>(reinterpret_cast<const char *>(sz) - 1);
			if(szTemp > pattern.szStart && ToSize_t(szTemp) == '\n')
				return true;
			return false;
		}
	};
	struct rp_end_line : public rp_base
	{
		virtual bool match(re_match_temp & pattern,const re_char * & sz,result_type & result){
			if(pattern._is_end(sz))
				return (pattern.option() & rf_endasenter) ? true : false;
			switch(ToSize_t(sz))
			{
			case '\r':
				{
					const re_char * szSave = sz;
					sz = Next(sz);
					if(pattern._is_end(sz))
						return true;
					if('\n' == ToSize_t(sz))
					{
						sz = Next(sz);
						return true;
					}
					sz = szSave;
					return false;
				}
				break;
			case '\n':
				sz = Next(sz);
				return true;
				break;
			}
			return false;
		}
	};
	struct rp_begin_all : public rp_base
	{
		virtual bool match(re_match_temp & pattern,const re_char * & sz,result_type & result){
			if(pattern._is_start(sz))
				return true;
			pattern._break();
			return false;
		}
	};
	struct rp_end_all : public rp_base
	{
		virtual bool match(re_match_temp & pattern,const re_char * & sz,result_type & result){
			return pattern._is_end(sz);
		}
	};
	//<m:n>
	struct rp_repeat_specify_greed : public rp_base
	{
		rp_base *	m_Leaf;
		long		m_Min,m_Max;
		virtual ~rp_repeat_specify_greed(){
			delete m_Leaf;
		}
		virtual bool match(re_match_temp & pattern,const re_char * & sz,result_type & result){

			const re_char * szSave = sz;
			long l;
			for(l=0; l<m_Min; ++l)
				if(pattern._is_end(szSave) || m_Leaf->match(pattern,szSave,result) == false)
					return false;
			if(m_Max > 0)
				for(;l<m_Max && !pattern._is_end(szSave) && m_Leaf->match(pattern,szSave,result); ++l);
			else
			{
				for(;!pattern._is_end(szSave); ++l)
				{
					const re_char * szTemp = szSave;
					if(!m_Leaf->match(pattern,szSave,result))
						break;
					if(szSave == szTemp)
					{//有些无穷循环的在分析阶段没有检测到
						pattern.eError = ree_recursion_always;
						pattern._break();
						return false;
					}
				}
			}
			
			sz = szSave;
			return true;
		}
	};
	//<m:n>?
	struct rp_repeat_specify : public rp_base
	{
		rp_base *	m_Leaf;
		rp_array *	m_Next;
		long		m_Min,m_Max;
		virtual ~rp_repeat_specify(){
			delete m_Leaf;
		}
		virtual bool match(re_match_temp & pattern,const re_char * & sz,result_type & result){
			const re_char * szSave = sz;
			long l;
			for(l=0; l<m_Min; ++l)
				if(pattern._is_end(szSave) || m_Leaf->match(pattern,szSave,result) == false)
					return false;
			if(m_Max > 0)
			{
				for(;l<m_Max && !pattern._is_end(szSave); ++l)
				{
					const re_char * szTemp = szSave;
					if(m_Next->match(pattern,szTemp,result))
					{
						sz = szSave;
						return true;
					}
					if(!m_Leaf->match(pattern,szSave,result))
						break;
				}
			}
			else
			{
				for(;!pattern._is_end(szSave); ++l)
				{
					const re_char * szTemp = szSave;
					if(m_Next->match(pattern,szTemp,result))
					{
						sz = szSave;
						return true;
					}
					if(!m_Leaf->match(pattern,szSave,result))
						break;
					if(szSave == szTemp)
					{//有些无穷循环的在分析阶段没有检测到
						pattern.eError = ree_recursion_always;
						pattern._break();
						return false;
					}
				}
			}

			sz = szSave;
			return true;
		}
	};
	//*?
	struct rp_repeat_more_zero : public rp_base
	{
		rp_base *	m_Leaf;
		rp_array *	m_Next;
		virtual ~rp_repeat_more_zero(){
			delete m_Leaf;
		}
		virtual bool match(re_match_temp & pattern,const re_char * & sz,result_type & result){
			const re_char * szSave = sz;
			for(;!pattern._is_end(szSave);)
			{
				const re_char * szTemp = szSave;
				if(m_Next->match(pattern,szTemp,result))
				{
					sz = szSave;
					return true;
				}
				szTemp = szSave;
				if(!m_Leaf->match(pattern,szSave,result))
					break;
				else
				{
					if(szSave == szTemp)
					{//有些无穷循环的在分析阶段没有检测到
						pattern.eError = ree_recursion_always;
						pattern._break();
						return false;
					}
				}
			}
			
			sz = szSave;
			return true;
		}
	};
	//+?
	struct rp_repeat_more_one : public rp_base
	{
		rp_base *	m_Leaf;
		rp_array *	m_Next;
		virtual ~rp_repeat_more_one(){
			delete m_Leaf;
		}
		virtual bool match(re_match_temp & pattern,const re_char * & sz,result_type & result){
			const re_char * szSave = sz;
			if(pattern._is_end(szSave) || m_Leaf->match(pattern,szSave,result) == false)
				return false;
			for(;!pattern._is_end(szSave);)
			{
				const re_char * szTemp = szSave;
				if(m_Next->match(pattern,szTemp,result))
				{
					sz = szSave;
					return true;
				}
				szTemp = szSave;
				if(!m_Leaf->match(pattern,szSave,result))
					break;
				else
				{
					if(szSave == szTemp)
					{//有些无穷循环的在分析阶段没有检测到
						pattern.eError = ree_recursion_always;
						pattern._break();
						return false;
					}
				}
			}

			sz = szSave;
			return true;
		}
	};
	//??
	struct rp_repeat_zero_one : public rp_base
	{
		rp_base *	m_Leaf;
		rp_array *	m_Next;
		virtual ~rp_repeat_zero_one(){
			delete m_Leaf;
		}
		virtual bool match(re_match_temp & pattern,const re_char * & sz,result_type & result){
			const re_char * szTemp = sz;
			if(m_Next->match(pattern,szTemp,result))
				return true;
			if(!pattern._is_end(sz))
				m_Leaf->match(pattern,sz,result);
			return true;
		}
	};
	//*
	struct rp_repeat_more_zero_greed : public rp_base
	{
		rp_base *	m_Leaf;
		virtual ~rp_repeat_more_zero_greed(){
			delete m_Leaf;
		}
		virtual bool match(re_match_temp & pattern,const re_char * & sz,result_type & result){
			const re_char * szSave = sz;
			for(;!pattern._is_end(szSave);)
			{
				const re_char * szTemp = szSave;
				if(m_Leaf->match(pattern,szSave,result))
				{
					if(szSave == szTemp)
					{//有些无穷循环的在分析阶段没有检测到
						pattern.eError = ree_recursion_always;
						pattern._break();
						return false;
					}
				}
				else
				{
					break;
				}
			}
			
			sz = szSave;
			return true;
		}
	};
	//+
	struct rp_repeat_more_one_greed : public rp_base
	{
		rp_base *	m_Leaf;
		virtual ~rp_repeat_more_one_greed(){
			delete m_Leaf;
		}
		virtual bool match(re_match_temp & pattern,const re_char * & sz,result_type & result){
			const re_char * szSave = sz;
			if(pattern._is_end(szSave) || m_Leaf->match(pattern,szSave,result) == false)
				return false;
			for(;!pattern._is_end(szSave);)
			{
				const re_char * szTemp = szSave;
				if(m_Leaf->match(pattern,szSave,result))
				{
					if(szSave == szTemp)
					{//有些无穷循环的在分析阶段没有检测到
						pattern.eError = ree_recursion_always;
						pattern._break();
						return false;
					}
				}
				else
				{
					break;
				}
			}

			sz = szSave;
			return true;
		}
	};
	//?
	struct rp_repeat_zero_one_greed : public rp_base
	{
		rp_base *	m_Leaf;
		virtual ~rp_repeat_zero_one_greed(){
			delete m_Leaf;
		}
		virtual bool match(re_match_temp & pattern,const re_char * & sz,result_type & result){
			if(!pattern._is_end(sz))
				m_Leaf->match(pattern,sz,result);
			return true;
		}
	};
	//!
	struct rp_not_regular : public rp_base
	{
		rp_base *	m_Leaf;
		virtual ~rp_not_regular(){
			delete m_Leaf;
		}
		virtual bool match(re_match_temp & pattern,const re_char * & sz,result_type & result){
			const re_char * szSave = sz;
			result_type r;
			if(!m_Leaf->match(pattern,szSave,r))
			{
				sz = Next(sz);
				return true;
			}
			return false;
		}
	};
	//|
	struct rp_or_regular : public rp_base
	{
		rp_base *	m_First;
		rp_base *	m_Second;
		virtual ~rp_or_regular(){
			delete m_First;
			delete m_Second;
		}
		virtual bool match(re_match_temp & pattern,const re_char * & sz,result_type & result){
			const re_char * szSave = sz;
			if(m_First->match(pattern,sz,result))
				return true;
			sz = szSave;
			if(m_Second->match(pattern,sz,result))
				return true;
			return false;
		}
	};
	//&
	struct rp_and_regular : public rp_base
	{
		rp_base *	m_First;
		rp_base *	m_Second;
		virtual ~rp_and_regular(){
			delete m_First;
			delete m_Second;
		}
		virtual bool match(re_match_temp & pattern,const re_char * & sz,result_type & result){
			const re_char * sz1 = sz;
			const re_char * sz2 = sz;
			if(!m_First->match(pattern,sz1,result))
				return false;
			if(!m_Second->match(pattern,sz2,result))
				return false;
			if(sz1 == sz2)
			{
				sz = sz1;
				return true;
			}
			return false;
		}
	};

//-----------------------------------------------------------------------------

	re_error _match(re_match_temp & pattern,const re_char * sz,result_type & result) const
	{
		if(m_Syntex == NULL)
			return ree_noparse;
		while(pattern._is_break() == false && !pattern._is_end(sz))
		{
			const re_char * szTemp = sz;
			if(m_Syntex->match(pattern,szTemp,result))
			{
				result.m_Match.begin = sz;
				result.m_Match.end = szTemp;
				return ree_ok;
			}
			sz = Next(sz);
		}

		return pattern.eError == ree_ok ? ree_nomatch : pattern.eError;
	}

//-----------------------------------------------------------------------------
	
public:
	// 分析的结果
	re_error				m_eError;
	re_flags				m_eFlags;
	rp_array *				m_Syntex;
	// 分析时的临时变量
	size_t					m_nGroupId;
	std::vector<rp_array *>	m_Syntexes;

	// parser
	void _push_syntex(rp_array * pSyntex){
		m_Syntexes.push_back(m_Syntex);
		m_Syntex = pSyntex;
	}
	void _pop_syntex(){
		m_Syntex = m_Syntexes.back();
		m_Syntexes.pop_back();
	}
/*
	void _break(){
		m_bBreak = true;
	}
*/
	void _optimize(){
	}
	size_t _get_escape_char(re_block * root,const re_char * & sz)
	{
		size_t c = ToSize_t(sz);
		const re_char * szSave = sz;
		sz = Next(sz);
		switch(c)
		{
		case rt_return:
			if(option() & rf_return)
				return '\r';
			return c;
		case rt_enter:
			return '\n';
		case rt_tab:
			return '\t';
		case rt_hex_prefix:
		case rt_hex_prefix2:
			{
				if(root->_is_end(sz))
					return c;
				re_char * szEnd;
				long lHex = to_hex(sz,root->szEnd,&szEnd);
				if(sz == szEnd)
					return c;
				sz = szEnd;
				return (size_t)lHex;
			}
			break;
		case rt_oct_prefix:
		case rt_oct_prefix2:
			{
				if(root->_is_end(sz))
					return c;
				re_char * szEnd;
				long lHex = to_oct(sz,root->szEnd,&szEnd);
				if(sz == szEnd)
					return c;
				sz = szEnd;
				return (size_t)lHex;
			}
			break;
		default:
			if(IsDigit(szSave))
			{
				re_char * szEnd;
				long lDigit = to_int(szSave,root->szEnd,&szEnd);
				sz = szEnd;
				return (size_t)lDigit;
			}
			break;
		}
		return c;
	}
	re_error _parse_not_range(re_block * root,const re_char * & sz)
	{
		sz = Next(sz);
		if(root->_is_end(sz))
			return ree_syntex;

		rp_range * Syntex;
		if(rt_not_range == ToSize_t(sz))
		{
			sz = Next(sz);
			if(root->_is_end(sz))
				return ree_syntex;
			Syntex = new rp_not_range;
		}
		else
		{
			Syntex = new rp_range;
		}

		m_Syntex->m_Syntex.push_back(Syntex);
		return _parse_range(root,sz,*Syntex);
	}
	re_error _parse_range(re_block * root,const re_char * & sz,rp_range & Syntex)
	{
		if(root->_is_end(sz))
			return ree_syntex;
		
		size_t old = ToSize_t(sz);
		do
		{
			sz = Next(sz);
			size_t begin = old;
			if(rt_range_end == begin)
				break;
			if(rt_escape == begin)
			{
				if(root->_is_end(sz))
					return ree_syntex;
				begin = _get_escape_char(root,sz);
			}
			if(root->_is_end(sz))
				return ree_syntex;

			old = ToSize_t(sz);
			if(rt_range_sign == old)
			{//范围
				sz = Next(sz);
				if(root->_is_end(sz))
					return ree_syntex;
				
				old = ToSize_t(sz);
				if(rt_range_end == old)
					return ree_syntex;
				if(rt_escape == old)
				{
					sz = Next(sz);
					if(root->_is_end(sz))
						return ree_syntex;
					old = _get_escape_char(root,sz);
				}
				else
					sz = Next(sz);
				
				range_range	rr;
				if(begin < old)
				{
					rr.begin = begin;
					rr.end = old;
				}
				else
				{
					rr.begin = old;
					rr.end = begin;
				}
				Syntex.m_Range.push_back(rr);
				
				if(!root->_is_end(sz))
					old = ToSize_t(sz);
			}
			else
			{
				Syntex.m_In.push_back(begin);
			}
		}
		while(!root->_is_end(sz));
		
		if(Syntex.m_In.size() == 0 && Syntex.m_Range.size() == 0)
			return ree_syntex;
		
		std::sort(Syntex.m_In.begin(),Syntex.m_In.end());
		return ree_ok;
	}
	re_error _parse_begin_line(const re_char * & sz)
	{
		rp_base * Syntex;
		if(option() & rf_multi_line)
			Syntex = new rp_begin_line;
		else
			Syntex = new rp_begin_all;
		m_Syntex->m_Syntex.push_back(Syntex);
		sz = Next(sz);
		return ree_ok;
	}
	re_error _parse_end_line(const re_char * & sz)
	{
		rp_base * Syntex;
		if(option() & rf_multi_line)
			Syntex = new rp_end_line;
		else
			Syntex = new rp_end_all;
		m_Syntex->m_Syntex.push_back(Syntex);
		sz = Next(sz);
		return ree_ok;
	}
	re_error _parse_any(const re_char * & sz)
	{
		sz = Next(sz);
		rp_base * Syntex;
		if(option() & rf_multi_line)
			Syntex = new rp_any_multi_line;
		else
			Syntex = new rp_any_single_line;
		m_Syntex->m_Syntex.push_back(Syntex);
		return ree_ok;
	}
	re_error _parse_more_zero(re_block * root,const re_char * & sz)
	{
		if(m_Syntex->m_Syntex.size() == 0)
			return ree_syntex;
		sz = Next(sz);
		if(!root->_is_end(sz))
		{
			if(rt_zero_one == ToSize_t(sz))
			{
				rp_repeat_more_zero * Syntex = new rp_repeat_more_zero;
				Syntex->m_Leaf = m_Syntex->m_Syntex.back();
				m_Syntex->m_Syntex.back() = Syntex;
				sz = Next(sz);
				
				rp_array * pNew = new rp_array;
				Syntex->m_Next = pNew;
				m_Syntex->m_Syntex.push_back(pNew);
				_push_syntex(pNew);
				const re_char * szSave = root->szStart;
				root->szStart = sz;
				re_error e = _parse_array(root);
				root->szStart = szSave;
				_pop_syntex();

				sz = root->szEnd;

				return e;
			}
		}

		rp_repeat_more_zero_greed * Syntex = new rp_repeat_more_zero_greed;
		Syntex->m_Leaf = m_Syntex->m_Syntex.back();
		m_Syntex->m_Syntex.back() = Syntex;

		return ree_ok;
	}
	re_error _parse_more_one(re_block * root,const re_char * & sz)
	{
		if(m_Syntex->m_Syntex.size() == 0)
			return ree_syntex;
		sz = Next(sz);
		if(!root->_is_end(sz))
		{
			if(rt_zero_one == ToSize_t(sz))
			{
				rp_repeat_more_one * Syntex = new rp_repeat_more_one;
				Syntex->m_Leaf = m_Syntex->m_Syntex.back();
				m_Syntex->m_Syntex.back() = Syntex;
				sz = Next(sz);
				
				rp_array * pNew = new rp_array;
				Syntex->m_Next = pNew;
				m_Syntex->m_Syntex.push_back(pNew);
				_push_syntex(pNew);
				const re_char * szSave = root->szStart;
				root->szStart = sz;
				re_error e = _parse_array(root);
				root->szStart = szSave;
				_pop_syntex();

				sz = root->szEnd;

				return e;
			}
		}

		rp_repeat_more_one_greed * Syntex = new rp_repeat_more_one_greed;
		Syntex->m_Leaf = m_Syntex->m_Syntex.back();
		m_Syntex->m_Syntex.back() = Syntex;

		return ree_ok;
	}
	re_error _parse_not(re_block * root,const re_char * & sz)
	{
		if(m_Syntex->m_Syntex.size() == 0)
			return ree_syntex;
		sz = Next(sz);

		rp_not_regular * Syntex = new rp_not_regular;
		Syntex->m_Leaf = m_Syntex->m_Syntex.back();
		m_Syntex->m_Syntex.back() = Syntex;

		return ree_ok;
	}
	re_error _parse_zero_one(re_block * root,const re_char * & sz)
	{
		if(m_Syntex->m_Syntex.size() == 0)
			return ree_syntex;
		sz = Next(sz);
		if(!root->_is_end(sz))
		{
			if(rt_zero_one == ToSize_t(sz))
			{
				rp_repeat_zero_one * Syntex = new rp_repeat_zero_one;
				Syntex->m_Leaf = m_Syntex->m_Syntex.back();
				m_Syntex->m_Syntex.back() = Syntex;
				sz = Next(sz);
				
				rp_array * pNew = new rp_array;
				Syntex->m_Next = pNew;
				m_Syntex->m_Syntex.push_back(pNew);
				_push_syntex(pNew);
				const re_char * szSave = root->szStart;
				root->szStart = sz;
				re_error e = _parse_array(root);
				root->szStart = szSave;
				_pop_syntex();

				sz = root->szEnd;

				return e;
			}
		}

		rp_repeat_zero_one_greed * Syntex = new rp_repeat_zero_one_greed;
		Syntex->m_Leaf = m_Syntex->m_Syntex.back();
		m_Syntex->m_Syntex.back() = Syntex;

		return ree_ok;
	}
	re_error _parse_count(re_block * root,const re_char * & sz)
	{
		if(m_Syntex->m_Syntex.size() == 0)
			return ree_syntex;
		sz = Next(sz);
		if(root->_is_end(sz))
			return ree_syntex;

		//<m:n>
		re_char * end;
		long m = to_int(sz,root->szEnd,&end);
		if(m < 0)
			return ree_syntex;
		if(root->_is_end(end) || rt_colon != ToSize_t(end))
			return ree_syntex;
		sz = Next(end);
		if(root->_is_end(sz))
			return ree_syntex;
		long n = to_int(sz,root->szEnd,&end);
		if(end == sz)
		{
			n = -1;
			if(root->_is_end(end) || rt_count_end != ToSize_t(end))
				return ree_syntex;
		}
		else
		{
			if(n <= 0)
				return ree_syntex;
			if(m > n)
				return ree_syntex;
			if(root->_is_end(end) || rt_count_end != ToSize_t(end))
				return ree_syntex;
		}

		sz = Next(end);

		//<m:n>?
		if(!root->_is_end(sz))
		{
			if(rt_zero_one == ToSize_t(sz))
			{
				rp_repeat_specify * Syntex = new rp_repeat_specify;
				Syntex->m_Leaf = m_Syntex->m_Syntex.back();
				Syntex->m_Min = m;
				Syntex->m_Max = n;
				m_Syntex->m_Syntex.back() = Syntex;
				sz = Next(sz);
				
				rp_array * pNew = new rp_array;
				Syntex->m_Next = pNew;
				m_Syntex->m_Syntex.push_back(pNew);
				_push_syntex(pNew);
				const re_char * szSave = root->szStart;
				root->szStart = sz;
				re_error e = _parse_array(root);
				root->szStart = szSave;
				_pop_syntex();

				sz = root->szEnd;

				return e;
			}
		}

		rp_repeat_specify_greed * Syntex = new rp_repeat_specify_greed;
		Syntex->m_Leaf = m_Syntex->m_Syntex.back();
		Syntex->m_Min = m;
		Syntex->m_Max = n;
		m_Syntex->m_Syntex.back() = Syntex;

		return ree_ok;
	}
	re_error _parse_exactness(const re_char * & sz)
	{
		rp_exactness * Syntex = new rp_exactness;
		Syntex->m_Coding = ToSize_t(sz);
		m_Syntex->m_Syntex.push_back(Syntex);
		sz = Next(sz);
		return ree_ok;
	}
	re_error _parse_escape(re_block * root,const re_char * & sz)
	{
		sz = Next(sz);
		if(root->_is_end(sz))
			return ree_syntex;
		size_t c = ToSize_t(sz);
		const re_char ** szAbbrevs = Abbrevs();
		for(;*szAbbrevs; ++szAbbrevs)
		{
			if(ToSize_t(*szAbbrevs) == c)
			{
				const re_char * szSyntex = Next(*szAbbrevs);
				rp_array * Syntex = new rp_array;

				//save info
				m_Syntex->m_Syntex.push_back(Syntex);
				_push_syntex(Syntex);
				const re_char * szEnd = reinterpret_cast<const re_char *>(reinterpret_cast<const char *>(szSyntex) + ByteLen(szSyntex));
				re_error e = _parse(szSyntex,szEnd);
				_pop_syntex();
				sz = Next(sz);

				return e == ree_ok ? ree_ok : ree_interior;
			}
		}
		
		rp_exactness * Syntex = new rp_exactness;
		Syntex->m_Coding = _get_escape_char(root,sz);
		m_Syntex->m_Syntex.push_back(Syntex);
		
		return ree_ok;
	}
	re_error _parse_array_root(re_block * root)
	{
		re_error e = ree_ok;
		const re_char * sz = root->szStart;
		while(!root->_is_end(sz))
		{
			switch(ToSize_t(sz))
			{
			case rt_any:			//'.',
				e = _parse_any(sz);
				break;
			case rt_escape:			//'\\',		//转义符号
				e = _parse_escape(root,sz);
				break;
			case rt_begin_line:		//'^',		//作为多行匹配的时候，匹配行的开始
				e = _parse_begin_line(sz);
				break;
			case rt_end_line:		//'$',		//作为多行匹配的时候，匹配行的结束。
												//行的结束以'\r\n'或'\n'作为结束。根据一个选项决定如何匹配
				e = _parse_end_line(sz);
				break;
			case rt_range_begin:	//'[',
				e = _parse_not_range(root,sz);
				break;
			case rt_more_zero:		//'*',
				e = _parse_more_zero(root,sz);
				break;
			case rt_more_one:		//'+',
				e = _parse_more_one(root,sz);
				break;
			case rt_zero_one:		//'?',
				e = _parse_zero_one(root,sz);
				break;
			case rt_count_begin:	//'<',		//<m:n>
				e = _parse_count(root,sz);
				break;
			case rt_new_begin:		//'(',
			//case rt_new_end:		//')',
			case rt_group_begin:	//'{',
			//case rt_group_end:		//'}',
			case rt_not_regular:	//'!',		//否定正则表达式
				e = _parse_not(root,sz);
				break;
			case rt_and_regular:	//'&',		//与正则表达式
			case rt_or_regular:		//'|',		//或正则表达式
			//case rt_range_end:
			//case rt_count_end:
				e = ree_interior;
				break;
			default:
				e = _parse_exactness(sz);
				break;
			}

			if(e != ree_ok)
				break;
		}

		return e;
	}
	re_error _parse_array_or(re_block * root)
	{
		if(m_Syntex->m_Syntex.size() == 0)
			return ree_syntex;

		rp_array * Syntex = new rp_array;
		_push_syntex(Syntex);
		re_error e = _parse_block(root);
		_pop_syntex();
		if(e != ree_ok)
		{
			delete Syntex;
			return e;
		}
		if(Syntex->m_Syntex.size() <= 0)	//r|
		{
			delete Syntex;
			return ree_syntex;
		}

		rp_or_regular * rp_or = new rp_or_regular;
		rp_or->m_First = m_Syntex->m_Syntex.back();
		rp_or->m_Second = Syntex->m_Syntex.front();
		m_Syntex->m_Syntex.back() = rp_or;

		typename rp_array::myvector::iterator i = Syntex->m_Syntex.begin();
		for(++i;i!=Syntex->m_Syntex.end();++i)
			m_Syntex->m_Syntex.push_back(*i);
		
		Syntex->m_Syntex.clear();
		delete Syntex;

		return ree_ok;
	}
	re_error _parse_array_and(re_block * root)
	{
		if(m_Syntex->m_Syntex.size() == 0)
			return ree_syntex;

		rp_array * Syntex = new rp_array;
		_push_syntex(Syntex);
		re_error e = _parse_block(root);
		_pop_syntex();
		if(e != ree_ok)
		{
			delete Syntex;
			return e;
		}
		if(Syntex->m_Syntex.size() <= 0)	//r|
		{
			delete Syntex;
			return ree_syntex;
		}

		rp_and_regular * rp_and = new rp_and_regular;
		rp_and->m_First = m_Syntex->m_Syntex.back();
		rp_and->m_Second = Syntex->m_Syntex.front();
		m_Syntex->m_Syntex.back() = rp_and;

		typename rp_array::myvector::iterator i = Syntex->m_Syntex.begin();
		for(++i;i!=Syntex->m_Syntex.end();++i)
			m_Syntex->m_Syntex.push_back(*i);

		Syntex->m_Syntex.clear();
		delete Syntex;

		return ree_ok;
	}
	re_error _parse_array_new(re_block * root)
	{
		rp_array * Syntex = new rp_array;
		m_Syntex->m_Syntex.push_back(Syntex);
		_push_syntex(Syntex);
		re_error e = _parse_block(root);
		_pop_syntex();
		if(Syntex->m_Syntex.size() == 0)
			return ree_syntex;
		return e;
	}
	re_error _parse_array_group(re_block * root)
	{
		rp_group * Syntex = new rp_group;
		Syntex->m_GroupId = m_nGroupId;
		++m_nGroupId;
		m_Syntex->m_Syntex.push_back(Syntex);
		_push_syntex(&Syntex->m_Array);
		re_error e = _parse_block(root);
		size_t s = m_Syntex->m_Syntex.size();
		_pop_syntex();
		if(s == 0)
			return ree_syntex;
		return e;
	}
	re_error _parse_array(re_block * root)
	{
		switch(root->eType)
		{
		case rt_root:
			return _parse_array_root(root);
		case rt_or_regular:
			return _parse_array_or(root);
		case rt_and_regular:
			return _parse_array_and(root);
		case rt_new_begin:
			return _parse_array_new(root);
		case rt_group_begin:
			return _parse_array_group(root);
		}
		return ree_interior;
	}
	re_error _parse_block(re_block * block)
	{
		re_error e = ree_syntex;
		for(typename re_block::myvector::iterator i=block->m_Block.begin();
			i!=block->m_Block.end();
			++i)
			{
				e = _parse_array(*i);
				if(e != ree_ok)
					break;
			}
		return e;
	}
	re_error _parse(const re_char * szStart,const re_char * szEnd)
	{
		re_block * block = new re_block;
		block->eType = rt_root;
		block->szStart = szStart;
		block->szEnd = szEnd;
		
		re_error e = _make_block(block);
		if(e == ree_ok)
			e = _parse_block(block);
		delete block;
		return e;
	}

//-----------------------------------------------------------------------------

	re_error _make_group_skip_range(re_block * root,const re_char * & szStart)
	{
		const re_char * sz = Next(szStart);
		if(root->_is_end(sz))
			return ree_syntex;
		for(;!root->_is_end(sz);sz = Next(sz))
		{
			size_t c = ToSize_t(sz);
			switch(c)
			{
			case rt_escape:
				sz = Next(sz);
				if(root->_is_end(sz))
					return ree_syntex;
				break;
			case rt_range_end:
				szStart = sz;
				return ree_ok;
			}
		}
		
		return ree_syntex;
	}
	re_error _make_group(re_block * root,const re_char * & szStart,re_types rtBegin,re_types rtEnd)
	{
		const re_char * sz = Next(szStart);
		int nCount = 1;
		for(;!root->_is_end(sz);)
		{
			size_t c = ToSize_t(sz);
			if(rtBegin == static_cast<re_types>(c))
			{
				++nCount;
			}
			else if(rtEnd == static_cast<re_types>(c))
			{
				--nCount;
			}
			else if(rt_escape == static_cast<re_types>(c))
			{
				sz = Next(sz);
				if(root->_is_end(sz))
					return ree_syntex;
			}
			else if(rt_range_begin == static_cast<re_types>(c))
			{
				re_error e = _make_group_skip_range(root,sz);
				if(e != ree_ok)
					return e;
			}
			if(nCount == 0)
				break;
			sz = Next(sz);
		}
		if(nCount != 0)
			return ree_syntex;

		re_block * block = new re_block;
		block->eType = rtBegin;
		block->szStart = Next(szStart);
		block->szEnd = sz;
		root->m_Block.push_back(block);
		re_error e = _make_block(block);
		if(e != ree_ok)
			return e;
		
		szStart = Next(sz);
		return ree_ok;
	}
	void _save_last_block(re_block * root,const re_char * & szStart,const re_char * szEnd)
	{
		if(szStart < szEnd)
		{
			re_block * block = new re_block;
			block->eType = rt_root;
			block->szStart = szStart;
			block->szEnd = szEnd;
			root->m_Block.push_back(block);
		}
	}
	re_error _make_block(re_block * root)
	{
		re_error e;
		const re_char * sz = root->szStart;
		const re_char * szBlockStart = sz;
		while(!root->_is_end(sz))
		{
			size_t rt = ToSize_t(sz);
			switch(rt)
			{
			case rt_escape:			//'\\',		//转义符号
				sz = Next(sz);
				if(root->_is_end(sz))
					return ree_syntex;
				sz = Next(sz);
				break;
			case rt_new_begin:		//'(',
				_save_last_block(root,szBlockStart,sz);
				e = _make_group(root,sz,rt_new_begin,rt_new_end);
				if(e != ree_ok)
					return e;
				szBlockStart = sz;
				break;
			case rt_new_end:		//')',
				return ree_syntex;
			case rt_group_begin:	//'{',
				_save_last_block(root,szBlockStart,sz);
				e = _make_group(root,sz,rt_group_begin,rt_group_end);
				if(e != ree_ok)
					return e;
				szBlockStart = sz;
				break;
			case rt_group_end:		//'}',
				return ree_syntex;
			case rt_and_regular:	//'&',		//与正则表达式
			case rt_or_regular:		//'|',		//或正则表达式
				{
					_save_last_block(root,szBlockStart,sz);
					sz = Next(sz);
					if(root->_is_end(sz))
						return ree_syntex;

					re_block * block = new re_block;
					block->eType = static_cast<re_types>(rt);
					block->szStart = sz;
					block->szEnd = root->szEnd;
					root->m_Block.push_back(block);
					e = _make_block(block);
					if(e != ree_ok)
						return e;
					szBlockStart = sz = root->szEnd;
				}
				break;
			case rt_range_begin:
				e = _make_group_skip_range(root,sz);
				if(e != ree_ok)
					return e;
				sz = Next(sz);
				break;
			default:
				sz = Next(sz);
				break;
			}
		}

		_save_last_block(root,szBlockStart,sz);

		return ree_ok;
	}
