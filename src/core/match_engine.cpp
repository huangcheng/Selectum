#include <core/match_engine.h>

#include <QRegularExpression>

namespace selectum {
namespace {

QString triggerToWire(Trigger trigger)
{
    switch (trigger) {
    case Trigger::Selection:
        return QStringLiteral("selection");
    case Trigger::Palette:
        return QStringLiteral("palette");
    case Trigger::Hotkey:
        return QStringLiteral("hotkey");
    case Trigger::Clipboard:
        return QStringLiteral("clipboard");
    }
    return QStringLiteral("selection");
}

bool matchesPlugin(const PluginRecord &plugin, const SelectionContext &ctx, Trigger trigger)
{
    // Spec §10 order
    if (!plugin.enabled) {
        return false;
    }

    if (!plugin.match.triggers.isEmpty()
        && !plugin.match.triggers.contains(triggerToWire(trigger))) {
        return false;
    }

    if (ctx.text.size() < plugin.match.minLength) {
        return false;
    }

    if (plugin.match.editableOnly && !ctx.editable) {
        return false;
    }

    if (!plugin.match.appAllow.isEmpty() && !plugin.match.appAllow.contains(ctx.appId)) {
        return false;
    }

    if (plugin.match.appDeny.contains(ctx.appId)) {
        return false;
    }

    if (!plugin.match.regex.isEmpty()) {
        const QRegularExpression re(plugin.match.regex);
        if (!re.isValid() || !re.match(ctx.text).hasMatch()) {
            return false;
        }
    }

    return true;
}

} // namespace

QVector<PluginRecord> matchPlugins(const QVector<PluginRecord> &plugins,
                                   const SelectionContext &ctx,
                                   Trigger trigger)
{
    QVector<PluginRecord> matched;
    matched.reserve(plugins.size());
    for (const PluginRecord &plugin : plugins) {
        if (matchesPlugin(plugin, ctx, trigger)) {
            matched.append(plugin);
        }
    }
    return matched;
}

} // namespace selectum
