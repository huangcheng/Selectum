#include <core/invoke_async.h>

#include <QMetaObject>
#include <QPointer>
#include <QThreadPool>

namespace selectum {

void invokeAsync(PluginHost &host,
                 const QString &pluginId,
                 const SelectionContext &ctx,
                 Trigger trigger,
                 const QMap<QString, QString> &config,
                 QObject *receiver,
                 std::function<void(Result)> done)
{
    QPointer<QObject> receiverGuard(receiver);
    QThreadPool::globalInstance()->start(
        [&host, pluginId, ctx, trigger, config, receiverGuard, done = std::move(done)]() mutable {
            const Result result = host.invoke(pluginId, ctx, trigger, config);

            if (!receiverGuard || !done) {
                return;
            }

            QObject *target = receiverGuard.data();
            QMetaObject::invokeMethod(
                target,
                [done = std::move(done), result]() { done(result); },
                Qt::QueuedConnection);
        });
}

} // namespace selectum
