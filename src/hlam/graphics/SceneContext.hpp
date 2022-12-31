#pragma once

class QOpenGLFunctions_1_1;

namespace graphics
{
class TextureLoader;

/**
*	@brief Container for scene-independent graphics resources
*/
class SceneContext final
{
public:
	SceneContext(QOpenGLFunctions_1_1* openglFunctions, TextureLoader* textureLoader)
		: OpenGLFunctions(openglFunctions)
		, TexLoader(textureLoader)
	{

	}

	QOpenGLFunctions_1_1* const OpenGLFunctions;
	TextureLoader* const TexLoader;

	int WindowWidth = 0;
	int WindowHeight = 0;
};
}
