#include <QtTest>

#include <QMutex>
#include <QThread>
#include <QWaitCondition>

#include <atomic>
#include <chrono>

#include <core/invoke_async.h>
#include <core/plugin_host.h>
#include <core/stub_secrets_store.h>
#include <selectum/abi/types.h>

using namespace selectum;

class TestInvokeAsync : public QObject
{
    Q_OBJECT

private slots:
    void callbackRunsOnReceiverThread();
};

void TestInvokeAsync::callbackRunsOnReceiverThread()
{
    PluginHost host;
    StubSecretsStore secrets;
    host.setSecretsStore(&secrets);

    QString error;
    QVERIFY2(host.scan(QStringLiteral(SELECTUM_TEST_PLUGINS_DIR), &error), qPrintable(error));

    SelectionContext ctx;
    ctx.text = QStringLiteral("hello");
    ctx.appId = QStringLiteral("com.example.app");
    ctx.editable = false;
    ctx.trigger = Trigger::Selection;

    QObject receiver;
    QCOMPARE(receiver.thread(), QThread::currentThread());

    std::atomic<bool> callbackStarted{false};
    std::atomic<bool> scheduleReturned{false};
    std::atomic<bool> callbackFinished{false};
    Qt::HANDLE callbackThreadId = nullptr;
    Result got;

    QMutex mutex;
    QWaitCondition cond;

    QThread worker;
    worker.start();
    QVERIFY(worker.isRunning());

    QObject caller;
    caller.moveToThread(&worker);

    const bool scheduled = QMetaObject::invokeMethod(
        &caller,
        [&]() {
            invokeAsync(host,
                        QStringLiteral("com.selectum.stub"),
                        ctx,
                        Trigger::Selection,
                        {},
                        &receiver,
                        [&](Result result) {
                            callbackStarted.store(true);
                            // Schedule must have returned before the posted callback runs.
                            QVERIFY(scheduleReturned.load());
                            callbackThreadId = QThread::currentThreadId();
                            got = std::move(result);
                            callbackFinished.store(true);
                            QMutexLocker lock(&mutex);
                            cond.wakeAll();
                        });
            scheduleReturned.store(true);
            // Queued to another thread: must not have run synchronously on the caller.
            QVERIFY(!callbackStarted.load());
        },
        Qt::BlockingQueuedConnection);
    QVERIFY(scheduled);

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (!callbackFinished.load() && std::chrono::steady_clock::now() < deadline) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        QMutexLocker lock(&mutex);
        if (!callbackFinished.load()) {
            cond.wait(&mutex, 50);
        }
    }

    worker.quit();
    QVERIFY(worker.wait(5000));

    QVERIFY2(callbackFinished.load(), "invokeAsync callback did not run");
    QCOMPARE(callbackThreadId, QThread::currentThreadId());
    QCOMPARE(got.status, ResultStatus::Ok);
    QCOMPARE(got.message, QStringLiteral("stub:hello"));
}

QTEST_MAIN(TestInvokeAsync)
#include "test_invoke_async.moc"
