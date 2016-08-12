#ifndef HTML_CHTMLPAGE_H
#define HTML_CHTMLPAGE_H

#include <memory>
#include <string>

class CHTMLHeader;
class CHTMLBody;

/**
*	Represents an HTML page.
*/
class CHTMLPage final
{
public:

	CHTMLPage();
	~CHTMLPage() = default;

	/**
	*	Generates this page's HTML, and returns it as a string.
	*/
	std::string GenerateHTML();

	/**
	*	Gets the header.
	*/
	std::shared_ptr<CHTMLHeader> GetHeader() const { return m_Header; }

	/**
	*	Gets the body.
	*/
	std::shared_ptr<CHTMLBody> GetBody() const { return m_Body; }

private:

	std::shared_ptr<CHTMLHeader> m_Header;
	std::shared_ptr<CHTMLBody> m_Body;

private:
	CHTMLPage( const CHTMLPage& ) = delete;
	CHTMLPage& operator=( const CHTMLPage& ) = delete;
};

#endif //HTML_CHTMLPAGE_H