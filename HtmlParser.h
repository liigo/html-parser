#ifndef __HtmlParser_H__
#define __HtmlParser_H__

#include <stddef.h>

//HtmlParser类，用于解析HTML文本
//by liigo, @2010-2012

//仅内部使用的内存缓存类
#define MEM_DEFAULT_BUFFER_SIZE  256
class MemBuffer
{
public:
	//指定缓存区初始大小(字节数), 为-1表示使用默认初始大小(MEM_DEFAULT_BUFFER_SIZE)
	MemBuffer(size_t nBufferSize = -1); //nBufferSize
	~MemBuffer();
public:
	//向缓存区复制数据，写入现有数据的末尾，必要时会自动扩充缓存区
	//返回新写入的数据在缓存区中的偏移量
	int append(void* pData, size_t nSize);
	//取数据首地址(在数据长度为0时返回NULL)，此地址非NULL时即是缓存区首地址
	void* getData() { return (m_nDataSize == 0 ? NULL : m_pBuffer); }
	//取数据长度
	size_t getSize() { return m_nDataSize; } 
	void reset() { m_nDataSize = 0; } //重置缓存区，清除原有内容
	void clean(); //清理缓存区，释放内存
private:
	void* require(size_t size);
private:
	unsigned char* m_pBuffer;
	size_t m_nDataSize, m_nBufferSize;
}

enum HtmlNodeType
{
	NODE_UNKNOWN = 0,
	NODE_START_TAG,
	NODE_CLOSE_TAG,
	NODE_CONTENT,
};

enum HtmlTagType
{
	TAG_UNKNOWN = 0,
	TAG_A, TAG_DIV, TAG_FONT, TAG_IMG, TAG_P, TAG_SPAN, TAG_BR, TAG_B, TAG_I, TAG_HR, 
	TAG_COLOR, TAG_BGCOLOR, //非标准HTML标签, 可以这样使用: <color=red>, 等效于 <color color=red>
};

struct HtmlNodeProp
{
	char* szName;
	char* szValue;
};

#define MAX_HTML_TAG_LENGTH  15 //节点名称的最大字符长度

struct HtmlNode
{
	HtmlNodeType type;
	HtmlTagType  tagType;
	char tagName[MAX_HTML_TAG_LENGTH+1];
	char* text;
	int propCount;
	HtmlNodeProp* props;
	void* pUser; //user customized, default to NULL
};


class HtmlParser
{
public:
	HtmlParser() {}
	~HtmlParser() { freeHtmlNodes(); }

public:
	//html
	void parseHtml(const char* szHtml);

	//const char* getHtml() const { return m_html.GetText(); } //继续考虑是否提供此功能

	//nodes
	unsigned int getHtmlNodeCount();
	HtmlNode* getHtmlNodes();
	//props
	const HtmlNodeProp* getNodeProp(const HtmlNode* pNode, const char* szPropName);
	const char* getNodePropStringValue(const HtmlNode* pNode, const char* szPropName, const char* szDefaultValue = NULL);
	int getNodePropIntValue(const HtmlNode* pNode, const char* szPropName, int defaultValue = 0);

protected:
	//允许子类覆盖, 以便识别更多结点(提高解析质量), 或者识别更少结点(提高解析速度)
	virtual HtmlTagType getHtmlTagTypeFromName(const char* szTagName);
	//允许子类覆盖, 以便更好的解析节点属性, 或者干脆不解析节点属性(提高解析速度)
	virtual void parseNodeProps(HtmlNode* pNode);

private:
	HtmlNode* newHtmlNode();
	void freeHtmlNodes();
	void dumpHtmlNodes();

private:
	CMem m_HtmlNodes;
};

//一些文本处理函数
char* duplicateStr(const char* pSrc, unsigned int nChar);
void freeDuplicatedStr(char* p);
unsigned int copyStr(char* pDest, unsigned int nDest, const char* pSrc, unsigned int nChar);


#endif //__HtmlParser_H__