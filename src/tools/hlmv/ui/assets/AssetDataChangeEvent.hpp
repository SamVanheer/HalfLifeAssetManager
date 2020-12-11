#pragma once

#include <QString>

namespace ui::assets
{
enum class AssetDataChangeType
{
	Addition,
	Removal,
	Modification
};

/**
*	@brief Contains information about a change made to an asset
*	Derive from this type to add more information
*	A copy constructor is required
*/
class AssetDataChangeEvent
{
public:
	AssetDataChangeEvent(AssetDataChangeType type)
		: _type(type)
	{
	}

	AssetDataChangeEvent(const AssetDataChangeEvent&) = default;

	AssetDataChangeType GetType() const { return _type; }

private:
	const AssetDataChangeType _type;
};
}
