#include "../HtmlParser.h"

void main()
{
	liigo::HtmlParser htmlParser;

	htmlParser.parseHtml("<link rel=\"next\" href=\"objects.html\">");
	htmlParser.parseHtml("...<p>---<a href=url>link</a>...");
	htmlParser.parseHtml("<p>---< a   href=url >link</a>");
	htmlParser.parseHtml("<p x=a y=b z = \"c <a href=url>\" >");
	htmlParser.parseHtml("<a x=0> <b y=1> <c z=ok w=false> - </c>");
	htmlParser.parseHtml("<color=red>");
	htmlParser.parseHtml("<p><!--remarks-->...</p>");
	htmlParser.parseHtml("<style>..<p.><<.every things here, all in style</style>");
	htmlParser.parseHtml("<script>..<p.><<.every things here, all in script</script>");
}
