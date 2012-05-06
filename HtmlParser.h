#ifndef __HtmlParser_H__
#define __HtmlParser_H__

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

// HtmlParser类，用于解析HTML文本
// http://blog.csdn.net/liigo/article/details/6153829 (重要)
// by liigo, @2010-2012

namespace liigo
{

//MemBuffer: 内存缓冲区类，管理缓存区中连续存储的数据
#define MEM_DEFAULT_BUFFER_SIZE  256
class MemBuffer
{
public:
	//nBufferSize指定缓存区初始大小(字节数), 为-1表示使用默认初始大小(MEM_DEFAULT_BUFFER_SIZE)
	//nBufferSize为0时暂不分配缓存区内存，延迟到下一次写入数据时再分配
	MemBuffer(size_t nBufferSize = -1);
	MemBuffer(const MemBuffer& other); //从other对象复制数据，缓存区是自行分配的
	virtual ~MemBuffer(); //析构时清理缓存区释放内存，除非已经detach()
	const MemBuffer& operator= (const MemBuffer& other); //清空数据后再把other内的数据复制进来

public:
	//向缓存区复制数据块，写入现有数据的末尾，必要时会自动扩充缓存区
	//返回新写入的数据块首地址在缓存区中的偏移量
	size_t appendData(const void* pData, size_t nSize);
	//取数据首地址(在数据长度为0时返回NULL)
	//在appendXXX()或resetSize()或shrink()或exchange()或operator=调用之后可能会导致数据首地址发生改变
	inline void* getData() const { return (m_nDataSize == 0 ? NULL : m_pBuffer); }
	//取指定偏移处数据地址，偏移offset应小于getDataSize()，否则不保证返回的地址有效
	inline void* getOffsetData(int offset) const { return (m_nDataSize == 0 ? NULL : ((unsigned char*)m_pBuffer + offset)); }
	//取数据长度
	inline size_t getDataSize() const { return m_nDataSize; }
	//重置数据长度，新长度可以为任意值，必要时会自动扩充缓存区，新增加的数据均为0字节值
	void resetDataSize(size_t size = 0);
	//清空数据，等效于resetDataSize(0)
	inline void empty() { resetDataSize(0); }
	//收缩缓存区，避免长时间占用不再使用的内存，但缓存区中的已有数据仍然完整保留
	void shrink();
	//清理缓存区，释放内存
	void clean();
	//放弃管理缓存区和其中的数据，用户应自行负责用free()释放detach()后的数据:
	//返回数据首地址，数据长度为detach()前getDataSize()返回的长度
	//detach()时指定参数bShrink为true可有效避免浪费未经使用的缓存区内存
	void* detach(bool bShrink = true);
	//交换两个对象(this & other)各自管理的所有内容（包括数据和缓存区）
	void exchange(MemBuffer& other);

	//添加基本类型数据
	inline size_t appendInt(int i) { return appendData(&i, sizeof(i)); }
	inline size_t appendChar(char c) { return appendData(&c, sizeof(c)); }
	//把指针p本身的值（而非p指向的数据）添加到缓存区
	inline size_t appendPointer(const void* p) { return appendData(&p, sizeof(p)); }
	//把文本内容添加到缓存区, len为写入的字节数（-1表示strlen(szText)），appendZeroChar表示是否额外添加'\0'
	size_t appendText(const char* szText, size_t len = -1, bool appendZeroChar = false);
	//把指定数量的0字节值添加到缓存区
	size_t appendZeroBytes(int count);

	//读取文件全部内容，如果keepExistData=true将保留缓存区原有数据，否则将清除原有数据
	//参数appendZeroChar表示是否额外添加字符'\0'，参数pReadBytesr如果非NULL将写入从文件中读取的字节数
	//在磁盘读写出错未完整读取文件内容的情况下，将返回false，但已经读取的部分数据仍然保留，pReadBytesr中会写入读取的字节数
	bool loadFromFile(const char* szFileName, bool keepExistData = false, bool appendZeroChar = false, size_t* pReadBytes = NULL);
	//将数据保存到文件，后两个参数(pBOM,bomLen)为欲写到文件头部的BOM(Byte Order Mark)
	//如果文件已经存在，将直接覆盖掉原有文件内容
	bool saveToFile(const char* szFileName, const void* pBOM = NULL, size_t bomLen = 0);

private:
	//要求缓存区中至少留出长度为size的未使用空间
	//返回未使用空间的首地址，即现有数据的末尾
	void* require(size_t size);

private:
	unsigned char* m_pBuffer; //缓存区首地址
	size_t m_nDataSize, m_nBufferSize; //数据长度，缓存区长度
};

enum HtmlNodeType
{
	NODE_UNKNOWN = 0,
	NODE_START_TAG, //开始标签，如 <a href="liigo.com"> 或 <br/>
	NODE_END_TAG,   //结束标签，如 </a>
	NODE_CONTENT,   //内容: 介于开始标签或结束标签之间的普通文本
	NODE_REMARKS,   //注释: <!-- -->
};

enum HtmlTagType
{
	TAG_UNKNOWN = 0,
	TAG_SCRIPT, TAG_STYLE, //出于解析需要必须识别,内部特别处理
	//以下按标签字母顺序排列, 来源：http://www.w3school.com.cn/tags/
	//此处仅定义类型值，不代表解析器可识别它，参见HtmlParser.onIdentifyHtmlTag()
	TAG_A, TAG_ABBR, TAG_ACRONYM, TAG_ADDRESS, TAG_APPLET, TAG_AREA,
	TAG_B, TAG_BASE,TAG_BASEFONT, TAG_BDO, TAG_BIG, TAG_BLOCKQUOTE, TAG_BODY, TAG_BR, TAG_BUTTON, 
	TAG_CAPTION, TAG_CENTER, TAG_CITE, TAG_CODE, TAG_COL, TAG_COLGROUP, 
	TAG_DD, TAG_DEL, TAG_DFN, TAG_DIR, TAG_DIV, TAG_DL, TAG_DT, TAG_EM, 
	TAG_FONT, TAG_FORM, TAG_FRAME, TAG_FRAMESET, 
	TAG_HEAD, TAG_H1, TAG_H2, TAG_H3, TAG_H4, H5, H6, TAG_HR, TAG_HTML, 
	TAG_I, TAG_IFRAME, TAG_IMG, TAG_INPUT, TAG_INS, TAG_KBD, 
	TAG_LABEL, TAG_LEGEND, TAG_LI, TAG_LINK, TAG_MAP, TAG_MENU, TAG_META, TAG_NOFRAMES, TAG_NOSCRIPT, 
	TAG_OBJECT, TAG_OL, TAG_OPTGROUP, TAG_OPTION, TAG_P, TAG_PARAM, TAG_PRE, TAG_Q, 
	TAG_S, TAG_SAMP, TAG_SELECT, TAG_SMALL, TAG_SPAN, TAG_STRIKE, TAG_STRONG, TAG_SUB, TAG_SUP, 
	TAG_TABLE, TAG_TBODY, TAG_TD, TAG_TEXTAREA, TAG_TFOOT, TAG_TH, TAG_THEAD, TAG_TITLE, TAG_TR, TAG_TT, 
	TAG_U, TAG_UL, TAG_VAR, 
	_TAG_USER_TAG_, //用户定义的其他标签类型值应大于_TAG_USER_TAG_，以确保不与上面定义的常量值重复
};

enum HtmlNodeFlag
{
	FLAG_SELF_CLOSING_TAG = 1 << 0, //自封闭标签: <br/>
	FLAG_CDATA_BLOCK      = 1 << 1, //是CDATA区块
};

struct HtmlAttribute
{
	char* szName;
	char* szValue;
};

#define MAX_HTML_TAG_LENGTH  15 //节点名称的最大字符长度,超出将被截断

struct HtmlNode
{
	HtmlNodeType type;    //节点类型
	HtmlTagType  tagType; //标签类型（仅当节点类型为NODE_START_TAG或NODE_END_TAG时有意义）
	char tagName[MAX_HTML_TAG_LENGTH+1]; //标签名称（如<A href="...">对应的标签名称为"A"）
	char* text; //文本，视标签类型(tagType)不同意义也不同，可能为NULL
				//如果type==NODE_START_TAG, text指向标签名称后面的属性文本
				//如果type==NODE_CONTENT或NODE_REMARKS, text指向内容或注释文本
	int attributeCount;        //属性个数（仅当属性被解析后有效，参见HtmlParser.parseAttributes()，下同）
	HtmlAttribute* attributes;  //属性（名称=值），参见HtmlParser.getAttribute()
	size_t flags;    //bit OR of HtmlNodeFlag
	char* pInternal; //reserved for internal usage
	void* pUser;     //user customized, default to NULL
};


//HtmlParser: HTML文本解析类
class HtmlParser
{
public:
	HtmlParser() {}
	virtual ~HtmlParser() { freeHtmlNodes(); }

private:
	//disallow copy and assign: only declare, no implementation
	HtmlParser(const HtmlParser&);
	void operator=(const HtmlParser&);

public:
	//html
	void parseHtml(const char* szHtml, bool parseProps = false);

	//nodes
	size_t getHtmlNodeCount();
	HtmlNode* getHtmlNode(size_t index); //must: 0 <= index < getHtmlNodeCount()
	void freeHtmlNodes();
	//attributes
	const HtmlAttribute* getAttribute(const HtmlNode* pNode, size_t index); //must: 0 <= index < pNode->attributeCount
	const HtmlAttribute* getAttribute(const HtmlNode* pNode, const char* szAttributeName); //return NULL if attribute not exist
	const char* getAttributeStringValue(const HtmlNode* pNode, const char* szAttributeName, const char* szDefaultValue = NULL);
	int getAttributeIntValue(const HtmlNode* pNode, const char* szAttributeName, int defaultValue = 0);
	void parseAttributes(HtmlNode* pNode); //解析节点属性
	//output
	void outputHtml(MemBuffer& buffer, bool keepBufferData = false);
	void outputHtmlNode(MemBuffer& buffer, const HtmlNode* pNode);
	void dumpHtmlNodes(FILE* f = stdout);

protected:
	//允许子类覆盖, 以便识别更多标签(提高解析质量), 或者识别更少标签(提高解析速度)
	//默认仅识别涉及HTML基本结构和信息的有限几个开始标签: A,IMG,META,BODY,TITLE,FRAME,IFRAME
	virtual HtmlTagType onIdentifyHtmlTag(const char* szTagName, HtmlNodeType nodeType);
	//允许子类覆盖, 以便更好的解析节点属性, 或者部分解析甚至干脆不解析节点属性(提高解析速度)
	//可以根据标签名称(pNode->tagName)或标签类型(pNode->tagType)判断是否需要解析属性
	//默认仅解析已识别出标签类型的开始标签的属性（即pNode->type == NODE_START_TAG && pNode->tagType != NODE_UNKNOWN）
	virtual void onParseAttributes(HtmlNode* pNode);
	//允许子类覆盖, 在某节点解析完成后被调用, 如果返回false则立刻停止解析HTML
	//这里也许是一个恰当的时机初始化pNode.pUser
	virtual bool onNodeReady(HtmlNode* pNode) { return true; }

private:
	HtmlNode* newHtmlNode();

private:
	MemBuffer m_HtmlNodes;
};

} //namespace liigo

#endif //__HtmlParser_H__
