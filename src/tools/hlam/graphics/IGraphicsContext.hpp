#pragma once

namespace graphics
{
class IGraphicsContext
{
public:
	virtual ~IGraphicsContext() {}

	virtual void Begin() = 0;

	virtual void End() = 0;
};
}
