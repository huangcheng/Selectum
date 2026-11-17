#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <core/plugin_host.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class QLineEdit;
class QListWidget;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(selectum::PluginHost *host, QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onFakeSelection();
    void onInvokeSelected();

private:
    void showResult(const selectum::Result &result);

    Ui::MainWindow *ui;
    selectum::PluginHost *m_host = nullptr;
    QLineEdit *m_textEdit = nullptr;
    QListWidget *m_matchList = nullptr;
    QPushButton *m_fakeButton = nullptr;
    QPushButton *m_invokeButton = nullptr;
};
#endif // MAINWINDOW_H
