#include "../HtmlParser.h"

using namespace liigo;

class ParseAll : public HtmlParser
{
	virtual void onParseNodeProps(HtmlNode* pNode)
	{
		parseNodeProps(pNode);
	}
};

static void testfile(const char* fileName);
static void testoutput(const char* szHtml);

void main()
{
	HtmlParser htmlParser;
	MemBuffer  mem;

	//to be fixed
	htmlParser.parseHtml("<a\tx=1> <a\nx=1\ny=2> <a\r\nx=1\r\ny=2>", true); //非空格分隔符
	htmlParser.parseHtml("<a x=\"abc\"y=0>", true); //属性之间没有分隔符，并不鲜见

	testoutput("<!doctype html>"); //不要解析<!DOCTYPE ...>的属性，原样输出
	testoutput("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">");
	testoutput("<hr/><p /><img src='...'/>"); //自封闭节点
	testoutput("<a defer url=liigo.com selected>"); //属性没有值

	htmlParser.parseHtml("<script rel=\"next\" href=\"objects.html\">", true);
	htmlParser.parseHtml("...<p>---<a href=url>link</a>...");
	htmlParser.parseHtml("<p>---< a   href=url >link</a>");
	htmlParser.parseHtml("<a x=a y=b z = \"c <a href=url>\" >", true); //属性值引号内有<或>不要影响解析
	htmlParser.parseHtml("<p>\"引号”不匹配</p>");
	htmlParser.parseHtml("<a x=0> <b y=1> <img z=ok w=false> - </img>", true);
	htmlParser.parseHtml("<color=red>");
	htmlParser.parseHtml("<p><!--remarks-->...</p>");
	htmlParser.parseHtml("<p><!--**<p></p>**--><x/>...</p>");
	htmlParser.parseHtml("<style>..<p.><<.every things here, all in style</style>");
	htmlParser.parseHtml("<script>..<p.><<.every things here, all in script</script>");

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
		htmlParser.outputHtmlNodes(out);
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
