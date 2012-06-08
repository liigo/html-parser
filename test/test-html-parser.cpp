#include "../HtmlParser.h"

using namespace liigo;

class ParseAll : public HtmlParser
{
	virtual void onParseAttributes(HtmlNode* pNode)
	{
		parseAttributes(pNode);
	}
};

static void testfile(const char* fileName);
static void testoutput(const char* szHtml);
static void parse_and_dump(const char* szHtml);
static void enumnodes();
static void test_custom_attributes();

void main()
{
	HtmlParser htmlParser;
	MemBuffer  mem;

	parse_and_dump("<a url=xx>父母<陪孩子长大>: </a>"); //视为不合法?
	parse_and_dump("<a value=野蛮肘击>"); //'蛮'字GB18030编码影响解析?
	parse_and_dump("<a alt=3岁120斤你信吗 src=罕见动物交配场景>"); //'斤见物'等字GB18030编码影响解析?	

	testoutput("abc<![CDATA[<<a/>>]]>xyz"); //正确解析CDATA
	testoutput("<!doctype html>"); //不要解析<!DOCTYPE ...>的属性，原样输出
	testoutput("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">");
	testoutput("<hr/><p /><img src='...'/>"); //自封闭标签
	testoutput("<a defer url=liigo.com x='' selected>"); //属性没有值
	testoutput("<a url=\"abc\'def\'\" x=\'hello \"liigo\"\'>"); //属性值双引号和单引号嵌套

	parse_and_dump("<script rel=\"next\" href=\"objects.html\">");
	parse_and_dump("...<p>---<a href=url>link</a>...");
	parse_and_dump("<p>---< a   href=url >link</a>");
	parse_and_dump("<a x=a y=b z = \"c <a href=url>\" >"); //属性值引号内有<或>不要影响解析
	parse_and_dump("<p>\"引号”不匹配</p>");
	parse_and_dump("<a x=0> <b y=1> <img z=ok w=false> - </img>");
	parse_and_dump("<color=red>");
	parse_and_dump("<p><!--remarks-->...</p>");
	parse_and_dump("<p><!--**<p></p>**--><x/>...</p>");
	parse_and_dump("<style>..<p.><<.every things here, all in style</style>");
	parse_and_dump("<script>..<p.><<.every things here, all in script</script>");
	parse_and_dump("<a href=\"http://www.163.com\">");  //确认解析后不是自封闭标签
	parse_and_dump("<a href=\"http://www.163.com\"/>"); //确认解析后是自封闭标签
	parse_and_dump("<a\tx=1> <a\nx=1\ny=2> <a\r\nx=1\r\ny=2>"); //非空格分隔符
	parse_and_dump("<a x=\"abc\"y=''z>"); //属性值引号后面没有空白分隔符，并不鲜见

	enumnodes(); //展示遍历节点的方法

	test_custom_attributes();

	//测试解析各大门户网站
	testfile("testfiles\\sina.com.cn.html");
	testfile("testfiles\\163.com.html");
	testfile("testfiles\\qq.com.html");
	testfile("testfiles\\sohu.com.html");
	testfile("testfiles\\baidu.com.html");
	testfile("testfiles\\google.com.html");
	testfile("testfiles\\plus.google.com.explore.html");
	testfile("testfiles\\cnbeta.com.html");
	testfile("testfiles\\taobao.com.html");

}


static void testfile(const char* fileName)
{
	static ParseAll htmlParser;
	static MemBuffer memOutHtml;
	static MemBuffer memHtml;
	if(memHtml.loadFromFile(fileName))
	{
		MemBuffer outFileName;
		outFileName.appendText(fileName);
		outFileName.appendText(".nodes.txt", -1, true);
		FILE* out = fopen((const char*)outFileName.getData(), "wb+");
		if(out == NULL)
			printf("can\'t open output file %s\n", (const char*)outFileName.getData());

		htmlParser.parseHtml((const char*)memHtml.getData(), true); //htmlParser.parseHtml()
		htmlParser.dumpHtmlNodes(out);
		fclose(out);

		memOutHtml.empty();
		htmlParser.outputHtml(memOutHtml); //htmlParser.outputHtml()
		outFileName.empty();
		outFileName.appendText(fileName);
		outFileName.appendText(".html.txt", -1, true);
		memOutHtml.saveToFile((const char*)outFileName.getData());
	}
	else
	{
		printf("can't open input file %s\n", fileName);
	}
}

static void testoutput(const char* szHtml)
{
	HtmlParser htmlParser;
	MemBuffer  mem;

	htmlParser.parseHtml(szHtml, true);
	htmlParser.outputHtml(mem); mem.appendChar('\0');
	printf("%s\n", (char*)mem.getData());
}

static void parse_and_dump(const char* szHtml)
{
	ParseAll parser;
	parser.parseHtml(szHtml, true);
	parser.dumpHtmlNodes();
}

static void enumnodes()
{
	HtmlParser htmlParser;
	htmlParser.parseHtml("<html><p>...</p>");

	//第一种遍历节点的方法: for循环
	for(int index = 0, count = htmlParser.getHtmlNodeCount(); index < count; index++)
	{
		HtmlNode* pNode = htmlParser.getHtmlNode(index);
		htmlParser.dumpHtmlNode(pNode);
	}

	//第二种遍历节点的方法: while循环
	//由于最后必然有一个额外添加的NODE_NULL节点，所以此法可行
	HtmlNode* pNode = htmlParser.getHtmlNode(0);
	while(pNode->type != NODE_NULL)
	{
		htmlParser.dumpHtmlNode(pNode);
		pNode++;
	}

	//即使在没有正常节点的情况下，while循环遍历方法也是可行的
	htmlParser.parseHtml(NULL);
	pNode = htmlParser.getHtmlNode(0);
	while(pNode->type != NODE_NULL)
	{
		htmlParser.dumpHtmlNode(pNode);
		pNode++;
	}

}

static void test_custom_attributes()
{
	HtmlParser htmlParser;
	htmlParser.parseHtml("<a custom='a=1 b=2 c d e=3 f' a=0>", true);
	HtmlNode* pNode = htmlParser.getHtmlNode(0);
	HtmlParser::parseExtraAttributes(HtmlParser::getAttributeStringValue(pNode, "custom"), pNode, "custom:");
	htmlParser.dumpHtmlNode(pNode);
}
