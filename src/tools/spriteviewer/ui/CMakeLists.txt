target_sources(${TARGET_NAME}
	PRIVATE
		C3DView.cpp
		C3DView.h
		CMainPanel.cpp
		CMainPanel.h
		CMainWindow.cpp
		CMainWindow.h
		CSpriteListBox.cpp
		CSpriteListBox.h
		CSpriteViewerApp.cpp
		CSpriteViewerApp.h
		wxSpriteViewer.h)

add_subdirectory(controlpanels)
