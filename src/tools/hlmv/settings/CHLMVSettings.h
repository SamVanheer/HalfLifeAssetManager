#ifndef HLMV_SETTINGS_CHLMVSETTINGS_H
#define HLMV_SETTINGS_CHLMVSETTINGS_H

#include "utility/Color.h"

#include "settings/CBaseSettings.h"
#include "settings/CRecentFiles.h"
#include "settings/CGameConfigManager.h"

namespace hlmv
{
/**
*	Settings for HLMV.
*/
class CHLMVSettings final : public settings::CBaseSettings, public settings::IGameConfigListener
{
public:
	static const size_t MAX_RECENT_FILES = 4;

	static const Color DEFAULT_GROUND_COLOR;

	static const Color DEFAULT_BACKGROUND_COLOR;

	static const Color DEFAULT_CROSSHAIR_COLOR;

	static const Color DEFAULT_LIGHT_COLOR;

	static const Color DEFAULT_WIREFRAME_COLOR;

	static const float MIN_FLOOR_LENGTH;

	static const float MAX_FLOOR_LENGTH;

	static const float DEFAULT_FLOOR_LENGTH;

public:
	CHLMVSettings( filesystem::IFileSystem* const pFileSystem );
	~CHLMVSettings();

	CHLMVSettings( const CHLMVSettings& other );
	CHLMVSettings& operator=( const CHLMVSettings& other );

	void ActiveConfigChanged( const std::shared_ptr<settings::CGameConfig>& oldConfig, const std::shared_ptr<settings::CGameConfig>& newConfig ) override final;

	/**
	*	Gets the recent files list.
	*/
	std::shared_ptr<const settings::CRecentFiles> GetRecentFiles() const { return m_RecentFiles; }

	/**
	*	@copydoc GetRecentFiles() const
	*/
	std::shared_ptr<settings::CRecentFiles> GetRecentFiles() { return m_RecentFiles; }

	/**
	*	Gets the user defined ground color.
	*/
	const Color& GetGroundColor() const { return m_GroundColor; }

	/**
	*	Sets the user defined ground color.
	*	@param color Color to set.
	*/
	void SetGroundColor( const Color& color ) { m_GroundColor = color; }

	/**
	*	Gets the user defined background color.
	*/
	const Color& GetBackgroundColor() const { return m_BackgroundColor; }

	/**
	*	Sets the user defined background color.
	*	@param color Color to set.
	*/
	void SetBackgroundColor( const Color& color ) { m_BackgroundColor = color; }

	/**
	*	Gets the user defined crosshair color.
	*/
	const Color& GetCrosshairColor() const { return m_CrosshairColor; }

	/**
	*	Sets the user defined crosshair color.
	*	@param color Color to set.
	*/
	void SetCrosshairColor( const Color& color ) { m_CrosshairColor = color; }

	/**
	*	Gets the floor length.
	*/
	float GetFloorLength() const { return m_flFloorLength; }

	/**
	*	Sets the floor length.
	*/
	void SetFloorLength( float flLength );

protected:
	bool PostInitialize( const char* const pszFilename ) override final;

	void PreShutdown( const char* const pszFilename ) override final;

	bool LoadFromFile( const kv::Block& root ) override final;

	bool SaveToFile( kv::Writer& writer ) override final;

private:
	void Copy( const CHLMVSettings& other );

private:
	std::shared_ptr<settings::CRecentFiles> m_RecentFiles = std::make_shared<settings::CRecentFiles>( MAX_RECENT_FILES );

	Color m_GroundColor = DEFAULT_GROUND_COLOR;

	Color m_BackgroundColor = DEFAULT_BACKGROUND_COLOR;

	Color m_CrosshairColor = DEFAULT_CROSSHAIR_COLOR;

	float m_flFloorLength = DEFAULT_FLOOR_LENGTH;
};
}

#endif //HLMV_SETTINGS_CHLMVSETTINGS_H