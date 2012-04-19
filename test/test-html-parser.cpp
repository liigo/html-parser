#include "../HtmlParser.h"

void main()
{
	HtmlParser htmlParser;

	htmlParser.ParseHtml(L"<link rel=\"next\" href=\"objects.html\">");
	htmlParser.ParseHtml(L"...<p>---<a href=url>link</a>...");
	htmlParser.ParseHtml(L"<p>---< a   href=url >link</a>");
	htmlParser.ParseHtml(L"<p x=a y=b z = \"c <a href=url>\" >");
	htmlParser.ParseHtml(L"<a x=0> <b y=1> <c z=ok w=false> - </c>");

}