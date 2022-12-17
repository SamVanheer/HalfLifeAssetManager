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

	const auto model = _asset->GetEntity()->GetEditableModel();

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

bool UniqueBoneNameValidator::IsUnique(const QString& text) const
{
	const auto model = _asset->GetEntity()->GetEditableModel();

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

bool UniqueTextureNameValidator::IsUnique(const QString& text) const
{
	const auto model = _asset->GetEntity()->GetEditableModel();

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

bool UniqueModelNameValidator::IsUnique(const QString& text) const
{
	const auto model = _asset->GetEntity()->GetEditableModel();

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
