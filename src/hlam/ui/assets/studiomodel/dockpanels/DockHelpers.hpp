#pragma once

#include <QComboBox>

namespace studiomodel
{
inline void SetRestoredModelIndex(int index, int count, QComboBox& comboBox)
{
	//If the model didn't have any elements and does now, default to the first element
	if (index == -1 && count > 0)
	{
		index = 0;
	}

	if (index >= count)
	{
		index = 0;
	}

	comboBox.setCurrentIndex(index);
}
}
