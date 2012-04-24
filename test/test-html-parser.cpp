#include "../HtmlParser.h"

using namespace liigo;

class ParseAll : public HtmlParser
{
	virtual void onParseNodeProps(HtmlNode* pNode)
	{
		parseNodeProps(pNode);
	}
};

static testfile(const char* fileName)
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

void main()
{
	HtmlParser htmlParser;

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
