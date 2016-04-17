#ifndef HLMV_SETTINGS_CHLMVSETTINGS_H
#define HLMV_SETTINGS_CHLMVSETTINGS_H

#include "utility/Color.h"

#include "settings/CBaseSettings.h"
#include "settings/CRecentFiles.h"
#include "settings/CGameConfigManager.h"

namespace hlmv
{
/**
*	Listener for HLMV settings.
*/
class IHLMVSettingsListener
{
public:
	virtual ~IHLMVSettingsListener() = 0;

	virtual void FPSChanged( const double flOldFPS, const double flNewFPS ) = 0;
};

inline IHLMVSettingsListener::~IHLMVSettingsListener()
{
}

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

public:
	CHLMVSettings();
	~CHLMVSettings();

	CHLMVSettings( const CHLMVSettings& other );
	CHLMVSettings& operator=( const CHLMVSettings& other );

	void ActiveConfigChanged( const std::shared_ptr<settings::CGameConfig>& oldConfig, const std::shared_ptr<settings::CGameConfig>& newConfig ) override final;

	/**
	*	Gets the HLMV listener.
	*/
	IHLMVSettingsListener* GetHLMVListener() { return m_pListener; }

	/**
	*	Sets the HLMV listener.
	*/
	void SetHLMVListener( IHLMVSettingsListener* const pListener )
	{
		m_pListener = pListener;
	}

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
	*	Gets the user defined light color.
	*/
	const Color& GetLightColor() const { return m_LightColor; }

	/**
	*	Sets the user defined light color.
	*	@param color Color to set.
	*/
	void SetLightColor( const Color& color ) { m_LightColor = color; }

	/**
	*	Gets the user defined wireframe color.
	*/
	const Color& GetWireframeColor() const { return m_WireframeColor; }

	/**
	*	Sets the user defined wireframe color.
	*	@param color Color to set.
	*/
	void SetWireframeColor( const Color& color ) { m_WireframeColor = color; }

	/**
	*	Gets the FPS.
	*/
	double GetFPS() const { return m_flFPS; }

	/**
	*	Sets the FPS.
	*/
	void SetFPS( const double flFPS );

protected:
	bool PostInitialize( const char* const pszFilename ) override final;

	void PreShutdown( const char* const pszFilename ) override final;

	bool LoadFromFile( const std::shared_ptr<CKvBlockNode>& root ) override final;

	bool SaveToFile( CKeyvaluesWriter& writer ) override final;

private:
	void Copy( const CHLMVSettings& other );

private:
	IHLMVSettingsListener* m_pListener = nullptr;

	std::shared_ptr<settings::CRecentFiles> m_RecentFiles = std::make_shared<settings::CRecentFiles>( MAX_RECENT_FILES );

	Color m_GroundColor = DEFAULT_GROUND_COLOR;

	Color m_BackgroundColor = DEFAULT_BACKGROUND_COLOR;

	Color m_CrosshairColor = DEFAULT_CROSSHAIR_COLOR;

	Color m_LightColor = DEFAULT_LIGHT_COLOR;

	Color m_WireframeColor = DEFAULT_WIREFRAME_COLOR;

	double m_flFPS = DEFAULT_FPS;
};
}

#endif //HLMV_SETTINGS_CHLMVSETTINGS_H