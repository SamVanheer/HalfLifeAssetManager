#pragma once

#include <QWizardPage>

#include "ui_AutodetectConfigurePropertiesPage.h"

class AutodetectConfigurePropertiesPage final : public QWizardPage
{
public:
	explicit AutodetectConfigurePropertiesPage(QWidget* parent = nullptr);
	~AutodetectConfigurePropertiesPage();

	bool isComplete() const override;

private:
	Ui_AutodetectConfigurePropertiesPage _ui;
};
