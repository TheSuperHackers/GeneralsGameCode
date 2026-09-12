module;

#include <cstdint>

export module games.generalszh.gameplay.production.completion.upgrade_completion;

export namespace generalszh::production
{
inline constexpr std::uint32_t UpgradeCameoSlots = 5;

template<class Authority, class Presentation>
void CompleteUpgrade(Authority &authority, Presentation &presentation)
{
	authority.RecordCost();
	authority.NotifyScripts();

	if (presentation.IsLocallyViewed() && presentation.HasDisplayName())
	{
		presentation.ShowMessage();
		presentation.RadarCue();

		auto sound = presentation.ResearchSound();
		if (presentation.IsValidSound(sound))
			presentation.EmitSound(sound);
		else
			presentation.GenericCompletionVoice();

		presentation.SetUnitSpecificSound(sound);
		presentation.EmitSound(sound);
	}

	if (authority.IsPlayerUpgrade())
		authority.GrantPlayerUpgrade();
	else
		authority.GrantObjectUpgrade();

	authority.RecordAcademy();

	auto selected = presentation.SelectedObject();
	if (presentation.HasSelectedObject(selected))
	{
		auto definition = presentation.SelectedDefinition(selected);
		for (std::uint32_t slot = 0; slot < UpgradeCameoSlots; ++slot)
		{
			if (presentation.CameoMatches(definition, slot))
			{
				presentation.MarkSelectionDirty();
				break;
			}
		}
	}

	authority.DiscardEntry();
}
}
