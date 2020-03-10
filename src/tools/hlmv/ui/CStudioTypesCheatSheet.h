#ifndef TOOLS_SHARED_UI_CSTUDIOTYPESCHEATSHEET_H
#define TOOLS_SHARED_UI_CSTUDIOTYPESCHEATSHEET_H

#include "ui/wxHLMV.h"

namespace ui
{
class CStudioTypesCheatSheet final : public wxDialog
{
public:
	CStudioTypesCheatSheet( wxWindow* pParent );

private:

private:
	CStudioTypesCheatSheet( const CStudioTypesCheatSheet& ) = delete;
	CStudioTypesCheatSheet& operator=( const CStudioTypesCheatSheet& ) = delete;
};
}

#endif //TOOLS_SHARED_UI_CSTUDIOTYPESCHEATSHEET_H