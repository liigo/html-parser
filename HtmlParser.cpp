#include "HtmlParser.h"

//HtmlParser类，用于解析HTML文本
//by liigo, @2010

const char* wcsnchr(const char* pStr, int len, char c)
{
	const char *p = pStr;
	while(1)
	{
		if(*p == c) return p;
		p++;
		if((p - pStr) == len) break;
	}
	return NULL;
}

const char* getFirstUnquotedChar(const char* pStr, char endcahr)
{
	char c;
	const char* p = pStr;
	bool inQuote1 = false, inQuote2 = false; //'inQuote1', "inQuote2"
	while(c = *p)
	{
		if(c == L'\'')
		{
			inQuote1 = !inQuote1;
		}
		else if(c == L'\"')
		{
			inQuote2 = !inQuote2;
		}

		if(!inQuote1 && !inQuote2)
		{
			if(c == endcahr) return p;
		}
		p++;
	}
	return NULL;
}

//nDest and nChar can by -1
unsigned int copyStr(char* pDest, unsigned int nDest, const char* pSrc, unsigned int nChar)
{
	if(pDest == NULL || nDest == 0)
		return 0;
	if(pSrc == NULL)
	{
		pDest[0] = L'\0';
		return 0;
	}
	if(nChar == (unsigned int)-1)
		nChar = wcslen(pSrc);
	if(nChar > nDest)
		nChar = nDest;
	memcpy(pDest, pSrc, nChar * sizeof(char));
	pDest[nChar] = L'\0';
	return nChar;
}

int copyStrUtill(char* pDest, unsigned int nDest, const char* pSrc, char endchar, bool ignoreEndCharInQuoted)
{
	if(nDest == 0) return 0;
	pDest[0] = L'\0';
	const char* pSearched = (ignoreEndCharInQuoted ? getFirstUnquotedChar(pSrc,endchar) : wcschr(pSrc, endchar));
	if(pSearched <= pSrc) return 0;
	return copyStr(pDest, nDest, pSrc, pSearched - pSrc);
}

//nChar can be -1
char* duplicateStr(const char* pSrc, unsigned int nChar)
{
	if(nChar == (unsigned int)-1)
		nChar = wcslen(pSrc);
	char* pNew = (char*) malloc( (nChar+1) * sizeof(char) );
	copyStr(pNew, -1, pSrc, nChar);
	return pNew;
}

char* duplicateStrUtill(const char* pSrc, char endchar, bool ignoreEndCharInQuoted)
{
	const char* pSearched = (ignoreEndCharInQuoted ? getFirstUnquotedChar(pSrc,endchar) : wcschr(pSrc, endchar));;
	if(pSearched <= pSrc) return NULL;
	int n = pSearched - pSrc;
	return duplicateStr(pSrc, n);
}

void freeDuplicatedStr(char* p)
{
	if(p) free(p);
}

HtmlNode* HtmlParser::newHtmlNode()
{
	static char staticHtmlNodeTemplate[sizeof(HtmlNode)] = {0};
	m_HtmlNodes.Append(staticHtmlNodeTemplate, sizeof(HtmlNode));
	HtmlNode* pNode = (HtmlNode*) (m_HtmlNodes.GetPtr() + m_HtmlNodes.GetSize() - sizeof(HtmlNode));
	return pNode;
}

void HtmlParser::parseHtml(const char* szHtml)
{
	//m_html = szHtml ? szHtml : L"";
	freeHtmlNodes();
	if(szHtml == NULL || *szHtml == L'\0') return;

	char* p = (char*) szHtml;
	char* s = (char*) szHtml;
	HtmlNode* pNode = NULL;
	char c;
	bool bInQuotes = false;

	while( c = *p )
	{
		if(c == L'\"')
		{
			bInQuotes = !bInQuotes;
			p++; continue;
		}
		if(bInQuotes)
		{
			p++; continue;
		}

		if(c == L'<')
		{
			if(p > s)
			{
				//Add Text Node
				pNode = newHtmlNode();
				pNode->type = NODE_CONTENT;
				pNode->text = duplicateStrUtill(s, L'<', true);
			}
			s = p + 1;
		}
		else if(c == L'>')
		{
			if(p > s)
			{
				//Add HtmlTag Node
				pNode = newHtmlNode();
				while(isspace(*s)) s++;
				pNode->type = (*s != L'/' ? NODE_START_TAG : NODE_CLOSE_TAG);
				if(*s == L'/') s++;
				copyStrUtill(pNode->tagName, MAX_HTML_TAG_LENGTH, s, L'>', true);
				//处理自封闭的结点, 如 <br/>, 删除tagName中可能会有的'/'字符
				//自封闭的结点的type设置为NODE_START_TAG应该可以接受(否则要引入新的NODE_STARTCLOSE_TAG)
				int tagNamelen = wcslen(pNode->tagName);
				if(pNode->tagName[tagNamelen-1] == L'/')
					pNode->tagName[tagNamelen-1] = L'\0';
				//处理结点属性
				for(int i = 0; i < tagNamelen; i++)
				{
					if(pNode->tagName[i] == L' ' //第一个空格后面跟的是属性列表
						|| pNode->tagName[i] == L'=') //扩展支持这种格式: <tagName=value>, 等效于<tagName tagName=value>
					{
						char* props = (pNode->tagName[i] == L' ' ? s + i + 1 : s);
						pNode->text = duplicateStrUtill(props, L'>', true);
						int nodeTextLen = wcslen(pNode->text);
						if(pNode->text[nodeTextLen-1] == L'/') //去掉最后可能会有的'/'字符, 如这种情况: <img src="..." />
							pNode->text[nodeTextLen-1] = L'\0';
						pNode->tagName[i] = L'\0';
						parseNodeProps(pNode); //parse props
						break;
					}
				}
				pNode->tagType = getHtmlTagTypeFromName(pNode->tagName);
			}
			s = p + 1;
		}

		p++;
	}

	if(p > s)
	{
		//Add Text Node
		pNode = newHtmlNode();
		pNode->type = NODE_CONTENT;
		pNode->text = duplicateStr(s, -1);
	}

#ifdef _DEBUG
	//dumpHtmlNodes(); //just for test
#endif
}

unsigned int HtmlParser::getHtmlNodeCount()
{
	return (m_HtmlNodes.GetSize() / sizeof(HtmlNode));
}

HtmlNode* HtmlParser::getHtmlNodes()
{
	return (HtmlNode*) m_HtmlNodes.GetPtr();
}

void HtmlParser::freeHtmlNodes()
{
	HtmlNode* pNodes = getHtmlNodes();
	for(int i = 0, count = getHtmlNodeCount(); i < count; i++)
	{
		HtmlNode* pNode = pNodes + i;
		if(pNode->text)
			freeDuplicatedStr(pNode->text);

		if(pNode->props)
		{
			for(int propIndex = 0; propIndex < pNode->propCount; propIndex++)
			{
				HtmlNodeProp* prop = pNode->props + propIndex;
				if(prop->szName)  freeDuplicatedStr(prop->szName);
				if(prop->szValue) freeDuplicatedStr(prop->szValue);
			}
			MFreeMemory(pNode->props); //see: CMem::Alloc and CMem::Detach
		}

	}
	m_HtmlNodes.Empty();
}

//[virtual]
HtmlTagType HtmlParser::getHtmlTagTypeFromName(const char* szTagName)
{
	//todo: uses hashmap
	struct N2T { const char* name; HtmlTagType type; };
	static N2T n2tTable[] = 
	{
		{ L"A", TAG_A },
		{ L"FONT", TAG_FONT },
		{ L"IMG", TAG_IMG },
		{ L"P", TAG_P },
		{ L"DIV", TAG_DIV },
		{ L"SPAN", TAG_SPAN },
		{ L"BR", TAG_BR },
		{ L"B", TAG_B },
		{ L"I", TAG_I },
		{ L"HR", TAG_HR },
		{ L"COLOR", TAG_COLOR },
		{ L"BGCOLOR", TAG_BGCOLOR },

	};

	for(int i = 0, count = sizeof(n2tTable)/sizeof(n2tTable[0]); i < count; i++)
	{
		N2T* p = &n2tTable[i];
		if(wcsicmp(p->name, szTagName) == 0)
			return p->type;
	}

	return TAG_UNKNOWN;
}

void skipSpaceChars(char*& p)
{
	if(p)
	{
		while(isspace(*p)) p++;
	}
}

const char* nextUnqotedSpaceChar(const char* p)
{
	const char* r = getFirstUnquotedChar(p, L' ');
	if(!r)
		r = getFirstUnquotedChar(p, L'\t');
	return r;
}

const char* duplicateStrAndUnquote(const char* str, unsigned int nChar)
{
	if( nChar > 1 && (str[0] == L'\"' && str[nChar-1] == L'\"') || (str[0] == L'\'' && str[nChar-1] == L'\'') )
	{
		str++; nChar-=2;
	}
	return duplicateStr(str, nChar);
}

//[virtual]
void HtmlParser::parseNodeProps(HtmlNode* pNode)
{
	if(pNode == NULL || pNode->propCount > 0 || pNode->text == NULL)
		return;
	char* p = pNode->text;
	char *ps = NULL;
	CMem mem;

	bool inQuote1 = false, inQuote2 = false;
	char c;
	while(c = *p)
	{
		if(c == L'\"')
		{
			inQuote1 = !inQuote1;
		}
		else if(c == L'\'')
		{
			inQuote2 = !inQuote2;
		}

		if((!inQuote1 && !inQuote2) && (c == L' ' || c == L'\t' || c == L'='))
		{
			if(ps)
			{
				mem.AddPointer(duplicateStrAndUnquote(ps, p - ps));
				ps = NULL;
			}
			if(c == L'=')
				mem.AddPointer(NULL);
		}
		else
		{
			if(ps == NULL)
				ps = p;
		}

		p++;
	}

	if(ps)
		mem.AddPointer(duplicateStrAndUnquote(ps, p - ps));

	mem.AddPointer(NULL);
	mem.AddPointer(NULL);

	char** pp = (char**) mem.GetPtr();

	CMem props;
	for(int i = 0, n = mem.GetSize() / sizeof(char*) - 2; i < n; i++)
	{
		props.AddPointer(pp[i]); //prop name
		if(pp[i+1] == NULL)
		{
			props.AddPointer(pp[i+2]); //prop value
			i += 2;
		}
		else
			props.AddPointer(NULL); //prop vlalue
	}

	pNode->propCount = props.GetSize() / sizeof(char*) / 2;
	pNode->props = (HtmlNodeProp*) props.Detach();
}

const HtmlNodeProp* HtmlParser::getNodeProp(const HtmlNode* pNode, const char* szPropName)
{
	if(pNode == NULL || pNode->propCount <= 0)
		return NULL;

	for(int i = 0; i < pNode->propCount; i++)
	{
		HtmlNodeProp* prop = pNode->props + i;
		if(wcsicmp(prop->szName, szPropName) == 0)
			return prop;
	}
	return NULL;
}

const char* HtmlParser::getNodePropStringValue(const HtmlNode* pNode, const char* szPropName, const char* szDefaultValue /*= NULL*/)
{
	const HtmlNodeProp* pProp = getNodeProp(pNode, szPropName);
	if(pProp)
		return pProp->szValue;
	else
		return szDefaultValue;
}

int HtmlParser::getNodePropIntValue(const HtmlNode* pNode, const char* szPropName, int defaultValue /*= 0*/)
{
	const HtmlNodeProp* pProp = getNodeProp(pNode, szPropName);
	if(pProp && pProp->szValue)
		return _wtoi(pProp->szValue);
	else
		return defaultValue;
}

void HtmlParser::dumpHtmlNodes()
{
#ifdef _DEBUG
	HtmlNode* pNodes = getHtmlNodes();
	char buffer[256];
	printf("\n-------- dumpHtmlNodes() --------\n");
	for(int i = 0, count = getHtmlNodeCount(); i < count; i++)
	{
		HtmlNode* pNode = pNodes + i;
		switch(pNode->type)
		{
		case NODE_CONTENT:
			sprintf(buffer, "%2d) type: NODE_CONTENT", i);
			break;
		case NODE_START_TAG:
			sprintf(buffer, "%2d) type: NODE_START_TAG, tagName: %s (%d)", i, pNode->tagName, pNode->tagType);
			break;
		case NODE_CLOSE_TAG:
			sprintf(buffer, "%2d) type: NODE_CLOSE_TAG, tagName: %s", i, pNode->tagName);
			break;
		case NODE_UNKNOWN:
		default:
			sprintf(buffer, "%2d) type: NODE_UNKNOWN", i);
			break;
		}
		printf(buffer);
		if(pNode->text)
		{
			printf(", text: ");
			printf(pNode->text);
		}
		printf("\n");

		if(pNode->propCount > 0)
		{
			printf("    props: ");
			for(int i = 0; i < pNode->propCount; i++)
			{
				HtmlNodeProp* prop = pNode->props + i;
				if(prop->szValue)
				{
					printf(prop->szName);
					printf(" = ");
					printf(prop->szValue);
				}
				else
					printf(prop->szName);
				if(i < pNode->propCount - 1)
				{
					printf(", ");
				}
			}
			printf("\n");
		}
	}
	printf("-------- end of dumpHtmlNodes() --------\n");
#endif
}

//-----------------------------------------------------------------------------
// class MemBuffer

MemBuffer::MemBuffer(size_t nBufferSize) : m_pBuffer(NULL), m_nDataSize(0), m_nBufferSize(0)
{
	if(nBufferSize == (size_t)-1)
		nBufferSize = MEM_DEFAULT_BUFFER_SIZE;
	if(nBufferSize > 0)
		require(nBufferSize);
}

MemBuffer::~MemBuffer()
{
	clean();
}

void MemBuffer::clean()
{
	if(m_pBuffer) free(m_pBuffer);
	m_pBuffer = NULL;
	m_nDataSize = m_nBufferSize = 0;
}

void* MemBuffer::require(size_t size)
{
	if(size == 0 || (m_nBufferSize - m_nDataSize) >= size)
		return (m_pBuffer + m_nDataSize); //现有缓存区足够使用，不需要扩充缓存区

	//计算新的缓存区大小
	size_t newBufferSize;
	if(m_nBufferSize == 0)
	{
		newBufferSize = size; //缓存区初始大小
	}
	else
	{
		//扩充缓存区
		newBufferSize = (m_nBufferSize == 0 ? MEM_DEFAULT_BUFFER_SIZE : m_nBufferSize);
		do {
			newBufferSize <<= 1; //扩充一倍
		}while(newBufferSize - m_nDataSize < size);
	}

	//分配缓存区内存
	if(m_pBuffer == NULL)
	{
		m_pBuffer = malloc(newBufferSize);
		memset(m_pBuffer, 0, newBufferSize);
	}
	else
	{
		m_pBuffer = realloc(m_pBuffer, newBufferSize);
		memset(m_pBuffer + m_nBufferSize, 0, newBufferSize - m_nBufferSize);
	}

	m_nBufferSize = newBufferSize; //设置新的缓存区大小

	return (m_pBuffer + m_nDataSize); //返回
}

int MemBuffer::append(void* pData, size_t nSize)
{
	void* p = require(nSize);
	memcpy(p, pData, nSize);
	m_nDataSize += nSize;
	return (m_nDataSize - nSize);
}

//-----------------------------------------------------------------------------

//just for test
#ifdef _DEBUG
class TestHtmlParser
{
public:
	TestHtmlParser()
	{
		HtmlParser htmlParser;

		htmlParser.parseHtml(L"<link rel=\"next\" href=\"objects.html\">");
		htmlParser.parseHtml(L"...<p>---<a href=url>link</a>...");
		htmlParser.parseHtml(L"<p>---< a   href=url >link</a>");
		htmlParser.parseHtml(L"<p x=a y=b z = \"c <a href=url>\" >");
		htmlParser.parseHtml(L"<a x=0> <b y=1> <c z=ok w=false> - </c>");
	}
};
//TestHtmlParser testHtmlParser;
#endif
