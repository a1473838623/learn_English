#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

//帮助文档路径C:\Qt\6.2.4\mingw_64\bin\assistant.exe
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    bool eventFilter(QObject *,QEvent *);

signals:
    void windowSizeChanged();

private:
    Ui::MainWindow *ui;

};
#endif // MAINWINDOW_H
