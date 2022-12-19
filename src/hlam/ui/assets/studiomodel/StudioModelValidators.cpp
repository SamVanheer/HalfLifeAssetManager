#include <QString>

#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelValidators.hpp"

namespace studiomodel
{
BaseUniqueNameValidator::State BaseUniqueNameValidator::validate(QString& text, int& pos) const
{
	if (State state = ByteLengthValidator::validate(text, pos); state != State::Acceptable)
	{
		return state;
	}

	const auto asset = _provider->GetCurrentAsset();

	if (!asset)
	{
		return State::Acceptable;
	}

	if (!IsUnique(asset, text))
	{
		return State::Invalid;
	}

	return State::Acceptable;
}

bool UniqueAttachmentNameValidator::IsUnique(StudioModelAsset* asset, const QString& text) const
{
	//Attachments names are all empty by default so they are always valid in this state
	if (text.isEmpty())
	{
		return true;
	}

	const auto model = asset->GetEntity()->GetEditableModel();

	for (int i = 0; i < model->Attachments.size(); ++i)
	{
		if (i != _currentIndex)
		{
			if (text == model->Attachments[i]->Name.c_str())
			{
				return false;
			}
		}
	}

	return true;
}

bool UniqueBoneNameValidator::IsUnique(StudioModelAsset* asset, const QString& text) const
{
	const auto model = asset->GetEntity()->GetEditableModel();

	for (int i = 0; i < model->Bones.size(); ++i)
	{
		if (i != _currentIndex)
		{
			if (text == model->Bones[i]->Name.c_str())
			{
				return false;
			}
		}
	}

	return true;
}

bool UniqueTextureNameValidator::IsUnique(StudioModelAsset* asset, const QString& text) const
{
	const auto model = asset->GetEntity()->GetEditableModel();

	for (int i = 0; i < model->Textures.size(); ++i)
	{
		if (i != _currentIndex)
		{
			if (text == model->Textures[i]->Name.c_str())
			{
				return false;
			}
		}
	}

	return true;
}

bool UniqueModelNameValidator::IsUnique(StudioModelAsset* asset, const QString& text) const
{
	const auto model = asset->GetEntity()->GetEditableModel();

	for (int i = 0; i < model->Bodyparts.size(); ++i)
	{
		if (i != _currentBodyPartIndex)
		{
			const auto& bodyPart = *model->Bodyparts[i];

			for (int j = 0; j < bodyPart.Models.size(); ++j)
			{
				if (j != _currentIndex)
				{
					const auto& model = bodyPart.Models[j];

					if (text == model.Name.c_str())
					{
						return false;
					}
				}
			}
		}
	}

	return true;
}
}
