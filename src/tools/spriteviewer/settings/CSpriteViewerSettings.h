#ifndef SPRITEVIEWER_SETTINGS_CSPRITEVIEWERSETTINGS_H
#define SPRITEVIEWER_SETTINGS_CSPRITEVIEWERSETTINGS_H

#include "utility/Color.h"

#include "settings/CBaseSettings.h"
#include "settings/CRecentFiles.h"

namespace sprview
{
/**
*	Settings for sprite viewer.
*/
class CSpriteViewerSettings final : public settings::CBaseSettings
{
public:
	static const size_t MAX_RECENT_FILES;

	static const Color DEFAULT_BACKGROUND_COLOR;

public:
	CSpriteViewerSettings( filesystem::IFileSystem* const pFileSystem );
	~CSpriteViewerSettings();

	CSpriteViewerSettings( const CSpriteViewerSettings& other );
	CSpriteViewerSettings& operator=( const CSpriteViewerSettings& other );

	/**
	*	Gets the recent files list.
	*/
	std::shared_ptr<const settings::CRecentFiles> GetRecentFiles() const { return m_RecentFiles; }

	/**
	*	@copydoc GetRecentFiles() const
	*/
	std::shared_ptr<settings::CRecentFiles> GetRecentFiles() { return m_RecentFiles; }

	/**
	*	Gets the user defined background color.
	*/
	const Color& GetBackgroundColor() const { return m_BackgroundColor; }

	/**
	*	Sets the user defined background color.
	*	@param color Color to set.
	*/
	void SetBackgroundColor( const Color& color ) { m_BackgroundColor = color; }

protected:
	bool PostInitialize( const char* const pszFilename ) override final;

	void PreShutdown( const char* const pszFilename ) override final;

	bool LoadFromFile( const kv::Block& root ) override final;

	bool SaveToFile( kv::Writer& writer ) override final;

private:
	void Copy( const CSpriteViewerSettings& other );

private:
	std::shared_ptr<settings::CRecentFiles> m_RecentFiles = std::make_shared<settings::CRecentFiles>( MAX_RECENT_FILES );

	Color m_BackgroundColor = DEFAULT_BACKGROUND_COLOR;
};
}

#endif //SPRITEVIEWER_SETTINGS_CSPRITEVIEWERSETTINGS_H