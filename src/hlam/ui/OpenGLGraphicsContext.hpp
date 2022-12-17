#pragma once

#include <memory>

#include <QOffscreenSurface>
#include <QOpenGLContext>

#include "graphics/IGraphicsContext.hpp"

class OpenGLGraphicsContext final : public graphics::IGraphicsContext
{
public:
	OpenGLGraphicsContext(std::unique_ptr<QSurface>&& surface, std::unique_ptr<QOpenGLContext>&& context)
		: _surface(std::move(surface))
		, _context(std::move(context))
	{
	}

	void Begin() override
	{
		_context->makeCurrent(_surface.get());
	}

	void End() override
	{
		_context->doneCurrent();
	}

private:
	const std::unique_ptr<QSurface> _surface;
	const std::unique_ptr<QOpenGLContext> _context;
};
