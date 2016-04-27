#ifndef ANGELSCRIPT_CHTMLPAGE_H
#define ANGELSCRIPT_CHTMLPAGE_H

#include <memory>
#include <string>

class CHTMLHeader;
class CHTMLBody;

class CHTMLPage
{
public:

	CHTMLPage();
	~CHTMLPage();

	virtual std::string GenerateHTML();

	std::shared_ptr<CHTMLHeader> GetHeader() const { return m_Header; }
	std::shared_ptr<CHTMLBody> GetBody() const { return m_Body; }

private:

	std::shared_ptr<CHTMLHeader> m_Header;
	std::shared_ptr<CHTMLBody> m_Body;

private:
	CHTMLPage( const CHTMLPage& );
	CHTMLPage& operator=( const CHTMLPage& );
};

#endif //ANGELSCRIPT_CHTMLPAGE_H