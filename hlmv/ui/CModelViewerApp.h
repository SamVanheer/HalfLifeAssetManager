#ifndef CMODELVIEWERAPP_H
#define CMODELVIEWERAPP_H

#include <vector>

#include "wxHLMV.h"

#include <wx/vidmode.h>

class wxDisplay;

class CModelViewerApp final : public wxApp
{
public:
	typedef std::vector<wxVideoMode> VideoModes_t;

public:
	static CModelViewerApp& GetApp() { return *m_pInstance; }

	virtual bool OnInit() override;

	virtual int OnExit() override;

	const VideoModes_t& GetVideoModes() const { return m_VideoModes; }

private:
	static CModelViewerApp* m_pInstance;

	wxDisplay* m_pPrimaryDisplay;

	VideoModes_t m_VideoModes;
};

#endif //CMODELVIEWERAPP_H