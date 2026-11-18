#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "fake_selection_injector.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include <core/invoke_async.h>

#if defined(Q_OS_MACOS)
#include <platform/macos/ax_selection.h>
#endif

MainWindow::MainWindow(selectum::PluginHost *host,
                       selectum::SelectionMonitor *monitor,
                       QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_host(host)
    , m_monitor(monitor)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("Selectum"));

    auto *central = ui->centralwidget;
    auto *layout = new QVBoxLayout(central);

    auto *permRow = new QHBoxLayout();
    m_permissionLabel = new QLabel(central);
    m_permissionButton = new QPushButton(QStringLiteral("Enable Accessibility…"), central);
    permRow->addWidget(m_permissionLabel, 1);
    permRow->addWidget(m_permissionButton);
    layout->addLayout(permRow);

    layout->addWidget(new QLabel(QStringLiteral("Fake selection text"), central));

    m_textEdit = new QLineEdit(central);
    m_textEdit->setText(QStringLiteral("hello"));
    m_textEdit->setClearButtonEnabled(true);
    layout->addWidget(m_textEdit);

    auto *buttonRow = new QHBoxLayout();
    m_fakeButton = new QPushButton(QStringLiteral("Fake selection"), central);
    m_invokeButton = new QPushButton(QStringLiteral("Invoke selected"), central);
    m_invokeButton->setEnabled(false);
    buttonRow->addWidget(m_fakeButton);
    buttonRow->addWidget(m_invokeButton);
    layout->addLayout(buttonRow);

    layout->addWidget(new QLabel(QStringLiteral("Matched plugins"), central));
    m_matchList = new QListWidget(central);
    layout->addWidget(m_matchList, 1);

    connect(m_fakeButton, &QPushButton::clicked, this, &MainWindow::onFakeSelection);
    connect(m_invokeButton, &QPushButton::clicked, this, &MainWindow::onInvokeSelected);
    connect(m_permissionButton, &QPushButton::clicked, this, &MainWindow::onRequestAccessibility);
    connect(m_matchList, &QListWidget::itemSelectionChanged, this, [this]() {
        m_invokeButton->setEnabled(m_matchList->currentItem() != nullptr);
    });
    connect(m_matchList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *) {
        onInvokeSelected();
    });

    if (m_monitor) {
        connect(m_monitor,
                &selectum::SelectionMonitor::selectionSettled,
                this,
                &MainWindow::onSelectionSettled);
    }

    refreshPermissionLabel();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onSelectionSettled(const selectum::SelectionContext &context)
{
    m_textEdit->setText(context.text);
    applyContext(context, true);
}

void MainWindow::onFakeSelection()
{
    applyContext(selectum::FakeSelectionInjector::makeContext(m_textEdit->text()), true);
}

void MainWindow::applyContext(const selectum::SelectionContext &context, bool autoInvoke)
{
    m_matchList->clear();
    m_invokeButton->setEnabled(false);
    m_lastContext = context;
    m_hasContext = true;

    if (!m_host) {
        statusBar()->showMessage(QStringLiteral("No PluginHost"), 3000);
        return;
    }

    const auto matched = m_host->match(context, selectum::Trigger::Selection);

    for (const auto &manifest : matched) {
        auto *item = new QListWidgetItem(manifest.id, m_matchList);
        item->setData(Qt::UserRole, manifest.id);
    }

    if (matched.isEmpty()) {
        statusBar()->showMessage(QStringLiteral("No matching plugins"), 3000);
        return;
    }

    m_matchList->setCurrentRow(0);
    statusBar()->showMessage(
        QStringLiteral("%1 match(es) from %2")
            .arg(matched.size())
            .arg(context.appId.isEmpty() ? QStringLiteral("selection") : context.appId),
        3000);

    if (autoInvoke) {
        onInvokeSelected();
    }
}

void MainWindow::onInvokeSelected()
{
    if (!m_host || !m_hasContext) {
        return;
    }

    QListWidgetItem *item = m_matchList->currentItem();
    if (!item) {
        return;
    }

    const QString pluginId = item->data(Qt::UserRole).toString();

    m_fakeButton->setEnabled(false);
    m_invokeButton->setEnabled(false);
    statusBar()->showMessage(QStringLiteral("Invoking %1…").arg(pluginId));

    selectum::invokeAsync(*m_host,
                          pluginId,
                          m_lastContext,
                          selectum::Trigger::Selection,
                          {},
                          this,
                          [this](selectum::Result result) {
                              m_fakeButton->setEnabled(true);
                              m_invokeButton->setEnabled(m_matchList->currentItem() != nullptr);
                              showResult(result);
                          });
}

void MainWindow::onRequestAccessibility()
{
#if defined(Q_OS_MACOS)
    selectum::requestAccessibilityPermissionPrompt();
#endif
    refreshPermissionLabel();
}

void MainWindow::refreshPermissionLabel()
{
    using selectum::AccessibilityPermission;
    AccessibilityPermission status = AccessibilityPermission::Unknown;
    if (m_monitor) {
        status = m_monitor->accessibilityPermission();
    }

    switch (status) {
    case AccessibilityPermission::Granted:
        m_permissionLabel->setText(QStringLiteral("Accessibility: granted — select text in another app"));
        m_permissionButton->setEnabled(false);
        break;
    case AccessibilityPermission::Denied:
        m_permissionLabel->setText(QStringLiteral("Accessibility: denied — Fake selection still works"));
        m_permissionButton->setEnabled(true);
        break;
    case AccessibilityPermission::Unknown:
        m_permissionLabel->setText(QStringLiteral("Accessibility: unavailable on this platform"));
        m_permissionButton->setEnabled(false);
        break;
    }
}

void MainWindow::showResult(const selectum::Result &result)
{
    const QString status = [&]() -> QString {
        switch (result.status) {
        case selectum::ResultStatus::Ok:
            return QStringLiteral("ok");
        case selectum::ResultStatus::Error:
            return QStringLiteral("error");
        case selectum::ResultStatus::Cancelled:
            return QStringLiteral("cancelled");
        }
        return QStringLiteral("unknown");
    }();

    const QString message =
        result.message.isEmpty() ? QStringLiteral("(no message)") : result.message;
    statusBar()->showMessage(QStringLiteral("%1: %2").arg(status, message), 5000);
    QMessageBox::information(this,
                             QStringLiteral("Invoke result"),
                             QStringLiteral("%1\n%2").arg(status, message));
}
