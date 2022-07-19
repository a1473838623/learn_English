#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_table1config.h"
#include <QLabel>
#include <QTimer>
#include <QDateTime>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QToolBar>
#include <math.h>
//添加数据库头文件
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlTableModel>
#include <QPushButton>
#include <QList>
#include <QSqlRecord>
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
    ui->centralwidget->installEventFilter(this);

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



    QToolBar *toolBar = new QToolBar(this);
    //设置toolBar初始位置位于左侧
    this->addToolBar(Qt::LeftToolBarArea,toolBar);
    toolBar->setMovable(true);
    toolBar->setFloatable(true);
    QAction *resetToolBarAction = toolBar->addAction("重置工具栏位置");
    connect(resetToolBarAction,&QAction::triggered,this,[=](){
        this->addToolBar(Qt::LeftToolBarArea,toolBar);
    });

    QSqlDatabase database = this->connect_database();

    //初始化数据库
    QSqlQuery sqlQuery;
    QString str_sql = "";
    str_sql = "create table if not exists words(id int primary key, name text, english_voice text, usa_voice text, meaning text, rememberType text, tips text)";
    sqlQuery.exec(str_sql);
    str_sql = "create table if not exists config(id int primary key, cycle_period text, daily_words_number text)";
    sqlQuery.exec(str_sql);
    str_sql = "insert into config values(1,'14 days','50')";
    sqlQuery.exec(str_sql);
    str_sql = "select * from config where id = 1";
    sqlQuery.exec(str_sql);
    QString cycle_period;
    QString daily_words_number;
    while(sqlQuery.next())
    {
        cycle_period = sqlQuery.value(1).toString();
        daily_words_number = sqlQuery.value(2).toString();
    }


    //获取需要纳入当天记忆的单词列表
//    QList<int> *word_index_list = new QList<int>();
//    QDateTime cdt =QDateTime::currentDateTime();
//    QString current_date =cdt.toString("yyyy-MM-dd");
//    str_sql = "select count(*) from words where tips = '" + current_date+ "'";
//    sqlQuery.exec(str_sql);
//    int todayWordsNumber;
//    while(sqlQuery.next())
//    {
//       todayWordsNumber = sqlQuery.value(0).toInt();
//    }


    database.close();
    QSqlTableModel *model_;    //数据库模型
    model_=new QSqlTableModel; //负责提取数据
    model_->setTable("words");//选择要输出的表名称
    model_->setEditStrategy(QSqlTableModel::OnFieldChange); //所有改变立即运用到数据库
    if(model_->select())
    {
        qDebug()<<"model设置成功";
    }
    else
    {
        qDebug()<<"model设置失败";
    }
    ui->tableView->setModel(model_);

    QMenu *fileMenu = ui->menubar->addMenu("文件");
    QMenu *configMenu = ui->menubar->addMenu("设置");
    QAction *table1_config_action = configMenu->addAction("Table1设置");
    connect(table1_config_action,&QAction::triggered,this,[=](){
        this->table1config = new Table1Config();
        table1config->setIfSaveAfterChanging(model_->editStrategy()!=QSqlTableModel::OnManualSubmit);
        emit table1config->initConfig();
        table1config->show();
        connect(table1config,&Table1Config::configOfTable1Changed,this,[=](bool ifSaveAfterChanging){
            if(ifSaveAfterChanging)
            {
                //所有改变立即运用到数据库
                model_->setEditStrategy(QSqlTableModel::OnFieldChange);
            }
            else
            {
                //所有改变都会在模型中缓存，直到调用submitAll()或revertAll()函数
                model_->setEditStrategy(QSqlTableModel::OnManualSubmit);
            }
        });
    });
    QGridLayout *pLayout = new QGridLayout();//网格布局
    pLayout->setColumnStretch(this->width()/600, 1);
    pLayout->setSpacing(14);
    QList<QPushButton*> *pBtnList = new QList<QPushButton*>();
    int buttonNum =daily_words_number.toInt();
    for(int i = 0; i < buttonNum; i++)
    {
        QPushButton *pBtn = new QPushButton();
        pBtnList->append(pBtn);
        pBtn->setText(model_->record(i).value(1).value<QString>());
        pBtn->setFixedSize(QSize(300,30));
        pLayout->addWidget(pBtn);//把按钮添加到布局控件中
    }
    ui->scrollArea->widget()->setLayout(pLayout);//把布局放置到QScrollArea的内部QWidget中

    QAction *adjustButtonToolBarAction = toolBar->addAction("调整Tab2布局");
    connect(adjustButtonToolBarAction,&QAction::triggered,this,[=](){
        for(int i = 0;i < pBtnList->size();i++)
        {
            pLayout->removeWidget(pBtnList->at(i));
        }
        pLayout->setColumnStretch(this->width()/325, 1);
        for(int i = 0;i < pBtnList->size();i++)
        {
            int btn_max_col = this->width()/325;
            pLayout->addWidget(pBtnList->at(i),i/btn_max_col,i%btn_max_col);
        }
    });

    QAction *importAction = fileMenu->addAction("导入");
    connect(importAction,&QAction::triggered,this,[=](){
        bool ifConfirmed = QMessageBox::information(this,"请耐心阅读以下提示","待导入的excel需要严格按照以下规则排列。");
        if(ifConfirmed)
        {
            QString filePath = QFileDialog::getOpenFileName(this);
            if(filePath != "")
            {
                QVector< QVector<QString> > cellValues = this->read_data_from_xlsx(filePath);
                QSqlDatabase database = this->connect_database();
                //先清空words表
                QSqlQuery sqlQuery;
                QString str_sql = "DELETE FROM words;";
                sqlQuery.exec(str_sql);
                str_sql = "update sqlite_sequence set seq=0 where name='words'";
                sqlQuery.exec(str_sql);

                for(int i = 0; i < cellValues.size(); i++)
                {
                    str_sql = "insert into words values("+QString::number(i,10)+",";
                    for(int j = 0; j < cellValues[i].size(); j++)
                    {
                        str_sql += "'"+cellValues[i][j]+"',";
                    }
                    str_sql.chop(1);
                    str_sql += ")";
                    if(!sqlQuery.exec(str_sql))
                    {
                        qDebug()<<"插入第"<< i+1 <<"行数据失败"<<str_sql;
                    }
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
    });





}

MainWindow::~MainWindow()
{
    delete ui;
}


bool MainWindow::eventFilter(QObject *obj,QEvent *ev)
{
    if(obj == ui->centralwidget)
    {
        if(ev->type()==QEvent::Resize)
        {
            emit windowSizeChanged();
            return QWidget::eventFilter(obj,ev);
        }
    }
    return QWidget::eventFilter(obj,ev);
}

QSqlDatabase MainWindow::connect_database()
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
    return database;
}

QVector< QVector<QString> > MainWindow::read_data_from_xlsx(QString filePath)
{
    QVector< QVector<QString> > cellValues;

    Document xlsxR(filePath);
    if (xlsxR.load())
    {
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

            QVector<CellLocation> clList = wsheet->getFullCells( &maxRow, &maxCol );

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

            sheetIndexNumber++;
        }
    }
    return cellValues;
}
