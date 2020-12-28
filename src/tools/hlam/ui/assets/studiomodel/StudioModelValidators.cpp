#include <QString>

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelValidators.hpp"

namespace ui::assets::studiomodel
{
BaseUniqueNameValidator::State BaseUniqueNameValidator::validate(QString& text, int& pos) const
{
	if (State state = ByteLengthValidator::validate(text, pos); state != State::Acceptable)
	{
		return state;
	}

	if (!IsUnique(text))
	{
		return State::Invalid;
	}

	return State::Acceptable;
}

bool UniqueAttachmentNameValidator::IsUnique(const QString& text) const
{
	//Attachments names are all empty by default so they are always valid in this state
	if (text.isEmpty())
	{
		return true;
	}

	const auto header = _asset->GetStudioModel()->GetStudioHeader();

	for (int i = 0; i < header->numattachments; ++i)
	{
		if (i != _currentIndex)
		{
			if (text == header->GetAttachment(i)->name)
			{
				return false;
			}
		}
	}

	return true;
}

bool UniqueBoneNameValidator::IsUnique(const QString& text) const
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();

	for (int i = 0; i < header->numbones; ++i)
	{
		if (i != _currentIndex)
		{
			if (text == header->GetBone(i)->name)
			{
				return false;
			}
		}
	}

	return true;
}

bool UniqueTextureNameValidator::IsUnique(const QString& text) const
{
	const auto header = _asset->GetStudioModel()->GetTextureHeader();

	for (int i = 0; i < header->numtextures; ++i)
	{
		if (i != _currentIndex)
		{
			if (text == header->GetTexture(i)->name)
			{
				return false;
			}
		}
	}

	return true;
}
}
