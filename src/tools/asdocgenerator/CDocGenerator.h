#ifndef CDOCGENERATOR_H
#define CDOCGENERATOR_H

#include <memory>
#include <vector>

#include "utility/CString.h"

#include "keyvalues/KVForward.h"

class CHTMLPage;
class CHTMLBody;
class CHTMLElement;

CString DefaultContentConverter( const kv::KV& text );
CString NamespaceContentConverter( const kv::KV& text );

/**
*	Generates HTML documentation for Sven Co-op Angelscript documentation.
*/
class CDocGenerator final
{
public:
	CDocGenerator() = default;
	~CDocGenerator() = default;

	/**
	*	Generates HTML documentation from a file.
	*	@param pszInputFilename The input filename. This is the keyvalues text file that the game generates.
	*	@param pszOutputDirectory The directory where the HTML files are saved to.
	*	@return true on success, false otherwise.
	*/
	bool GenerateFromFile( const char* const pszInputFilename, const char* const pszOutputDirectory );

private:
	using GenerateFn = std::shared_ptr<CHTMLPage>( CDocGenerator::* )( const kv::Block& data );

	struct DocsData_t final
	{
		const std::string szDestinationDirectory;
		const std::shared_ptr<CHTMLBody> body;
		const kv::Block* const pData;

		DocsData_t( const std::string szDestinationDirectory,
					const std::shared_ptr<CHTMLBody> body,
					const kv::Block* const pData )
			: szDestinationDirectory( szDestinationDirectory )
			, body( body )
			, pData( pData )
		{
		}
	};

	struct GenDocsData_t final
	{
		const char* const pszType;
		const char* const pszBlockName;
		const GenerateFn generateFn;
		const char* const pszBlockListName;

		GenDocsData_t( const char* const pszType,
					   const char* const pszBlockName,
					   const GenerateFn generateFn,
					   const char* const pszBlockListName = nullptr )
			: pszType( pszType )
			, pszBlockName( pszBlockName )
			, generateFn( generateFn )
			, pszBlockListName( pszBlockListName )
		{
		}
	};

	bool GenerateDocs( const DocsData_t& data, const GenDocsData_t& genData );

	std::shared_ptr<CHTMLPage> CreatePage( const char* pszTitle );

	std::shared_ptr<CHTMLPage> GenerateClass( const kv::Block& classData );
	std::shared_ptr<CHTMLPage> GenerateEnum( const kv::Block& enumData );
	std::shared_ptr<CHTMLPage> GenerateGlobalFunctions( const kv::Block& functionsData );
	std::shared_ptr<CHTMLPage> GenerateGlobalProperties( const kv::Block& propertiesData );
	std::shared_ptr<CHTMLPage> GenerateTypedefs( const kv::Block& typedefsData );
	std::shared_ptr<CHTMLPage> GenerateFuncDefs( const kv::Block& funcdefsData );

	using CustomContentConverterFn = CString ( * )( const kv::KV& text );

	struct CollectionHeader_t final
	{
		const char* const pszHeader;
		const char* const pszDescription;
	};

	struct ContentEntry_t
	{
		const std::string szElementName;
		const CustomContentConverterFn converterFn;

		ContentEntry_t( std::string&& szElementName, const CustomContentConverterFn converterFn )
			: szElementName( std::move( szElementName ) )
			, converterFn( converterFn )
		{
		}
	};

	struct TableContentEntry_t final : public ContentEntry_t
	{
		const std::string szHeaderName;

		TableContentEntry_t( std::string&& szHeaderName, std::string&& szElementName, const CustomContentConverterFn converterFn )
			: ContentEntry_t( std::move( szElementName ), converterFn )
			, szHeaderName( std::move( szHeaderName ) )
		{
		}
	};

	typedef std::vector<TableContentEntry_t> TableContentEntries_t;

	struct Content_t final
	{
		const char* const pszHeaderName;
		const char* const pszBlockName;
		const char* const pszBlockListName;
		const TableContentEntries_t vecContent;

		Content_t( const char* const pszHeaderName, const char* const pszBlockListName, TableContentEntries_t&& vecContent )
			: pszHeaderName( pszHeaderName )
			, pszBlockName( nullptr )
			, pszBlockListName( pszBlockListName )
			, vecContent( std::move( vecContent ) )
		{
		}

		Content_t( const char* const pszHeaderName, const char* const pszBlockName, const char* const pszBlockListName, TableContentEntries_t&& vecContent )
			: pszHeaderName( pszHeaderName )
			, pszBlockName( pszBlockName )
			, pszBlockListName( pszBlockListName )
			, vecContent( std::move( vecContent ) )
		{
		}
	};

	struct CollectionPage_t final
	{
		const char* const pszPageName;
		const CollectionHeader_t header;
		const Content_t content;
	};

	typedef std::vector<ContentEntry_t> TypeContentEntries_t;

	struct TypePage_t final
	{
		const char* const pszName;
		const char* const pszNamespace;
		const char* const pszDocumentation;
		const TypeContentEntries_t typeContents;
		const std::vector<Content_t> contents;
	};

	std::shared_ptr<CHTMLPage> GenerateTypePage( const kv::Block& data, const TypePage_t& pageData );

	std::shared_ptr<CHTMLPage> GenerateCollectionPage( const kv::Block& data, const CollectionPage_t& pageData );

	std::shared_ptr<CHTMLElement> GenerateCollectionHeader( const kv::Block& data, const CollectionHeader_t& header );

	std::pair<bool, std::shared_ptr<CHTMLElement>> GenerateTable( const kv::Block& data, const Content_t& content );

	typedef std::pair<bool, std::vector<std::shared_ptr<CHTMLElement>>> GenContentsResult_t;

	GenContentsResult_t GenerateContents( const kv::Block& data, const std::vector<Content_t> contents );

	void SavePage( const std::string& szDirectory, std::shared_ptr<CHTMLPage> page );

private:
	CDocGenerator( const CDocGenerator& ) = delete;
	CDocGenerator& operator=( const CDocGenerator& ) = delete;
};

#endif //CDOCGENERATOR_H