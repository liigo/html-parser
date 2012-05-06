#include "HtmlParser.h"
#include <memory.h>
#include <string.h>
#include <ctype.h>

//HtmlParser类，用于解析HTML文本
//by liigo, @2010-2012

using namespace liigo;

const char* strnchr(const char* pStr, int len, char c)
{
	if(pStr == NULL || len <= 0)
		return NULL;
	const char *p = pStr;
	while(1)
	{
		if(*p == c) return p;
		p++;
		if((p - pStr) == len) break;
	}
	return NULL;
}

inline bool matchchar(char c1, char c2, bool bCaseSensitive)
{
	return bCaseSensitive ? (c1 == c2) : (tolower(c1) == tolower(c2));
}

const char* findFirstUnquotedChar(const char* pStr, char endchar)
{
	char c;
	const char* p = pStr;
	bool inQuote1 = false, inQuote2 = false; //'inQuote1', "inQuote2"
	while(c = *p)
	{
		if(c == '\'')
		{
			inQuote1 = !inQuote1;
		}
		else if(c == '\"')
		{
			inQuote2 = !inQuote2;
		}

		if(!inQuote1 && !inQuote2)
		{
			if(c == endchar) return p;
		}
		p++;
	}
	return NULL;
}

const char* findFirstUnquotedChars(const char* pStr, char* endchars, int nchars, bool bCaseSensitive)
{
	char c;
	const char* p = pStr;
	bool inQuote1 = false, inQuote2 = false; //'inQuote1', "inQuote2"
	while(c = *p)
	{
		if(c == '\'')
		{
			inQuote1 = !inQuote1;
		}
		else if(c == '\"')
		{
			inQuote2 = !inQuote2;
		}

		if(!inQuote1 && !inQuote2)
		{
			for(int i = 0; i < nchars; i++)
			{
				if(matchchar(c, endchars[i], bCaseSensitive))
					return p;
			}
		}
		p++;
	}
	return NULL;
}

// return true if pStr1 starts with pStr2. pStr1 not NULL, pStr2 not NULL, lenStr2 > 0.
static bool strStartWith(const char* pStr1, const char* pStr2, int lenStr2, bool bCaseSensitive)
{
	for(int i = 0; i < lenStr2; i++)
	{
		if(pStr1[i] == '\0') return false;
		if(!matchchar(pStr1[i], pStr2[i], bCaseSensitive))
			return false;
	}
	return true;
}

const char* findFirstUnquotedStr(const char* pSourceStr, const char* pDestStr, bool bCaseSensitive)
{
	if(pDestStr == NULL || *pDestStr == '\0') return NULL;

	char c;
	const char* p = pSourceStr;
	bool inQuote1 = false, inQuote2 = false; //'inQuote1', "inQuote2"
	int lenDestStr = strlen(pDestStr);
	while(c = *p)
	{
		if(c == '\'')
		{
			inQuote1 = !inQuote1;
		}
		else if(c == '\"')
		{
			inQuote2 = !inQuote2;
		}

		if(!inQuote1 && !inQuote2)
		{
			if(strStartWith(p, pDestStr, lenDestStr, bCaseSensitive))
				return p;
		}
		p++;
	}
	return NULL;
}

const char* findFirstStr(const char* pSourceStr, const char* pDestStr, bool bCaseSensitive)
{
	if(pDestStr == NULL || *pDestStr == '\0') return NULL;

	char c;
	const char* p = pSourceStr;
	int lenDestStr = strlen(pDestStr);
	while(c = *p)
	{
		if(strStartWith(p, pDestStr, lenDestStr, bCaseSensitive))
			return p;
		p++;
	}
	return NULL;
}

//nDest and nChar can by -1
size_t copyStr(char* pDest, size_t nDest, const char* pSrc, size_t nChar)
{
	if(pDest == NULL || nDest == 0)
		return 0;
	if(pSrc == NULL)
	{
		pDest[0] = '\0';
		return 0;
	}
	if(nChar == (size_t)-1)
		nChar = strlen(pSrc);
	if(nChar > nDest)
		nChar = nDest;
	memcpy(pDest, pSrc, nChar * sizeof(char));
	pDest[nChar] = '\0';
	return nChar;
}

int copyStrUtill(char* pDest, size_t nDest, const char* pSrc, char endchar, bool ignoreEndCharInQuoted)
{
	if(nDest == 0) return 0;
	pDest[0] = '\0';
	const char* pSearched = (ignoreEndCharInQuoted ? findFirstUnquotedChar(pSrc,endchar) : strchr(pSrc, endchar));
	if(pSearched <= pSrc) return 0;
	return copyStr(pDest, nDest, pSrc, pSearched - pSrc);
}

//nChar can be -1
char* duplicateStr(const char* pSrc, size_t nChar)
{
	if(nChar == (size_t)-1)
		nChar = strlen(pSrc);
	char* pNew = (char*) malloc( (nChar+1) * sizeof(char) );
	copyStr(pNew, -1, pSrc, nChar);
	return pNew;
}

char* duplicateStrUtill(const char* pSrc, char endchar, bool ignoreEndCharInQuoted)
{
	const char* pSearched = (ignoreEndCharInQuoted ? findFirstUnquotedChar(pSrc,endchar) : strchr(pSrc, endchar));;
	if(pSearched <= pSrc) return NULL;
	int n = pSearched - pSrc;
	return duplicateStr(pSrc, n);
}

void freeDuplicatedStr(char* p)
{
	if(p) free(p);
}

void skipSpaceChars(char*& p)
{
	if(p)
	{
		while(isspace(*p)) p++;
	}
}

inline const char* nextUnqotedSpaceChar(const char* p)
{
	return findFirstUnquotedChars(p, " \n\r\t\f", 5, true);
}

char* duplicateStrAndUnquote(const char* str, size_t nChar)
{
	if( nChar > 1 && (str[0] == '\"' && str[nChar-1] == '\"') || (str[0] == '\'' && str[nChar-1] == '\'') )
	{
		str++; nChar-=2;
	}
	return duplicateStr(str, nChar);
}

//-----------------------------------------------------------------------------

// TagName & TagType
struct N2T { const char* name; HtmlTagType type; };

static HtmlTagType identifyHtmlTagInTable(const char* szTagName, N2T* table, int count)
{
	for(int i = 0; i < count; i++)
	{
		N2T* p = table + i;
		if(stricmp(p->name, szTagName) == 0)
			return p->type;
	}
	return TAG_UNKNOWN;
}

//出于解析需要必须识别的HtmlTagType
static HtmlTagType identifyHtmlTag_Internal(const char* szTagName)
{
	static N2T n2tTable[] = 
	{
		{ "SCRIPT", TAG_SCRIPT },
		{ "STYLE",  TAG_STYLE  },
	};

	return identifyHtmlTagInTable(szTagName, n2tTable, sizeof(n2tTable)/sizeof(n2tTable[0]));
}

//[virtual]
HtmlTagType HtmlParser::onIdentifyHtmlTag(const char* szTagName, HtmlNodeType nodeType)
{
	//默认仅识别涉及HTML基本结构和信息的有限几个TAG
	//交给用户自行扩展以便识别更多或更少

	if(nodeType != NODE_START_TAG)
		return TAG_UNKNOWN;

	static N2T n2tTable[] = 
	{
		{ "A",      TAG_A      },
		{ "IMG",    TAG_IMG    },
		{ "META",   TAG_META   },
		{ "BODY",   TAG_BODY   },
		{ "TITLE",  TAG_TITLE  },
		{ "FRAME",  TAG_FRAME  },
		{ "IFRAME", TAG_IFRAME },
	};

	return identifyHtmlTagInTable(szTagName, n2tTable, sizeof(n2tTable)/sizeof(n2tTable[0]));
}

HtmlNode* HtmlParser::newHtmlNode()
{
	static char staticHtmlNodeTemplate[sizeof(HtmlNode)] = {0};
	size_t offset = m_HtmlNodes.appendData(staticHtmlNodeTemplate, sizeof(HtmlNode));
	HtmlNode* pNode = (HtmlNode*) m_HtmlNodes.getOffsetData(offset);
	return pNode;
}

static void setNodeAttributeText(HtmlNode* pNode, const char* pStart)
{
	while(isspace(*pStart)) pStart++;

	if(pStart[0] == '>') return; //no attribute text
	if(pStart[0] == '/' && pStart[1] == '>')
	{
		pNode->flags |= FLAG_SELF_CLOSING_TAG; //自封闭标签
		return; //no attribute text
	}

	char* attributeText = duplicateStrUtill(pStart, '>', true);
	if(attributeText)
	{
		int len = strlen(attributeText);
		if(attributeText[len-1] == '/') //去掉最后可能会有的'/'字符, 如这种情况: <img src="..." />
		{
			attributeText[len-1] = '\0';
			pNode->flags |= FLAG_SELF_CLOSING_TAG; //自封闭标签
		}
		pNode->text = attributeText;
	}
}

static const char* s_LF = "\n"; //Unix,Linux
static const char* s_CR = "\r"; //Mac
static const char* s_CRLF = "\r\n"; //Windows

// pNode != NULL, pStart != NULL, len > 0
static void setContentNodeText(HtmlNode* pNode, const char* pStart, int len)
{
	//由于空行文本比较常见，每个行首标签的前面都可能有一个空行文本
	//此处优化直接返回常量文本，以减少不必要的内存分配
	//freeHtmlNodes()里释放pNode->text时需做相应的处理
	if(len == 1 && pStart[0] == '\n') { pNode->text = (char*)s_LF; return; }

	if(pStart[0] == '\r')
	{
		if(len == 1) { pNode->text = (char*)s_CR; return; }
		if(len == 2 && pStart[1] == '\n') { pNode->text = (char*)s_CRLF; return; }
	}
	pNode->text = duplicateStr(pStart, len);
}

void HtmlParser::parseHtml(const char* szHtml, bool parseAttributes)
{
	freeHtmlNodes();
	if(szHtml == NULL || *szHtml == '\0') return;

	char* p = (char*) szHtml;
	char* s = (char*) szHtml;
	HtmlNode* pNode = NULL;
	char c;
	bool bInQuote1  = false; //between ' and '
	bool bInQuote2  = false; //between " and "
	bool bInScript  = false; //between <scripte> and </script>
	bool bInStyle   = false; //between <style> and </style>
	bool bInsideTag = false; //between < and >

	while( c = *p )
	{
		if(bInsideTag)
		{
			//在 < 和 > 之间，跳过单引号或双引号内的任何文本（包括<和>）
			if(c == '\'')
			{
				bInQuote1 = !bInQuote1;
				p++; continue;
			}
			else if(c == '\"')
			{
				bInQuote2 = !bInQuote2;
				p++; continue;
			}
			if(bInQuote1 || bInQuote2)
			{
				p++; continue;
			}
		}

		if(bInScript)
		{
			//跳过<script>和</script>之间的任何文本
			const char* pEndScript = findFirstStr(p, "</script>", false);
			if(pEndScript)
			{
				bInScript = false;
				p = (char*)pEndScript;
				c = *p;
			}
			else
				goto onerror; //error: no </script>
		}
		else if(bInStyle)
		{
			//跳过<style>和</style>之间的任何文本
			const char* pEndStyle = findFirstStr(p, "</style>", false);
			if(pEndStyle)
			{
				bInStyle = false;
				p = (char*)pEndStyle;
				c = *p;
			}
			else
				goto onerror; //error: no </style>
		}

		if(c == '<') // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		{
			if(p > s)
			{
				//Add Content Node
				pNode = newHtmlNode();
				pNode->type = NODE_CONTENT;
				setContentNodeText(pNode, s, p - s);
				if(onNodeReady(pNode) == false) goto onuserend;
			}

			//处理HTML注释或CDATA
			if(p[1] == '!')
			{
				if(p[2] == '-' && p[3] == '-')
				{
					//注释: <!-- ... -->
					const char* pEndRemarks = findFirstStr(p + 4, "-->", true);
					if(pEndRemarks)
					{
						//Add Remarks Node
						pNode = newHtmlNode();
						pNode->type = NODE_REMARKS;
						pNode->text = duplicateStr(p + 4, pEndRemarks - (p + 4));
						if(onNodeReady(pNode) == false) goto onuserend;
						s = p = (char*)pEndRemarks + 3;
						bInsideTag = bInQuote1 = bInQuote2 = false;
						continue;
					}
					//else: no -->, not very bad, try continue parsing
				}
				else if(p[2] == '[' && strStartWith(p+3, "CDATA[", 5, false))
				{
					//CDATA: <![CDATA[ ... ]]>
					const char* pEndCData = findFirstStr(p + 9, "]]>", true);
					if(pEndCData)
					{
						//Add Content Node with CDATA flag
						pNode = newHtmlNode();
						pNode->type = NODE_CONTENT;
						pNode->text = duplicateStr(p + 9, pEndCData - (p + 9));
						pNode->flags |= FLAG_CDATA_BLOCK; //CDATA标记, used by outputHtml()
						if(onNodeReady(pNode) == false) goto onuserend;
						s = p = (char*)pEndCData + 3;
						bInsideTag = bInQuote1 = bInQuote2 = false;
						continue;
					}
					//else: no ]]>, not very bad, try contiune parsing
				}
			}


			s = p + 1;
			bInsideTag = true; bInQuote1 = bInQuote2 = false;
		}
		else if(c == '>') // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		{
			if(p > s)
			{
				//创建新节点(HtmlNode)，得到节点类型(NodeType)和名称(TagName)
				pNode = newHtmlNode();
				while(isspace(*s)) s++;
				pNode->type = (*s == '/' ? NODE_END_TAG : NODE_START_TAG);
				if(*s == '/') s++;
				//这里得到的tagName可能包含一部分属性文本，需在下面修正
				//tagName也可能是这种情况 a href="///////////////// （即缓冲区被填满,最后一个/是属性值中的字符）
				copyStrUtill(pNode->tagName, MAX_HTML_TAG_LENGTH, s, '>', true);
				int tagNameLen = strlen(pNode->tagName);
				if(tagNameLen < MAX_HTML_TAG_LENGTH && pNode->tagName[tagNameLen-1] == '/')
				{
					//处理自封闭的标签, 如<br/>, 删除tagName中可能会有的'/'字符
					//自封闭的结点的type设置为NODE_START_TAG应该可以接受(否则要引入新的节点类型NODE_STARTCLOSE_TAG)
					pNode->flags |= FLAG_SELF_CLOSING_TAG; //used by outputHtml()
					pNode->tagName[tagNameLen-1] = '\0';
					tagNameLen--;
				}
				//修正节点名称，提取属性文本(存入pNode->text)
				int i;
				for(i = 0; i < tagNameLen; i++)
				{
					if(isspace(pNode->tagName[i])    //第一个空白字符后面跟的是属性文本
						|| pNode->tagName[i] == '=') //扩展支持这种格式: <tagName=value>, 等效于<tagName tagName=value>
					{
						char* attributes = (pNode->tagName[i] == '=' ? s : s + i + 1);
						setNodeAttributeText(pNode, attributes); //此处也处理了自封闭标签
						pNode->tagName[i] = '\0';
						break;
					}
				}
				if(i == tagNameLen)
				{
					//parse error, tag name is too long, MAX_HTML_TAG_LENGTH is too small
				}

				//识别节点类型(HtmlTagType) - 内部
				pNode->tagType = identifyHtmlTag_Internal(pNode->tagName); //内部识别SCRIPT,STYLE
				if(pNode->tagType == TAG_STYLE)
					bInStyle = (pNode->type == NODE_START_TAG);
				if(pNode->tagType == TAG_SCRIPT)
					bInScript = (pNode->type == NODE_START_TAG);
				//识别节点类型(HtmlTagType) - 用户
				if(pNode->tagType == TAG_UNKNOWN)
					pNode->tagType = onIdentifyHtmlTag(pNode->tagName, pNode->type);

				//解析节点属性
				if(pNode->type == NODE_START_TAG && parseAttributes && pNode->text)
					onParseAttributes(pNode);

				if(onNodeReady(pNode) == false) goto onuserend;
			}

			s = p + 1;
			bInsideTag = bInQuote1 = bInQuote2 = false;
		}

		p++;
	}

	if(p > s)
	{
		//Add Content Node
		pNode = newHtmlNode();
		pNode->type = NODE_CONTENT;
		setContentNodeText(pNode, s, strlen(s));
		if(onNodeReady(pNode) == false) goto onuserend;
	}

	goto dumpnodes;
	return;

onerror:
	pNode = newHtmlNode();
	pNode->type = NODE_CONTENT;
	pNode->text = duplicateStr(s, -1);
	goto dumpnodes;
	return;

onuserend:
	goto dumpnodes;
	return;

dumpnodes:
#ifdef _DEBUG
	dumpHtmlNodes(); //just for test
#endif
}

size_t HtmlParser::getHtmlNodeCount()
{
	return (size_t)(m_HtmlNodes.getDataSize() / sizeof(HtmlNode));
}

HtmlNode* HtmlParser::getHtmlNode(size_t index)
{
	return (HtmlNode*)m_HtmlNodes.getData() + index;
}

void HtmlParser::freeHtmlNodes()
{
	for(int i = 0, count = getHtmlNodeCount(); i < count; i++)
	{
		HtmlNode* pNode = getHtmlNode(i);
		if(pNode->text)
		{
			if(pNode->type != NODE_CONTENT)
				freeDuplicatedStr(pNode->text);
			else if(pNode->text != s_CRLF && pNode->text != s_LF && pNode->text != s_CR) //see setContentNodeText()
				freeDuplicatedStr(pNode->text);
		}

		if(pNode->attributes)
		{
			for(int attributeIndex = 0; attributeIndex < pNode->attributeCount; attributeIndex++)
			{
				HtmlAttribute* attribute = pNode->attributes + attributeIndex;
				if(attribute->szName)  freeDuplicatedStr(attribute->szName);
				if(attribute->szValue) freeDuplicatedStr(attribute->szValue);
			}
			free(pNode->attributes); //see: HtmlParser.parseAttributes(), MemBuffer.detach()
		}
	}
	m_HtmlNodes.clean();
}

//[virtual]
void HtmlParser::onParseAttributes(HtmlNode* pNode)
{
	if(pNode->tagType != NODE_UNKNOWN)
		parseAttributes(pNode);
}

void HtmlParser::parseAttributes(HtmlNode* pNode)
{
	if(pNode == NULL || pNode->attributeCount > 0 || pNode->text == NULL)
		return;
	if(pNode->tagName[0] == '!' && stricmp(pNode->tagName+1, "DOCTYPE") == 0)
		return; //don't parse <!DOCTYPE ...>'s text: not name=value syntax

	char* p = pNode->text;
	char* ps = NULL;
	MemBuffer mem;

	bool inQuote1 = false, inQuote2 = false;
	char c;
	while(c = *p)
	{
		if(c == '\'')
			inQuote1 = !inQuote1;
		else if(c == '\"')
			inQuote2 = !inQuote2;

		bool notInQuote = (!inQuote1 && !inQuote2);

		if(notInQuote && (c == '\"' || c == '\'') && !isspace(p[1]))
		{
			//处理属性值引号后面没有空白分隔符的情况：a="v1"b=v2 （这种错误写法不少见，应兼容之）
			if(ps)
			{
				mem.appendPointer(duplicateStrAndUnquote(ps, p - ps + 1));
				ps = NULL;
			}
			p++;
			continue;
		}

		if(notInQuote && (c == '=' || isspace(c)))
		{
			if(ps)
			{
				mem.appendPointer(duplicateStrAndUnquote(ps, p - ps));
				ps = NULL;
			}
			if(c == '=')
				mem.appendPointer(NULL);
		}
		else
		{
			if(ps == NULL)
				ps = p;
		}

		p++;
	}

	if(ps)
		mem.appendPointer(duplicateStrAndUnquote(ps, p - ps));

	mem.appendPointer(NULL);
	mem.appendPointer(NULL);

	char** pp = (char**) mem.getData();

	MemBuffer attributes;
	for(int i = 0, n = mem.getDataSize() / sizeof(char*) - 2; i < n; i++)
	{
		attributes.appendPointer(pp[i]); //attribute name
		if(pp[i+1] == NULL)
		{
			attributes.appendPointer(pp[i+2]); //attribute value
			i += 2;
		}
		else
			attributes.appendPointer(NULL); //attribute vlalue
	}

	pNode->attributeCount = attributes.getDataSize() / sizeof(char*) / 2;
	pNode->attributes = (HtmlAttribute*) attributes.getData();
	attributes.detach();
}

const HtmlAttribute* HtmlParser::getAttribute(const HtmlNode* pNode, size_t index)
{
	return pNode->attributes + index;
}

const HtmlAttribute* HtmlParser::getAttribute(const HtmlNode* pNode, const char* szAttributeName)
{
	if(pNode == NULL || pNode->attributeCount <= 0)
		return NULL;

	for(int i = 0; i < pNode->attributeCount; i++)
	{
		HtmlAttribute* attribute = pNode->attributes + i;
		if(stricmp(attribute->szName, szAttributeName) == 0)
			return attribute;
	}
	return NULL;
}

const char* HtmlParser::getAttributeStringValue(const HtmlNode* pNode, const char* szAttributeName, const char* szDefaultValue /*= NULL*/)
{
	const HtmlAttribute* pAttribute = getAttribute(pNode, szAttributeName);
	if(pAttribute)
		return pAttribute->szValue;
	else
		return szDefaultValue;
}

int HtmlParser::getAttributeIntValue(const HtmlNode* pNode, const char* szAttributeName, int defaultValue /*= 0*/)
{
	const HtmlAttribute* pAttribute = getAttribute(pNode, szAttributeName);
	if(pAttribute && pAttribute->szValue)
		return atoi(pAttribute->szValue);
	else
		return defaultValue;
}

void HtmlParser::dumpHtmlNodes(FILE* f)
{
	char buffer[256] = {0};
	fprintf(f, "\r\n-------- begin HtmlParser.dumpHtmlNodes() --------\r\n");
	for(int i = 0, count = getHtmlNodeCount(); i < count; i++)
	{
		HtmlNode* pNode = getHtmlNode(i);
		switch(pNode->type)
		{
		case NODE_CONTENT:
			sprintf(buffer, "%2d) type: NODE_CONTENT", i);
			break;
		case NODE_START_TAG:
			sprintf(buffer, "%2d) type: NODE_START_TAG, tagName: %s (%d)", i, pNode->tagName, pNode->tagType);
			break;
		case NODE_END_TAG:
			sprintf(buffer, "%2d) type: NODE_END_TAG, tagName: %s (%d)", i, pNode->tagName, pNode->tagType);
			break;
		case NODE_REMARKS:
			sprintf(buffer, "%2d) type: NODE_REMARKS", i);
			break;
		case NODE_UNKNOWN:
		default:
			sprintf(buffer, "%2d) type: NODE_UNKNOWN", i);
			break;
		}
		fprintf(f, "%s", buffer);
		if(pNode->text)
			fprintf(f, ", text: %s", pNode->text);
		if(pNode->flags & FLAG_SELF_CLOSING_TAG)
			fprintf(f, ", flags: />"); //自封闭
		if(pNode->flags & FLAG_CDATA_BLOCK)
			fprintf(f, ", flags: CDATA"); //CDATA
		fprintf(f, "\r\n");

		if(pNode->attributeCount > 0)
		{
			fprintf(f, "    attributes: ");
			for(int i = 0; i < pNode->attributeCount; i++)
			{
				HtmlAttribute* attribute = pNode->attributes + i;
				if(attribute->szValue)
					fprintf(f, "%s = \"%s\"", attribute->szName, attribute->szValue);
				else
					fprintf(f, "%s", attribute->szName);
				if(i < pNode->attributeCount - 1)
				{
					fprintf(f, ", ");
				}
			}
			fprintf(f, "\r\n");
		}
	}
	fprintf(f, "-------- end of HtmlParser.dumpHtmlNodes() --------\r\n");
}

void HtmlParser::outputHtml(MemBuffer& buffer, bool keepBufferData)
{
	if(!keepBufferData) buffer.empty();
	for(int nodeIndex = 0, count = getHtmlNodeCount(); nodeIndex < count; nodeIndex++)
	{
		HtmlNode* pNode = getHtmlNode(nodeIndex);
		outputHtmlNode(buffer, pNode);
	}
}

void HtmlParser::outputHtmlNode(MemBuffer& buffer, const HtmlNode* pNode)
{
	int attributeIndex = 0;

	if(pNode == NULL)
		return;

	switch(pNode->type)
	{
	case NODE_CONTENT:
		if(pNode->flags & FLAG_CDATA_BLOCK)
		{
			buffer.appendText("<![CDATA[", 9);
			buffer.appendText(pNode->text);
			buffer.appendText("]]>", 3);
		}
		else
			buffer.appendText(pNode->text);
		break;
	case NODE_START_TAG:
		buffer.appendChar('<');
		buffer.appendText(pNode->tagName);
		if(pNode->attributeCount > 0)
			buffer.appendChar(' ');
		for(attributeIndex = 0; attributeIndex < pNode->attributeCount; attributeIndex++)
		{
			const HtmlAttribute* pAttribute = getAttribute(pNode, attributeIndex);
			buffer.appendText(pAttribute->szName);
			if(pAttribute->szValue)
			{
				bool hasQuoteChar = (strchr(pAttribute->szValue, '\"') != NULL);
				buffer.appendChar('=');
				buffer.appendChar(hasQuoteChar ? '\'' : '\"');
				buffer.appendText(pAttribute->szValue);
				buffer.appendChar(hasQuoteChar ? '\'' : '\"');
			}
			if(attributeIndex < pNode->attributeCount - 1)
				buffer.appendChar(' ');
		}
		if(pNode->attributeCount == 0 && pNode->text) //比如 <!DOCTYPE ...>
		{
			buffer.appendChar(' ');
			buffer.appendText(pNode->text);
		}
		if(pNode->flags & FLAG_SELF_CLOSING_TAG)
			buffer.appendChar('/'); //自封闭
		buffer.appendChar('>');
		break;
	case NODE_END_TAG:
		buffer.appendText("</");
		buffer.appendText(pNode->tagName);
		buffer.appendChar('>');
		break;
	case NODE_REMARKS:
		buffer.appendText("<!--");
		buffer.appendText(pNode->text);
		buffer.appendText("-->");
		break;
	case NODE_UNKNOWN:
	default:
		fprintf(stderr, "HtmlParser.outputHtmlNode(): NODE_UNKNOWN\n");
		break;
	} //end switch
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

MemBuffer::MemBuffer(const MemBuffer& other) : m_pBuffer(NULL), m_nDataSize(0), m_nBufferSize(0)
{
	*this = other; // invoke operator=()
}

MemBuffer::~MemBuffer()
{
	clean();
}

const MemBuffer& MemBuffer::operator= (const MemBuffer& other)
{
	if(&other != this)
	{
		resetDataSize(0);
		appendData(other.getData(), other.getDataSize());
	}
	return *this;
}

void MemBuffer::clean()
{
	if(m_pBuffer) free(m_pBuffer);
	m_pBuffer = NULL;
	m_nDataSize = m_nBufferSize = 0;
}

void* MemBuffer::detach(bool bShrink)
{
	if(bShrink) shrink();
	void* pReturn = m_pBuffer;
	//数据长度为0时返回NULL,内部释放m_pBuffer
	if(m_pBuffer && m_nDataSize == 0)
	{
		free(m_pBuffer);
		pReturn = NULL;
	}
	m_pBuffer = NULL;
	m_nDataSize = m_nBufferSize = 0;
	return pReturn;
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
			newBufferSize <<= 1; //每次扩充一倍
		}while(newBufferSize - m_nDataSize < size);
	}

	//分配缓存区内存
	if(m_pBuffer == NULL)
	{
		m_pBuffer = (unsigned char*) malloc(newBufferSize);
		memset(m_pBuffer, 0, newBufferSize);
	}
	else
	{
		m_pBuffer = (unsigned char*) realloc(m_pBuffer, newBufferSize);
		memset(m_pBuffer + m_nBufferSize, 0, newBufferSize - m_nBufferSize);
	}

	m_nBufferSize = newBufferSize; //设置新的缓存区大小

	return (m_pBuffer + m_nDataSize); //返回
}

void MemBuffer::shrink()
{
	if(m_pBuffer == NULL || m_nBufferSize == m_nDataSize)
		return;
	//assert(m_nBufferSize > m_nDataSize);
	m_nBufferSize = (m_nDataSize > 0 ? m_nDataSize : MEM_DEFAULT_BUFFER_SIZE);
	m_pBuffer = (unsigned char*) realloc(m_pBuffer, m_nBufferSize);
}

size_t MemBuffer::appendData(const void* pData, size_t nSize)
{
	if(pData == NULL) return m_nDataSize;
	void* p = require(nSize);
	memcpy(p, pData, nSize);
	m_nDataSize += nSize;
	return (m_nDataSize - nSize);
}

void MemBuffer::resetDataSize(size_t size)
{
	size_t oldDataSize = m_nDataSize;

	if(size <= m_nBufferSize)
	{
		m_nDataSize = size;
	}
	else
	{
		m_nDataSize = 0;
		require(size);
		m_nDataSize = size;
	}

	if(m_nDataSize < oldDataSize)
	{
		//如果数据长度被压缩，把被裁掉的部分数据清零
		memset(m_pBuffer + m_nDataSize, 0, oldDataSize - m_nDataSize);
	}
}

void MemBuffer::exchange(MemBuffer& other)
{
	unsigned char* pBuffer = m_pBuffer;
	size_t nBufferSize = m_nBufferSize;
	size_t nDataSize = m_nDataSize;

	m_pBuffer = other.m_pBuffer;
	m_nBufferSize = other.m_nBufferSize;
	m_nDataSize = other.m_nDataSize;

	other.m_pBuffer = pBuffer;
	other.m_nBufferSize = nBufferSize;
	other.m_nDataSize = nDataSize;
}

size_t MemBuffer::appendText(const char* szText, size_t len, bool appendZeroChar)
{
	if(szText == NULL)
		return m_nDataSize;
	if(len == (size_t)-1)
		len = strlen(szText);
	size_t offset = appendData(szText, len);
	if(appendZeroChar)
		appendChar('\0');
	return offset;
}

size_t MemBuffer::appendZeroBytes(int count)
{
	if(count > 0)
	{
		resetDataSize(m_nDataSize + count);
		return m_nDataSize - count;
	}
	else
	{
		return m_nDataSize;
	}
}

bool MemBuffer::loadFromFile(const char* szFileName, bool keepExistData, bool appendZeroChar, size_t* pReadBytes)
{
	if(!keepExistData) empty();
	if(pReadBytes) *pReadBytes = 0;
	if(szFileName == NULL) return false;
	FILE* pfile = fopen(szFileName, "rb");
	if(pfile)
	{
		fseek(pfile, 0, SEEK_END);
		long filelen = ftell(pfile);
		fseek(pfile, 0, SEEK_SET);
		size_t n = 0;
		if(filelen > 0)
		{
			size_t oldDataSize = getDataSize();
			resetDataSize(oldDataSize + filelen);
			n = fread(getOffsetData(oldDataSize), 1, filelen, pfile);
			resetDataSize(oldDataSize + n);
		}
		if(appendZeroChar) appendChar('\0');
		if(pReadBytes) *pReadBytes = n;

		fclose(pfile);
		return (n == (size_t)filelen); // n != filelen 的情况下，返回false，同时返回了不完整的数据。不确认这种处理方法好不好。
	}
	
	return false;
}

bool MemBuffer::saveToFile(const char* szFileName, const void* pBOM, size_t bomLen)
{
	FILE* pfile = fopen(szFileName, "wb+");
	if(pfile)
	{
		if(pBOM)
			fwrite(pBOM, 1, bomLen, pfile);
		if(getDataSize() > 0)
			fwrite(getData(), 1, getDataSize(), pfile);
		fclose(pfile);
	}
	return false;
}


//-----------------------------------------------------------------------------

