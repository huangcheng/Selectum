#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <core/plugin_host.h>
#include <platform/selection_monitor.h>
#include <selectum/abi/types.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(selectum::PluginHost *host,
                        selectum::SelectionMonitor *monitor,
                        QWidget *parent = nullptr);
    ~MainWindow() override;

public slots:
    void onSelectionSettled(const selectum::SelectionContext &context);

private slots:
    void onFakeSelection();
    void onInvokeSelected();
    void onRequestAccessibility();
    void refreshPermissionLabel();

private:
    void applyContext(const selectum::SelectionContext &context, bool autoInvoke);
    void showResult(const selectum::Result &result);

    Ui::MainWindow *ui;
    selectum::PluginHost *m_host = nullptr;
    selectum::SelectionMonitor *m_monitor = nullptr;
    selectum::SelectionContext m_lastContext;
    bool m_hasContext = false;

    QLineEdit *m_textEdit = nullptr;
    QListWidget *m_matchList = nullptr;
    QPushButton *m_fakeButton = nullptr;
    QPushButton *m_invokeButton = nullptr;
    QLabel *m_permissionLabel = nullptr;
    QPushButton *m_permissionButton = nullptr;
};
#endif // MAINWINDOW_H
