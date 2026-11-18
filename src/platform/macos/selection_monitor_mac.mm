#include "selection_monitor_mac.h"

#include "ax_selection.h"

#include <platform/selection_settle.h>

#import <AppKit/AppKit.h>

namespace selectum {

SelectionMonitorMac::SelectionMonitorMac(QObject *parent)
    : SelectionMonitor(parent)
{
    m_debounce.setSingleShot(true);
    m_debounce.setInterval(80);
    connect(&m_debounce, &QTimer::timeout, this, &SelectionMonitorMac::settleRead);
}

SelectionMonitorMac::~SelectionMonitorMac()
{
    stop();
}

void SelectionMonitorMac::start()
{
    if (m_running) {
        return;
    }
    m_running = true;

    m_mouseUpMonitor = (__bridge_retained void *)[NSEvent
        addGlobalMonitorForEventsMatchingMask:NSEventMaskLeftMouseUp
                                      handler:^(NSEvent *) {
                                          QMetaObject::invokeMethod(this,
                                                                    &SelectionMonitorMac::onGlobalMouseUp,
                                                                    Qt::QueuedConnection);
                                      }];
}

void SelectionMonitorMac::stop()
{
    m_debounce.stop();
    if (m_mouseUpMonitor) {
        id monitor = (__bridge_transfer id)m_mouseUpMonitor;
        m_mouseUpMonitor = nullptr;
        [NSEvent removeMonitor:monitor];
    }
    m_running = false;
}

AccessibilityPermission SelectionMonitorMac::accessibilityPermission() const
{
    return queryAccessibilityPermission();
}

void SelectionMonitorMac::onGlobalMouseUp()
{
    if (!m_running) {
        return;
    }
    m_debounce.start();
}

void SelectionMonitorMac::settleRead()
{
    SelectionContext ctx;
    if (!readFocusedSelection(&ctx)) {
        return;
    }

    @autoreleasepool {
        const QString selfId = QString::fromNSString([[NSBundle mainBundle] bundleIdentifier]);
        if (!selfId.isEmpty() && ctx.appId == selfId) {
            return;
        }
    }

    if (!shouldEmitSettledSelection(m_lastEmittedText, ctx.text, true)) {
        return;
    }

    m_lastEmittedText = ctx.text.trimmed();
    emit selectionSettled(ctx);
}

} // namespace selectum
