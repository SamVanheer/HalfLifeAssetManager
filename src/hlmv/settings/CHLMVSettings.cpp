#include "CHLMVSettings.h"

namespace hlmv
{
CHLMVSettings::CHLMVSettings()
{
}

CHLMVSettings::~CHLMVSettings()
{
}

CHLMVSettings::CHLMVSettings( const CHLMVSettings& other )
	: CBaseSettings( other )
{
	Copy( other );
}

CHLMVSettings& CHLMVSettings::operator=( const CHLMVSettings& other )
{
	CBaseSettings::operator=( other );

	if( this != &other )
	{
		Copy( other );
	}

	return *this;
}

void CHLMVSettings::Copy( const CHLMVSettings& other )
{
	*m_RecentFiles = *other.m_RecentFiles;
}

bool CHLMVSettings::LoadFromFile( const std::shared_ptr<CKvBlockNode>& root )
{
	return true;
}

bool CHLMVSettings::SaveToFile( CKeyvaluesWriter& writer )
{
	return true;
}
}