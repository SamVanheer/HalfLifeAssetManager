#ifndef COPTIONSDIALOG_H
#define COPTIONSDIALOG_H

#include "ui/wxInclude.h"

class COptionsDialog final : public wxDialog
{
public:
	COptionsDialog();
	~COptionsDialog();

private:
	COptionsDialog( const COptionsDialog& ) = delete;
	COptionsDialog& operator=( const COptionsDialog& ) = delete;
};

#endif //COPTIONSDIALOG_H