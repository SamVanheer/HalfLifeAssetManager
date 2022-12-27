target_sources(HLAM
	PRIVATE
		IAssetManagerPlugin.hpp
		quake1/Quake1AssetManagerPlugin.cpp
		quake1/Quake1AssetManagerPlugin.hpp
		source1/Source1AssetManagerPlugin.cpp
		source1/Source1AssetManagerPlugin.hpp)

add_subdirectory(halflife)
