#ifndef HLMV_SETTINGS_CHLMVSETTINGS_H
#define HLMV_SETTINGS_CHLMVSETTINGS_H

#include "settings/CBaseSettings.h"
#include "settings/CRecentFiles.h"

namespace hlmv
{
/**
*	Settings for HLMV.
*/
class CHLMVSettings final : public settings::CBaseSettings
{
public:
	static const size_t MAX_RECENT_FILES = 4;

public:
	CHLMVSettings();
	~CHLMVSettings();

	CHLMVSettings( const CHLMVSettings& other );
	CHLMVSettings& operator=( const CHLMVSettings& other );

	/**
	*	Gets the recent files list.
	*/
	std::shared_ptr<const settings::CRecentFiles> GetRecentFiles() const { return m_RecentFiles; }

	/**
	*	@copydoc GetRecentFiles() const
	*/
	std::shared_ptr<settings::CRecentFiles> GetRecentFiles() { return m_RecentFiles; }

protected:
	bool LoadFromFile( const std::shared_ptr<CKvBlockNode>& root ) override final;

	bool SaveToFile( CKeyvaluesWriter& writer ) override final;

private:
	void Copy( const CHLMVSettings& other );

private:
	std::shared_ptr<settings::CRecentFiles> m_RecentFiles = std::make_shared<settings::CRecentFiles>( MAX_RECENT_FILES );
};
}

#endif //HLMV_SETTINGS_CHLMVSETTINGS_H