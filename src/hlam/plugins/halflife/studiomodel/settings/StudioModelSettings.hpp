#pragma once

#include <QSet>
#include <QString>

#include "settings/BaseSettings.hpp"

enum class XashOpenMode
{
	Ask = 0,
	Always,
	Never
};

class StudioModelSettings final : public BaseSettings
{
public:
	static constexpr bool DefaultAutodetectViewmodels{true};
	static constexpr bool DefaultActivateTextureViewWhenTexturesPanelOpened{true};

	static constexpr int MinimumGroundLength = 0;
	static constexpr int MaximumGroundLength = 2048;
	static constexpr int DefaultGroundLength = 100;

	using BaseSettings::BaseSettings;

	void LoadSettings() override;
	void SaveSettings() override;

	bool ShouldAutodetectViewmodels() const { return _autodetectViewModels; }

	void SetAutodetectViewmodels(bool value)
	{
		if (_autodetectViewModels != value)
		{
			_autodetectViewModels = value;
		}
	}

	bool ShouldActivateTextureViewWhenTexturesPanelOpened() const
	{
		return _activateTextureViewWhenTexturesPanelOpened;
	}

	void SetActivateTextureViewWhenTexturesPanelOpened(bool value)
	{
		_activateTextureViewWhenTexturesPanelOpened = value;
	}

	bool IsControlsBarVisible() const;
	void SetControlsBarVisible(bool value);

	bool IsTimelineVisible() const;
	void SetTimelineVisible(bool value);

	bool AreEditControlsVisible() const;
	void SetEditControlsVisible(bool value);

	int GetGroundLength() const { return _groundLength; }

	void SetGroundLength(int value)
	{
		_groundLength = value;
	}

	XashOpenMode GetXashOpenMode() const { return _xashOpenMode; }

	void SetXashOpenMode(XashOpenMode mode)
	{
		_xashOpenMode = mode;
	}

	float GetCameraFOV(const QString& name, float defaultValue) const;
	void SetCameraFOV(const QString& name, float value);

	const QSet<int>& GetSoundEventIds() const { return _soundEventIds; }

	void SetSoundEventIds(QSet<int>&& soundEventIds)
	{
		_soundEventIds = std::move(soundEventIds);
	}

private:
	bool _autodetectViewModels{DefaultAutodetectViewmodels};
	bool _activateTextureViewWhenTexturesPanelOpened{DefaultActivateTextureViewWhenTexturesPanelOpened};

	int _groundLength = DefaultGroundLength;

	XashOpenMode _xashOpenMode = XashOpenMode::Ask;

	QSet<int> _soundEventIds;
};
