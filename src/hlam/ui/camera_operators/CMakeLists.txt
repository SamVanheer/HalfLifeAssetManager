target_sources(HLAM
	PRIVATE
		ArcBallCameraOperator.hpp
		CameraOperator.hpp
		CameraOperators.hpp
		FirstPersonCameraOperator.hpp
		FreeLookCameraOperator.hpp
		TextureCameraOperator.cpp
		TextureCameraOperator.hpp)

add_subdirectory(dockpanels)
