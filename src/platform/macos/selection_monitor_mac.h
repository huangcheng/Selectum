#pragma once

#include <platform/selection_monitor.h>

#include <QString>
#include <QTimer>

namespace selectum {

class SelectionMonitorMac final : public SelectionMonitor
{
    Q_OBJECT

public:
    explicit SelectionMonitorMac(QObject *parent = nullptr);
    ~SelectionMonitorMac() override;

    void start() override;
    void stop() override;
    AccessibilityPermission accessibilityPermission() const override;

private:
    void onGlobalMouseUp();
    void settleRead();

    void *m_mouseUpMonitor = nullptr;
    QTimer m_debounce;
    QString m_lastEmittedText;
    bool m_running = false;
};

} // namespace selectum
