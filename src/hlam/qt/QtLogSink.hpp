#pragma once

#include <limits>
#include <memory>

#include <QDebug>
#include <QLoggingCategory>
#include <QMessageLogger>

#include <spdlog/sinks/base_sink.h>

/**
*	@brief spdlog sink that forward messages to Qt's logging system
*/
template<typename Mutex>
class QtLogSink final : public spdlog::sinks::base_sink<Mutex>
{
public:
	QtLogSink(const QLoggingCategory& category)
		: _category(category)
	{
		this->set_level(spdlog::level::trace);
	}

protected:
	void sink_it_(const spdlog::details::log_msg& msg) override
	{
		if (!IsLevelEnabled(msg.level))
		{
			return;
		}

		//Truncate payload if it's too large
		int payloadSize = std::numeric_limits<int>::max();

		if (msg.payload.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()))
		{
			payloadSize = static_cast<int>(msg.payload.size());
		}

		QMessageLogger logger{msg.source.filename, msg.source.line, msg.source.funcname, _category.categoryName()};

		auto debug = GetLogger(msg.level, logger);

		debug.nospace().noquote();

		//This allocates memory, but there is no API for logging UTF8 directly so this will have to do
		debug << QString::fromUtf8(msg.payload.data(), payloadSize);
	}

	void flush_() override
	{
		//Nothing
	}

private:
	bool IsLevelEnabled(spdlog::level_t level) const
	{
		switch (level)
		{
		case spdlog::level::trace:
			[[fallthrough]];
		case spdlog::level::debug: return _category.isDebugEnabled();
		case spdlog::level::info: return _category.isInfoEnabled();
		case spdlog::level::warn: return _category.isWarningEnabled();
		case spdlog::level::err:
			[[fallthrough]];
		case spdlog::level::critical: return _category.isCriticalEnabled();
		}

		return false;
	}

	static QDebug GetLogger(spdlog::level_t level, const QMessageLogger& logger)
	{
		switch (level)
		{
		case spdlog::level::trace:
			[[fallthrough]];
		case spdlog::level::debug: return logger.debug();
		case spdlog::level::info: return logger.info();
		case spdlog::level::warn: return  logger.warning();
		default:
			[[fallthrough]];
		case spdlog::level::err:
			[[fallthrough]];
		case spdlog::level::critical: return logger.critical();
		}
	}

private:
	const QLoggingCategory& _category;
};
