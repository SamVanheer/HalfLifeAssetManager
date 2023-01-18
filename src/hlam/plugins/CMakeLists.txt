target_sources(HLAM
	PRIVATE
		IAssetManagerPlugin.hpp
		forwarding/ForwardingAssetManagerPlugin.cpp
		forwarding/ForwardingAssetManagerPlugin.hpp)

add_subdirectory(halflife)
