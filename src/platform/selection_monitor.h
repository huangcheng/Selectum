#pragma once

#include <memory>

#include <QObject>

#include <selectum/abi/types.h>

namespace selectum {

enum class AccessibilityPermission {
    Unknown,
    Granted,
    Denied,
};

class SelectionMonitor : public QObject
{
    Q_OBJECT

public:
    explicit SelectionMonitor(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    ~SelectionMonitor() override = default;

    virtual void start() = 0;
    virtual void stop() = 0;
    virtual AccessibilityPermission accessibilityPermission() const = 0;

signals:
    void selectionSettled(const selectum::SelectionContext &context);
};

// Returns a platform monitor, or a no-op monitor when unsupported.
std::unique_ptr<SelectionMonitor> createSelectionMonitor(QObject *parent = nullptr);

} // namespace selectum
