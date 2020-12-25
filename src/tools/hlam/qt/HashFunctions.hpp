#pragma once

#include <functional>

#include <QHashFunctions>
#include <QUUid>

namespace std
{
template<>
struct hash<QUuid>
{
    std::size_t operator()(const QUuid& id) const noexcept
    {
        return qHash(id, qHash(std::hash<int>{}(0)));
    }
};
}
