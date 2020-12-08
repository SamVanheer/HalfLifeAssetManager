#pragma once

namespace graphics
{
class IGraphicsContext
{
public:
	virtual ~IGraphicsContext() = 0 {}

	virtual void Begin() = 0;

	virtual void End() = 0;
};
}
