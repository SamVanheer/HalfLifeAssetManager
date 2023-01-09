#pragma once

#include <memory>

#include <QLoggingCategory>
#include <QString>
#include <QUuid>

#include <spdlog/logger.h>
#include <spdlog/fmt/fmt.h>

#include "qt/QtLogSink.hpp"

inline std::shared_ptr<spdlog::logger> CreateQtLoggerSt(const QLoggingCategory& category)
{
	auto sink = std::make_shared<QtLogSink<spdlog::details::null_mutex>>(category);

	auto logger = std::make_shared<spdlog::logger>(category.categoryName(), std::move(sink));

	logger->set_level(spdlog::level::trace);

	return logger;
}

template <>
struct fmt::formatter<QString>
{
	constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
	{
		auto it = ctx.begin();

		if (it != ctx.end() && *it != '}')
		{
			throw format_error("invalid format");
		}

		return it;
	}

	template <typename FormatContext>
	auto format(const QString& str, FormatContext& ctx) const -> decltype(ctx.out())
	{
		return fmt::format_to(ctx.out(), "{}", str.toStdString());
	}
};

template <>
struct fmt::formatter<QUuid>
{
	constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
	{
		auto it = ctx.begin();

		if (it != ctx.end() && *it != '}')
		{
			throw format_error("invalid format");
		}

		return it;
	}

	template <typename FormatContext>
	auto format(const QUuid& id, FormatContext& ctx) const -> decltype(ctx.out())
	{
		return fmt::format_to(ctx.out(), "{}", id.toString().toStdString());
	}
};
