#include "soundsystem/SoundConstants.hpp"
#include "soundsystem/ISoundSystem.hpp"

#include "entity/HLMVStudioModelEntity.hpp"

LINK_ENTITY_TO_CLASS(studiomodel, HLMVStudioModelEntity);

void HLMVStudioModelEntity::OnCreate()
{
	BaseClass::OnCreate();

	SetThink(&ThisClass::AnimThink);

	//Always think for smooth animations
	SetFlags(entity::FL_ALWAYSTHINK);
}

bool HLMVStudioModelEntity::Spawn()
{
	SetSequence(0);
	SetController(0, 0.0f);
	SetController(1, 0.0f);
	SetController(2, 0.0f);
	SetController(3, 0.0f);
	SetMouth(0.0f);

	auto model = GetEditableModel();

	for (int n = 0; n < model->Bodyparts.size(); ++n)
	{
		SetBodygroup(n, 0);
	}

	SetSkin(0);

	return true;
}

void HLMVStudioModelEntity::HandleAnimEvent(const AnimEvent& event)
{
	//TODO: move to subclass.
	switch (event.id)
	{
	case SCRIPT_EVENT_SOUND:			// Play a named wave file
	case SCRIPT_EVENT_SOUND_VOICE:
	case SCRIPT_CLIENT_EVENT_SOUND:
	{
		if (PlaySound)
		{
			int pitch = soundsystem::PITCH_NORM;

			if (PitchFramerateAmplitude)
			{
				pitch = static_cast<int>(pitch * GetFrameRate());
			}

			GetContext()->SoundSystem->PlaySound(event.options, soundsystem::VOLUME_NORM, pitch);
		}

		break;
	}

	default: break;
	}
}

void HLMVStudioModelEntity::AnimThink()
{
	if (PlaySequence)
	{
		const float flTime = AdvanceFrame(0.0f, 0.1f);

		DispatchAnimEvents(true);
	}
}
