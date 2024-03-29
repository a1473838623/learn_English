#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <table1config.h>
#include <table2config.h>
#include <wordwidget.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

//帮助文档路径C:\Qt\5.15.2\mingw81_64\bin\assistant.exe
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    //事件过滤器
    bool eventFilter(QObject *,QEvent *);
    //连接数据库
    QSqlDatabase connect_database();
    //从excel读取数据
    QVector< QVector<QString> > read_data_from_xlsx(QString filePath);

signals:
    void windowSizeChanged();

private:
    Ui::MainWindow *ui;
    Table1Config *table1config;
    Table2Config *table2config;
    WordWidget *wordWidget;

};
#endif // MAINWINDOW_H
