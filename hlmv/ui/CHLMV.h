#ifndef HLMV_CHLMV_H
#define HLMV_CHLMV_H

class CMainWindow;
class CHLMVSettings;

namespace hlmv
{
/*
*	Facade class to access the entire HLMV UI.
*/
class CHLMV final
{
public:
	CHLMV( CHLMVSettings* pSettings, CMainWindow* pMainWindow );
	~CHLMV();

	CHLMVSettings* GetSettings() { return m_pSettings; }
	const CHLMVSettings* GetSettings() const { return m_pSettings; }

	//Load/Save model
	bool LoadModel( const char* const pszFilename );
	bool PromptLoadModel();

	bool SaveModel( const char* const pszFilename );
	bool PromptSaveModel();

	//Background and Ground textures
	bool LoadBackgroundTexture( const char* const pszFilename );
	bool PromptLoadBackgroundTexture();

	void UnloadBackgroundTexture();

	bool LoadGroundTexture( const char* const pszFilename );
	bool PromptGroundTexture();

	void UnloadGroundTexture();

	//Model Display

	//Body Parts

	//Textures

	//Sequences

	//Weapon Origin

	//Fullscreen

private:
	CHLMVSettings* m_pSettings;
	CMainWindow* m_pMainWindow;

private:
	CHLMV( const CHLMV& ) = delete;
	CHLMV& operator=( const CHLMV& ) = delete;
};
}

#endif //HLMV_CHLMV_H