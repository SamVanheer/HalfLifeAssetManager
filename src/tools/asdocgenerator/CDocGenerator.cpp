#include <cassert>
#include <fstream>

#include "angelscript.h"

#include "shared/Logging.h"

#include "keyvalues/Keyvalues.h"

#include "html\CHTMLPage.h"
#include "html\CHTMLHeader.h"
#include "html\CHTMLBody.h"
#include "html\CHTMLElement.h"

#include "CDocGenerator.h"

CString DefaultContentConverter( const kv::KV& text )
{
	return text.GetValue();
}

CString NamespaceContentConverter( const kv::KV& text )
{
	return !text.GetValue().IsEmpty() ? text.GetValue() : "&lt;Global&gt;";
}

bool CDocGenerator::GenerateFromFile( const char* const pszInputFilename, const char* const pszOutputDirectory )
{
	assert( pszInputFilename );
	assert( pszOutputDirectory );

	kv::Parser parser( pszInputFilename );

	if( !parser.HasInputData() )
	{
		Error( "Couldn't open file \"%s\", aborting\n", pszInputFilename );
		return false;
	}

	parser.SetEscapeSeqConversion( GetEscapeSeqConversion() );

	Message( "Reading keyvalues from file '%s'\n", pszInputFilename );

	const kv::Parser::ParseResult result = parser.Parse();
	
	if( result != kv::Parser::ParseResult::SUCCESS )
	{
		Error( "Failed to read file \"%s\" with error '%s'; aborting\n", pszInputFilename, kv::Parser::ParseResultToString( result ) );
		return false;
	}

	auto root = parser.GetKeyvalues();

	auto indexPage = CreatePage( "index" );

	auto body = indexPage->GetBody();

	body->AddObject( std::make_shared<CHTMLElement>( "h1", "Index" ) );

	body->AddObject( std::make_shared<CHTMLElement>( "p", "Sven Co-op Angelscript documentation." ) );

	auto documentation = root->FindFirstChild<kv::Block>( "Angelscript Documentation" );

	if( !documentation )
	{
		Error( "GenerateDocumentation: Node \"Angelscript Documentation\" must be a block node; aborting\n" );
		return false;
	}

	const DocsData_t docsData = { pszOutputDirectory, body, documentation };

	const GenDocsData_t genDocsData[] =
	{
		{ "Classes", "Classes", &CDocGenerator::GenerateClass, "Class" },
		{ "Enumerations", "Enums", &CDocGenerator::GenerateEnum, "Enum" },
		{ "Global Functions", "Functions", &CDocGenerator::GenerateGlobalFunctions },
		{ "Global Properties", "Properties", &CDocGenerator::GenerateGlobalProperties },
		{ "Type Definitions", "Typedefs", &CDocGenerator::GenerateTypedefs },
		{ "Function Definitions", "FuncDefs", &CDocGenerator::GenerateFuncDefs }
	};

	for( const auto& data : genDocsData )
	{
		if( !GenerateDocs( docsData, data ) )
			return false;
	}

	SavePage( pszOutputDirectory, indexPage );

	Message( "Done\n" );

	return true;
}

bool CDocGenerator::GenerateDocs( const DocsData_t& data, const GenDocsData_t& genData )
{
	data.body->AddObject( std::make_shared<CHTMLElement>( "h2", genData.pszType ) );

	auto uList = std::make_shared<CHTMLElement>( "ul" );

	data.body->AddObject( uList );

	auto block = data.pData->FindFirstChild<kv::Block>( genData.pszBlockName );

	if( !block )
	{
		Error( "GenerateDocs: Node \"%s\" must be a block node; aborting\n", genData.pszBlockName );
		return false;
	}

	Message( "Generating %s\n", genData.pszType );

	kv::Block::Children_t blocks;

	if( genData.pszBlockListName )
		blocks = block->GetChildrenByKey( genData.pszBlockListName );
	else
		blocks.push_back( block );

	for( auto it = blocks.rbegin(); it != blocks.rend(); ++it )
	{
		auto currentBlock = *it;

		if( !currentBlock || currentBlock->GetType() != kv::NodeType::BLOCK )
		{
			Error( "GenerateDocumentation: Node \"%s\" must be a block node; aborting\n", genData.pszBlockListName ? genData.pszBlockListName : genData.pszBlockName );
			return false;
		}

		auto page = ( this->*genData.generateFn )( *static_cast<kv::Block*>( currentBlock ) );

		if( page )
		{
			SavePage( data.szDestinationDirectory, page );

			auto li = std::make_shared<CHTMLElement>( "li" );
			auto a = std::make_shared<CHTMLElement>( "a", page->GetHeader()->GetTitle()->GetTextContents() );
			a->SetAttributeValue( "href", page->GetHeader()->GetTitle()->GetTextContents() + ".htm" );
			li->AddObject( a );
			uList->AddObject( li );
		}
		else
		{
			Warning( "GenerateDocs: Failed to create page for node '%s'\n", block->GetKey().CStr() );
		}
	}

	return true;
}

std::shared_ptr<CHTMLPage> CDocGenerator::CreatePage( const char* pszTitle )
{
	assert( pszTitle );

	auto page = std::make_shared<CHTMLPage>();

	page->GetHeader()->GetTitle()->SetTextContents( pszTitle );

	auto stylesheet = page->GetHeader()->GetStyleSheet();

	stylesheet->SetAttributeValue( "rel", "stylesheet" );
	stylesheet->SetAttributeValue( "type", "text/css" );
	stylesheet->SetAttributeValue( "href", "doc.css" );

	return page;
}

CString ClassFlagsContentConverter( const kv::KV& text )
{
	const asDWORD uiFlags = strtoul( text.GetValue().CStr(), nullptr, 10 );

	const char* pszType;

	if( uiFlags & asOBJ_REF )
		pszType = "Reference type";
	else if( uiFlags & asOBJ_VALUE )
		pszType = "Value type";
	else
		pszType = "Unknown type";

	return CString( "Type: " ) + pszType;
}

std::shared_ptr<CHTMLPage> CDocGenerator::GenerateClass( const kv::Block& classData )
{
	return GenerateTypePage(
		classData,
		{
			"ClassName",
			"Namespace",
			"Documentation",
			{
				{ "Flags", &ClassFlagsContentConverter },
			},
			{
				{
					"Methods",
					"Methods",
					"Method",
					{
						{ "Declaration", "Declaration", &DefaultContentConverter },
						{ "Description", "Documentation", &DefaultContentConverter }
					}
				},
				{
					"Properties",
					"Properties",
					"Property",
					{
						{ "Declaration", "Declaration", &DefaultContentConverter },
						{ "Description", "Documentation", &DefaultContentConverter }
					}
				}
			}
		}
	);
}

std::shared_ptr<CHTMLPage> CDocGenerator::GenerateEnum( const kv::Block& enumData )
{
	return GenerateTypePage(
		enumData,
		{
			"Name",
			"Namespace",
			"Documentation",
			{},
			{
				{
					"Values",
					"Values",
					"Value",
					{
						{ "Name", "Name", &DefaultContentConverter },
						{ "Description", "Documentation", &DefaultContentConverter }
					}
				}
			}
		}
	);
}

std::shared_ptr<CHTMLPage> CDocGenerator::GenerateGlobalFunctions( const kv::Block& functionsData )
{
	return GenerateCollectionPage( functionsData,
	{
		"Functions",
		"Global Functions",
		"Functions that are accessible at a global level.",
		{
			"Functions",
			"Function",
			{
				{ "Namespace", "Namespace", &NamespaceContentConverter },
				{ "Declaration", "Declaration", &DefaultContentConverter },
				{ "Description", "Documentation", &DefaultContentConverter }
			}
		}
	}
	);
}

std::shared_ptr<CHTMLPage> CDocGenerator::GenerateGlobalProperties( const kv::Block& propertiesData )
{
	return GenerateCollectionPage( propertiesData,
	{
		"Properties",
		"Global Properties",
		"Properties that are accessible at a global level.",
		{
			"Properties",
			"Property",
			{
				{ "Namespace", "Namespace", &NamespaceContentConverter },
				{ "Declaration", "Declaration", &DefaultContentConverter },
				{ "Description", "Documentation", &DefaultContentConverter }
			}
		}
	}
	);
}

std::shared_ptr<CHTMLPage> CDocGenerator::GenerateTypedefs( const kv::Block& typedefsData )
{
	return GenerateCollectionPage( typedefsData,
	{
		"Typedefs",
		"Type definitions",
		"Typedefs alias one type to another.",
		{
			"Typedefs",
			"Typedef",
			{
				{ "Namespace", "Namespace", &NamespaceContentConverter },
				{ "Type", "Type", &DefaultContentConverter },
				{ "Name", "Name", &DefaultContentConverter },
				{ "Description", "Documentation", &DefaultContentConverter }
			}
		}
	}
	);
}

std::shared_ptr<CHTMLPage> CDocGenerator::GenerateFuncDefs( const kv::Block& funcdefsData )
{
	return GenerateCollectionPage( funcdefsData,
	{
		"FuncDefs",
		"Function Definitions",
		"Function definitions are callbacks that can be passed around.<br/>"
		"Consult the <a href=\"http://www.angelcode.com/angelscript/sdk/docs/manual/doc_callbacks.html\">Angelscript documentation</a> for more info.",
		{
			"FuncDefs",
			"FuncDef",
			{
				{ "Namespace", "Namespace", &NamespaceContentConverter },
				{ "Declaration", "Name", &DefaultContentConverter },
				{ "Description", "Documentation", &DefaultContentConverter }
			}
		}
	}
	);
}

std::shared_ptr<CHTMLPage> CDocGenerator::GenerateTypePage( const kv::Block& data, const TypePage_t& pageData )
{
	auto name = data.FindFirstChild<kv::KV>( pageData.pszName );

	if( !name )
	{
		Error( "GenerateTypePage: Node \"%s\" must be a text node!\n", pageData.pszName );
		return nullptr;
	}

	auto page = CreatePage( name->GetValue().CStr() );

	auto body = page->GetBody();

	body->AddObject( std::make_shared<CHTMLElement>( "h1", name->GetValue().CStr() ) );

	auto nspace = data.FindFirstChild<kv::KV>( pageData.pszNamespace );

	if( !nspace )
	{
		Error( "GenerateTypePage: Node \"%s\" must be a text node\n", pageData.pszNamespace );
		return nullptr;
	}

	const auto& nspaceText = nspace->GetValue();

	if( !nspaceText.IsEmpty() )
	{
		body->AddObject( std::make_shared<CHTMLElement>( "p", std::string( "Namespace: " ) + nspaceText.CStr() ) );
	}

	auto documentation = data.FindFirstChild<kv::KV>( pageData.pszDocumentation );

	if( !documentation )
	{
		Error( "GenerateTypePage: Node \"%s\" must be a text node\n", pageData.pszDocumentation );
		return nullptr;
	}

	body->AddObject( std::make_shared<CHTMLElement>( "p", documentation->GetValue().CStr() ) );

	for( const auto& typeContent : pageData.typeContents )
	{
		auto contentText = data.FindFirstChild<kv::KV>( typeContent.szElementName.c_str() );

		if( !contentText )
		{
			Error( "GenerateTypePage: Node \"%s\" must be a text node\n", typeContent.szElementName.c_str() );
			return nullptr;
		}

		body->AddObject( std::make_shared<CHTMLElement>( "p", typeContent.converterFn( *contentText ).CStr() ) );
	}

	auto result = GenerateContents( data, pageData.contents );

	if( result.first )
	{
		for( auto element : result.second )
		{
			body->AddObject( element );
		}
	}

	return page;
}

std::shared_ptr<CHTMLPage> CDocGenerator::GenerateCollectionPage( const kv::Block& data, const CollectionPage_t& pageData )
{
	auto page = CreatePage( pageData.pszPageName );

	auto body = page->GetBody();

	auto div = GenerateCollectionHeader( data, pageData.header );

	body->AddObject( div );

	auto result = GenerateTable( data, pageData.content );

	if( !result.first )
		return nullptr;

	if( result.second )
		body->AddObject( result.second );

	return page;
}

std::shared_ptr<CHTMLElement> CDocGenerator::GenerateCollectionHeader( const kv::Block& data, const CollectionHeader_t& header )
{
	auto div = std::make_shared<CHTMLElement>( "div" );

	div->AddObject( std::make_shared<CHTMLElement>( "h1", header.pszHeader ) );

	div->AddObject( std::make_shared<CHTMLElement>( "p", header.pszDescription ) );

	return div;
}

/*
* Generates a table based on a list of text nodes
* The returned pair contains true if the list was empty, or if the table was generated successfully
* Returns false and nullptr if an error occurred
*/
std::pair<bool, std::shared_ptr<CHTMLElement>> CDocGenerator::GenerateTable( const kv::Block& data, const Content_t& content )
{
	auto blocks = data.GetChildrenByKey( content.pszBlockListName );

	if( blocks.empty() )
	{
		return std::make_pair( true, nullptr );
	}

	auto table = std::make_shared<CHTMLElement>( "table" );

	auto tableHeader = std::make_shared<CHTMLElement>( "tr" );

	for( const auto& content : content.vecContent )
	{
		tableHeader->AddObject( std::make_shared<CHTMLElement>( "th", content.szHeaderName ) );
	}

	table->AddObject( tableHeader );

	for( auto it = blocks.rbegin(); it != blocks.rend(); ++it )
	{
		auto blockNode = *it;

		if( blockNode->GetType() != kv::NodeType::BLOCK )
		{
			Error( "GenerateTable: Node \"%s\" must be a block node\n", content.pszBlockListName );
			continue;
		}

		auto block = static_cast<kv::Block*>( blockNode );

		auto row = std::make_shared<CHTMLElement>( "tr" );

		for( const auto& contentEntry : content.vecContent )
		{
			auto contextText = block->FindFirstChild<kv::KV>( contentEntry.szElementName.c_str() );

			if( !contextText )
			{
				Error( "GenerateTable: Node \"%s\" must be a text node\n", contentEntry.szElementName.c_str() );
				return std::make_pair( false, nullptr );
			}

			const CString szText = ( contentEntry.converterFn ? contentEntry.converterFn : DefaultContentConverter )( *contextText );

			row->AddObject( std::make_shared<CHTMLElement>( "td", szText.CStr() ) );
		}

		table->AddObject( row );
	}

	auto div = std::make_shared<CHTMLElement>( "div" );
	div->AddObject( std::make_shared<CHTMLElement>( "h2", content.pszHeaderName ) );
	div->AddObject( table );

	return std::make_pair( true, div );
}

CDocGenerator::GenContentsResult_t CDocGenerator::GenerateContents( const kv::Block& data, const std::vector<Content_t> contents )
{
	std::vector<std::shared_ptr<CHTMLElement>> contentElements;

	for( const auto& content : contents )
	{
		auto block = data.FindFirstChild<kv::Block>( content.pszBlockName );

		if( !block )
		{
			Error( "GenerateContents: Node \"%s\" must be a block node\n", content.pszBlockName );
			return GenContentsResult_t( false, {} );
		}

		auto result = GenerateTable( *block, content );

		if( !result.first )
			return GenContentsResult_t( false, {} );

		if( result.second )
			contentElements.push_back( result.second );
	}

	return GenContentsResult_t( true, std::move( contentElements ) );
}

void CDocGenerator::SavePage( const std::string& szDirectory, std::shared_ptr<CHTMLPage> page )
{
	const std::string szPath = std::string( ".\\" ) + szDirectory + '\\' + page->GetHeader()->GetTitle()->GetTextContents() + ".htm";
	std::ofstream stream( szPath );

	if( !stream.is_open() )
	{
		Error( "Could not open file '%s' for writing\n", szPath.c_str() );
		return;
	}

	std::string contents = page->GenerateHTML();

	stream << contents;

	stream.close();
}
