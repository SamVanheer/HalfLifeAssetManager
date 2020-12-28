#pragma once

#include "qt/ByteLengthValidator.hpp"

namespace ui::assets::studiomodel
{
class StudioModelAsset;

class BaseUniqueNameValidator : public qt::ByteLengthValidator
{
public:
	BaseUniqueNameValidator(int maxByteLength, StudioModelAsset* asset, QObject* parent = nullptr)
		: ByteLengthValidator(maxByteLength, parent)
		, _asset(asset)
	{
	}

	State validate(QString& text, int& pos) const override;

protected:
	virtual bool IsUnique(const QString& text) const = 0;

public slots:
	void SetCurrentIndex(int index)
	{
		if (_currentIndex != index)
		{
			_currentIndex = index;
			emit changed();
		}
	}

protected:
	int _currentIndex{-1};
	StudioModelAsset* const _asset;
};

class UniqueAttachmentNameValidator : public BaseUniqueNameValidator
{
public:
	using BaseUniqueNameValidator::BaseUniqueNameValidator;

protected:
	bool IsUnique(const QString& text) const override;
};

class UniqueBoneNameValidator : public BaseUniqueNameValidator
{
public:
	using BaseUniqueNameValidator::BaseUniqueNameValidator;

protected:
	bool IsUnique(const QString& text) const override;
};

class UniqueTextureNameValidator : public BaseUniqueNameValidator
{
public:
	using BaseUniqueNameValidator::BaseUniqueNameValidator;

protected:
	bool IsUnique(const QString& text) const override;
};
}
