#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QSettings>

#include "ui/options/gameconfigurations/AutodetectConfigurePropertiesPage.hpp"

AutodetectConfigurePropertiesPage::AutodetectConfigurePropertiesPage(QWidget* parent)
	: QWizardPage(parent)
{
	_ui.setupUi(this);

	registerField("GameDirectory", _ui.GameDirectory);
	registerField("GameExecutable", _ui.GameExecutable);
	registerField("BaseGameDirectory", _ui.BaseGameDirectory);

	// On Windows the Half-Life directory is located using this registry key.
#ifdef WIN32
	QSettings modInstallPath{R"(HKEY_CURRENT_USER\SOFTWARE\Valve\Steam)", QSettings::NativeFormat};

	_ui.GameDirectory->setText(modInstallPath.value("ModInstallPath").toString().trimmed());

	// Set default values for Half-Life.
	if (const auto gameDirectory = _ui.GameDirectory->text(); !gameDirectory.isEmpty())
	{
		_ui.GameExecutable->setText(QDir{gameDirectory}.absoluteFilePath("hl.exe"));
		_ui.BaseGameDirectory->setText(QDir{gameDirectory}.absoluteFilePath("valve"));
	}
#endif

	connect(_ui.GameDirectory, &QLineEdit::textChanged, this, &AutodetectConfigurePropertiesPage::completeChanged);
	connect(_ui.GameExecutable, &QLineEdit::textChanged, this, &AutodetectConfigurePropertiesPage::completeChanged);
	connect(_ui.BaseGameDirectory, &QLineEdit::textChanged, this, &AutodetectConfigurePropertiesPage::completeChanged);

	connect(_ui.GameDirectory, &QLineEdit::textChanged, this,
		[this]
		{
			// Reset the other values since they depend on this one.
			_ui.GameExecutable->setText({});
			_ui.BaseGameDirectory->setText({});
		});

	connect(_ui.BrowseGameDirectory, &QPushButton::clicked, this,
		[this]
		{
			const auto gameDirectory = QFileDialog::getExistingDirectory(
				this, "Select game directory", _ui.GameDirectory->text());

			if (!gameDirectory.isEmpty())
			{
				_ui.GameDirectory->setText(gameDirectory);
			}
		});

	connect(_ui.BrowseGameExecutable, &QPushButton::clicked, this,
		[this]
		{
			auto directory = _ui.GameExecutable->text();

			if (directory.isEmpty())
			{
				directory = _ui.GameDirectory->text();
			}

			const auto path = QFileDialog::getOpenFileName(this, "Select Game Executable", directory,
				QStringLiteral("Executable Files (*.exe);;All Files (*.*)"));

			if (!path.isEmpty())
			{
				_ui.GameExecutable->setText(path);
			}
		});

	connect(_ui.BrowseBaseGameDirectory, &QPushButton::clicked, this,
		[this]
		{
			auto directory = _ui.BaseGameDirectory->text();

			if (directory.isEmpty())
			{
				directory = _ui.GameDirectory->text();
			}

			const auto path = QFileDialog::getExistingDirectory(
				this, "Select Base Game Directory", _ui.BaseGameDirectory->text());

			if (!path.isEmpty())
			{
				_ui.BaseGameDirectory->setText(path);
			}
		});
}

AutodetectConfigurePropertiesPage::~AutodetectConfigurePropertiesPage() = default;

bool AutodetectConfigurePropertiesPage::isComplete() const
{
	const auto gameDirectory = _ui.GameDirectory->text();
	const auto gameExecutable = _ui.GameExecutable->text();
	const auto baseGameDirectory = _ui.BaseGameDirectory->text();

	return !gameDirectory.isEmpty() && QDir {gameDirectory}.exists()
		&& !gameExecutable.isEmpty() && QFile::exists(gameExecutable)
		&& !baseGameDirectory.isEmpty() && QDir{baseGameDirectory}.exists();
}
