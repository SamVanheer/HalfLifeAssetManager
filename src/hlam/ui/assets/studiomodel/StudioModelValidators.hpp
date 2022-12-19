#pragma once

#include "qt/ByteLengthValidator.hpp"

namespace studiomodel
{
class StudioModelAssetProvider;

class BaseUniqueNameValidator : public qt::ByteLengthValidator
{
public:
	BaseUniqueNameValidator(int maxByteLength, StudioModelAssetProvider* provider, QObject* parent = nullptr)
		: ByteLengthValidator(maxByteLength, parent)
		, _provider(provider)
	{
	}

	State validate(QString& text, int& pos) const override;

protected:
	virtual bool IsUnique(StudioModelAsset* asset, const QString& text) const = 0;

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
	StudioModelAssetProvider* const _provider;
};

class UniqueAttachmentNameValidator : public BaseUniqueNameValidator
{
public:
	using BaseUniqueNameValidator::BaseUniqueNameValidator;

protected:
	bool IsUnique(StudioModelAsset* asset, const QString& text) const override;
};

class UniqueBoneNameValidator : public BaseUniqueNameValidator
{
public:
	using BaseUniqueNameValidator::BaseUniqueNameValidator;

protected:
	bool IsUnique(StudioModelAsset* asset, const QString& text) const override;
};

class UniqueTextureNameValidator : public BaseUniqueNameValidator
{
public:
	using BaseUniqueNameValidator::BaseUniqueNameValidator;

protected:
	bool IsUnique(StudioModelAsset* asset, const QString& text) const override;
};

class UniqueModelNameValidator : public BaseUniqueNameValidator
{
public:
	using BaseUniqueNameValidator::BaseUniqueNameValidator;

protected:
	bool IsUnique(StudioModelAsset* asset, const QString& text) const override;

public slots:
	void SetCurrentBodyPartIndex(int index)
	{
		if (_currentBodyPartIndex != index)
		{
			_currentBodyPartIndex = index;
			emit changed();
		}
	}

protected:
	int _currentBodyPartIndex{-1};
};
}
