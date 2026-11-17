#pragma once

#include <QJsonObject>

#include <selectum/abi/types.h>

namespace selectum {

QJsonObject toJson(const ScreenRect &value);
bool fromJson(const QJsonObject &json, ScreenRect *out);

QJsonObject toJson(const SelectionContext &value);
bool fromJson(const QJsonObject &json, SelectionContext *out);

QJsonObject toJson(const Invocation &value);
bool fromJson(const QJsonObject &json, Invocation *out);

QJsonObject toJson(const Result &value);
bool fromJson(const QJsonObject &json, Result *out);

} // namespace selectum
