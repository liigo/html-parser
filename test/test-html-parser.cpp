#include "../HtmlParser.h"

static testfile(const char* fileName)
{
	static liigo::HtmlParser htmlParser;
	static liigo::MemBuffer mem;
	if(mem.loadFromFile(fileName))
	{
		liigo::MemBuffer outFileName;
		outFileName.appendText(fileName);
		outFileName.appendText(".txt", -1, true);
		FILE* out = fopen((const char*)outFileName.getData(), "wb+");
		if(out == NULL)
			printf("can\'t open output file %s\n", (const char*)outFileName.getData());

		htmlParser.parseHtml((const char*)mem.getData(), true);
		htmlParser.dumpHtmlNodes(out);

		fclose(out);
	}
	else
	{
		printf("can't open input file %s\n", fileName);
	}
}

void main()
{
	liigo::HtmlParser htmlParser;

	htmlParser.parseHtml("<link rel=\"next\" href=\"objects.html\">", true);
	htmlParser.parseHtml("...<p>---<a href=url>link</a>...");
	htmlParser.parseHtml("<p>---< a   href=url >link</a>");
	htmlParser.parseHtml("<p x=a y=b z = \"c <a href=url>\" >", true);
	htmlParser.parseHtml("<a x=0> <b y=1> <c z=ok w=false> - </c>");
	htmlParser.parseHtml("<color=red>");
	htmlParser.parseHtml("<p><!--remarks-->...</p>");
	htmlParser.parseHtml("<style>..<p.><<.every things here, all in style</style>");
	htmlParser.parseHtml("<script>..<p.><<.every things here, all in script</script>");

	testfile("testfiles\\sina.com.cn.html");
	testfile("testfiles\\163.com.html");
	testfile("testfiles\\qq.com.html");
	testfile("testfiles\\sohu.com.html");
	testfile("testfiles\\google.com.html");
	testfile("testfiles\\plus.google.com.html");
	testfile("testfiles\\cnbeta.com.html");

}
