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

MainWindow::MainWindow(selectum::PluginHost *host, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_host(host)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("Selectum"));

    auto *central = ui->centralwidget;
    auto *layout = new QVBoxLayout(central);

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
    connect(m_matchList, &QListWidget::itemSelectionChanged, this, [this]() {
        m_invokeButton->setEnabled(m_matchList->currentItem() != nullptr);
    });
    connect(m_matchList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *) {
        onInvokeSelected();
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onFakeSelection()
{
    m_matchList->clear();
    m_invokeButton->setEnabled(false);

    if (!m_host) {
        statusBar()->showMessage(QStringLiteral("No PluginHost"), 3000);
        return;
    }

    const auto ctx = selectum::FakeSelectionInjector::makeContext(m_textEdit->text());
    const auto matched = m_host->match(ctx, selectum::Trigger::Selection);

    for (const auto &manifest : matched) {
        auto *item = new QListWidgetItem(manifest.id, m_matchList);
        item->setData(Qt::UserRole, manifest.id);
    }

    if (matched.isEmpty()) {
        statusBar()->showMessage(QStringLiteral("No matching plugins"), 3000);
        return;
    }

    m_matchList->setCurrentRow(0);
    statusBar()->showMessage(QStringLiteral("%1 match(es)").arg(matched.size()), 3000);
    onInvokeSelected();
}

void MainWindow::onInvokeSelected()
{
    if (!m_host) {
        return;
    }

    QListWidgetItem *item = m_matchList->currentItem();
    if (!item) {
        return;
    }

    const QString pluginId = item->data(Qt::UserRole).toString();
    const auto ctx = selectum::FakeSelectionInjector::makeContext(m_textEdit->text());

    m_fakeButton->setEnabled(false);
    m_invokeButton->setEnabled(false);
    statusBar()->showMessage(QStringLiteral("Invoking %1…").arg(pluginId));

    selectum::invokeAsync(*m_host,
                          pluginId,
                          ctx,
                          selectum::Trigger::Selection,
                          {},
                          this,
                          [this](selectum::Result result) {
                              m_fakeButton->setEnabled(true);
                              m_invokeButton->setEnabled(m_matchList->currentItem() != nullptr);
                              showResult(result);
                          });
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
