#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLabel>
#include <QTimer>
#include <QDateTime>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QToolBar>
//添加数据库头文件
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlTableModel>
using namespace std;

#include "xlsxdocument.h"
#include "xlsxchartsheet.h"
#include "xlsxcellrange.h"
#include "xlsxchart.h"
#include "xlsxrichstring.h"
#include "xlsxworkbook.h"
using namespace QXlsx;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{ 
    ui->setupUi(this);

    this->setWindowTitle("背单词软件");

    //左侧状态栏
    QLabel *label = new QLabel("designed by yangqifan",this);
    ui->statusbar->addWidget(label);

    //右侧状态栏
    QLabel *label2 = new QLabel("",this);
    ui->statusbar->addPermanentWidget(label2);
    //右侧状态栏每隔100毫秒更新显示当前时间
    QTimer *timer1 = new QTimer(label2);
    connect(timer1,&QTimer::timeout,label2,[=](){
        QDateTime cdt =QDateTime::currentDateTime();
        QString current_date_time =cdt.toString("yyyy.MM.dd hh:mm:ss");
        label2->setText(current_date_time);
    });
    timer1->start(100);

    QMenu *fileMenu = ui->menubar->addMenu("文件");
    QAction *importAction = fileMenu->addAction("导入");

    //为了使toolBar初始位置位于左侧，手动创建toolBar
    QToolBar *toolBar = new QToolBar(this);
    this->addToolBar(Qt::LeftToolBarArea,toolBar);
    toolBar->setMovable(true);
    toolBar->setFloatable(true);
    QAction *resetToolBarAction = toolBar->addAction("重置工具栏位置");
    connect(resetToolBarAction,&QAction::triggered,this,[=](){
        this->addToolBar(Qt::LeftToolBarArea,toolBar);
    });

    QSqlDatabase database;
    if (QSqlDatabase::contains("qt_sql_default_connection"))
    {
        database = QSqlDatabase::database("qt_sql_default_connection");
        qDebug() << "连接已存在";
    }
    else
    {
        //若不存在数据库连接，则创建连接
        database = QSqlDatabase::addDatabase("QSQLITE");
        database.setDatabaseName("MyDataBase.db");
        database.setUserName("admin");
        database.setPassword("123456");
        qDebug() << "创建新连接";
    }
    //打开数据库
    bool ifOpened = database.open();
    if(!ifOpened)
    {
        QMessageBox::critical(this,"打开数据库失败",database.lastError().text());
    }
    else
    {
        qDebug() << "打开数据库成功";
    }
    QSqlQuery sqlQuery;
    QString str_sql = "";
    str_sql = "create table if not exists words(id int primary key, name text, english_voice text, usa_voice text, meaning text, rememberType text, tips text)";
    sqlQuery.exec(str_sql);
    database.close();
    QSqlTableModel *model_;    //数据库模型
    model_=new QSqlTableModel; //负责提取数据
    model_->setTable("words");//选择要输出的表名称
    if(model_->select())
    {
        qDebug()<<"model设置成功";
    }
    else
    {
        qDebug()<<"model设置失败";
    }
    ui->tableView->setModel(model_);

    connect(importAction,&QAction::triggered,this,[=](){
        bool ifConfirmed = QMessageBox::information(this,"请耐心阅读以下提示","待导入的excel需要严格按照以下规则排列。");
        if(ifConfirmed)
        {
            QString filePath = QFileDialog::getOpenFileName(this);
            if(filePath != "")
            {
                Document xlsxR(filePath);
                if (xlsxR.load())
                {
                    QSqlDatabase database;
                    if (QSqlDatabase::contains("qt_sql_default_connection"))
                    {
                        database = QSqlDatabase::database("qt_sql_default_connection");
                        qDebug() << "连接已存在";
                    }
                    else
                    {
                        //若不存在数据库连接，则创建连接
                        database = QSqlDatabase::addDatabase("QSQLITE");
                        database.setDatabaseName("MyDataBase.db");
                        database.setUserName("admin");
                        database.setPassword("123456");
                        qDebug() << "创建新连接";
                    }
                    //打开数据库
                    bool ifOpened = database.open();
                    if(!ifOpened)
                    {
                        QMessageBox::critical(this,"打开数据库失败",database.lastError().text());
                    }
                    else
                    {
                        qDebug() << "打开数据库成功";
                    }

                    //先清空words表
                    QSqlQuery sqlQuery;
                    QString str_sql = "DELETE FROM words;";
                    sqlQuery.exec(str_sql);
                    str_sql = "update sqlite_sequence set seq=0 where name='words'";
                    sqlQuery.exec(str_sql);

                    int sheetIndexNumber = 0;
                    foreach( QString currentSheetName, xlsxR.sheetNames() )
                    {
                        // get current sheet
                        AbstractSheet* currentSheet = xlsxR.sheet( currentSheetName );
                        if ( NULL == currentSheet )
                            continue;

                        // get full cells of current sheet
                        int maxRow = -1;
                        int maxCol = -1;
                        currentSheet->workbook()->setActiveSheet( sheetIndexNumber );
                        Worksheet* wsheet = (Worksheet*) currentSheet->workbook()->activeSheet();
                        if ( NULL == wsheet )
                            continue;

                        QString strSheetName = wsheet->sheetName(); // sheet name
                        qDebug() << strSheetName;

                        QVector<CellLocation> clList = wsheet->getFullCells( &maxRow, &maxCol );
                        QVector< QVector<QString> > cellValues;
                        for (int rc = 0; rc < maxRow; rc++)
                        {
                            QVector<QString> tempValue;
                            for (int cc = 0; cc < maxCol; cc++)
                            {
                                tempValue.push_back(QString(""));
                            }
                            cellValues.push_back(tempValue);
                        }

                        for ( int ic = 0; ic < clList.size(); ++ic )
                        {
                            CellLocation cl = clList.at(ic); // cell location
                            int row = cl.row - 1;
                            int col = cl.col - 1;

                            QSharedPointer<Cell> ptrCell = cl.cell; // cell pointer

                            // value of cell
                            QVariant var = cl.cell.data()->value();
                            QString str = var.toString();

                            cellValues[row][col] = str;
                        }
                        // print
                        for (int rc = 1; rc < maxRow; rc++)
                        {
                            for (int cc = 0; cc < maxCol; cc++)
                            {
                                QString strCell = cellValues[rc][cc];
                                qDebug() << "( row : " << rc
                                         << ", col : " << cc
                                         << ") " << strCell; // display cell value
                            }
                            QSqlQuery sqlQuery;
                            QString str_sql = "";
                            str_sql = QString("insert into words values(%1,'%2','%3','%4','%5','%6','%7')")
                                    .arg(QString::number(rc,10),cellValues[rc][0],cellValues[rc][1],cellValues[rc][2],cellValues[rc][3],cellValues[rc][4],cellValues[rc][5]);
                            if(!sqlQuery.exec(str_sql))
                            {
                                qDebug()<<"插入第"<< rc+1 <<"行数据失败"<<str_sql;
                            }
                        }
                        sheetIndexNumber++;
                    }
                    database.close();
                    QSqlTableModel *model_;    //数据库模型
                    model_=new QSqlTableModel; //负责提取数据
                    model_->setTable("words");//选择要输出的表名称
                    if(model_->select())
                    {
                        qDebug()<<"model设置成功";
                    }
                    else
                    {
                        qDebug()<<"model设置失败";
                    }
                    ui->tableView->setModel(model_);
                }
            }
        }
    });

}

MainWindow::~MainWindow()
{
    delete ui;
}
