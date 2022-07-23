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
#include <QTextToSpeech>
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
    QAction *adjustButtonToolBarAction = toolBar->addAction("调整Tab2布局");

    ui->label->setText("已导入的单词库");

    QSqlDatabase database = this->connect_database();

    //初始化数据库
    QSqlQuery sqlQuery;
    QString str_sql = "";
    str_sql = "create table if not exists words(id int primary key, name text, english_voice text, usa_voice text, meaning text, rememberType text, tips text)";
    sqlQuery.exec(str_sql);
    str_sql = "create table if not exists config(id int primary key, cycle_period text, daily_words_number text, table1_config_ifSaveAfterChanging text, table2_config_localeLanguageName text,table2_config_voiceType text, table2_config_voiceRate text, table2_config_voicePitch text, table2_config_voiceVolume text)";
    sqlQuery.exec(str_sql);
    str_sql = "insert into config values(1,'14','50','true','en_UK','Halen','0.5','0.5','0.5')";
    sqlQuery.exec(str_sql);
    str_sql = "select * from config where id = 1";
    sqlQuery.exec(str_sql);
    int cycle_period = 14;
    int daily_words_number = 50;
    QString table1_config_ifSaveAfterChanging = "true";
    QString table2_config_localeLanguageName = "en_UK";
    QString table2_config_voiceType = "";
    float table2_config_voiceRate = 0.0;
    float table2_config_voicePitch = 0.0;
    float table2_config_voiceVolume = 1.0;
    while(sqlQuery.next())
    {
        cycle_period = sqlQuery.value(1).toInt();
        daily_words_number = sqlQuery.value(2).toInt();
        table1_config_ifSaveAfterChanging = sqlQuery.value(3).toString();
        table2_config_localeLanguageName = sqlQuery.value(4).toString();
        table2_config_voiceType = sqlQuery.value(5).toString();
        table2_config_voiceRate = sqlQuery.value(6).toFloat();
        table2_config_voicePitch = sqlQuery.value(7).toFloat();
        table2_config_voiceVolume = sqlQuery.value(8).toFloat();
    }

    //获取需要纳入当天记忆的单词id列表
    QList<int> *word_index_list = new QList<int>();

    QDateTime cdt =QDateTime::currentDateTime();
    QString current_date =cdt.toString("yyyy.MM.dd");
    //首先清空今天之前rememberType = 0的单词的tips
    str_sql = "update words set tips = '' where tips != '' and tips != '"+current_date + "' and rememberType = '0'";
    sqlQuery.exec(str_sql);
    //将不是今天的rememberType为0.9的改为0.5
    str_sql = "update words set rememberType = '0.5' where tips != '"+current_date + "' and rememberType = '0.9'";
    sqlQuery.exec(str_sql);
    str_sql = "select count(*) from words where tips = '"+ current_date + "' and rememberType != '0.5'";
    sqlQuery.exec(str_sql);
    int today_words = 0;
    while(sqlQuery.next())
    {
        today_words = sqlQuery.value(0).toInt();
    }



    if(today_words < daily_words_number)
    {
        //今日单词数不够，还需要加。
        str_sql = "select * from words where tips = '' and rememberType = '0' limit " +  QString::number(daily_words_number-today_words);
        sqlQuery.exec(str_sql);
        while(sqlQuery.next())
        {
            QSqlQuery sqlQuery2;
            QString str_sql2 = "update words set tips = '"+ current_date + "' where id = " + QString::number(sqlQuery.value(0).toInt());
            sqlQuery2.exec(str_sql2);
        }
    }
    else if(today_words > daily_words_number) //需要去除一些单词
    {

        str_sql = "select count(*) from words where tips = '"+ current_date + "' and rememberType = '0'";
        sqlQuery.exec(str_sql);
        int today_not_start_number = 0;
        while(sqlQuery.next())
        {
            today_not_start_number = sqlQuery.value(0).toInt();
        }
        int number_to_minus;
        if(today_not_start_number >= today_words - daily_words_number)
        {
            number_to_minus = today_words - daily_words_number;
        }
        else
        {
            number_to_minus = today_not_start_number;
        }
        //将多余标记为0的单词的tips标记为空。
        str_sql = "update words set tips = '' where id in (select id from words where tips = '"+ current_date + "' and rememberType = '0' limit "+ QString::number(number_to_minus)+")";
        sqlQuery.exec(str_sql);
    }

    //将循环到今天或循环到今天之前的单词的tips设置为今天
    QDateTime today = QDateTime::fromString(current_date,"yyyy.MM.dd");
    str_sql = "update words set tips = '"+ current_date +"' where rememberType = '0.5' and tips not in (";
    for(int i = today.toSecsSinceEpoch();i >= today.toSecsSinceEpoch()-cycle_period*24*3600;i = i - 24*3600)
    {
        QDateTime date = QDateTime::fromSecsSinceEpoch(i);
        str_sql += "'" + date.toString("yyyy.MM.dd") + "',";
    }
    str_sql.chop(1);
    str_sql += ")";
    sqlQuery.exec(str_sql);

    //依次纳入rememberType为 0 和 0.5的单词 0.9表示当天记住的单词，0.5表示非今天记住的单词，0表示从来没有记忆过的单词，1表示已熟记之后也不会复习
    str_sql = "select * from words where tips = '"+ current_date + "' and rememberType = '0'";
    sqlQuery.exec(str_sql);
    while(sqlQuery.next())
    {
        word_index_list->append(sqlQuery.value(0).toInt());
    }
    str_sql = "select * from words where tips = '"+ current_date + "' and rememberType = '0.5'";
    sqlQuery.exec(str_sql);
    while(sqlQuery.next())
    {
        word_index_list->append(sqlQuery.value(0).toInt());
    }


    QSqlTableModel *model_;    //数据库模型
    model_=new QSqlTableModel; //负责提取数据
    model_->setTable("words");//选择要输出的表名称
    if("true" == table1_config_ifSaveAfterChanging )
    {
        model_->setEditStrategy(QSqlTableModel::OnFieldChange); //所有改变立即运用到数据库
    }
    else
    {
        //所有改变都会在模型中缓存，直到调用submitAll()或revertAll()函数
        model_->setEditStrategy(QSqlTableModel::OnManualSubmit);
    }
    model_->select();
    ui->tableView->setModel(model_);

    QMenu *fileMenu = ui->menubar->addMenu("文件");
    QMenu *configMenu = ui->menubar->addMenu("设置");
    QAction *table1_config_action = configMenu->addAction("Table1设置");
    QAction *table2_config_action = configMenu->addAction("Table2设置");
    this->table1config = new Table1Config();
    this->table1config->setIfSaveAfterChanging(model_->editStrategy()!=QSqlTableModel::OnManualSubmit);
    connect(table1_config_action,&QAction::triggered,this,[=](){
        emit table1config->readConfig();
        table1config->show();
        connect(table1config,&Table1Config::configOfTable1Changed,this,[=](bool ifSaveAfterChanging){
            QString flagString = "false";
            if(ifSaveAfterChanging)
            {
                //所有改变立即运用到数据库
                model_->setEditStrategy(QSqlTableModel::OnFieldChange);
                flagString = "true";
            }
            else
            {
                //所有改变都会在模型中缓存，直到调用submitAll()或revertAll()函数
                model_->setEditStrategy(QSqlTableModel::OnManualSubmit);
            }
            QSqlQuery sqlQuery;
            QString str_sql = "update config set table1_config_ifSaveAfterChanging = '"+ flagString +"' where id = 1";
            sqlQuery.exec(str_sql);
        });
    });


    QTextToSpeech  *tts = new QTextToSpeech(this);

    tts->setRate(table2_config_voiceRate);
    tts->setPitch(table2_config_voicePitch);
    tts->setVolume(table2_config_voiceVolume);
    tts->setLocale(QLocale(table2_config_localeLanguageName));

    QList<QString> *localeLanguageNameList = new QList<QString>();
    QList<QLocale> availableLocaleList = tts->availableLocales().toList();
    for(int i =0 ; i < availableLocaleList.size();i++)
    {
        localeLanguageNameList->append(availableLocaleList.at(i).name());
    }


    QList<QVoice> availableVoicesList = tts->availableVoices().toList();
    int voiceTypeIndex = 0;
    for(int i =0 ; i < availableVoicesList.size();i++)
    {
        if(table2_config_voiceType == availableVoicesList.at(i).name())
        {
            voiceTypeIndex = i;
        }
    }
    tts->setVoice(availableVoicesList.at(voiceTypeIndex));

    this->table2config = new Table2Config();
    this->table2config->setLocaleLanguageName(table2_config_localeLanguageName);
    this->table2config->setLocaleLanguageNameList(localeLanguageNameList);
    this->table2config->setVoiceType(table2_config_voiceType);
    this->table2config->setVoiceRate(table2_config_voiceRate);
    this->table2config->setVoicePitch(table2_config_voicePitch);
    this->table2config->setVoiceVolume(table2_config_voiceVolume);
    connect(this->table2config,&Table2Config::readText,this,[=](QString textToRead){
        if(tts->state()==QTextToSpeech::Ready)
        {
            tts->say(textToRead);
        }
    });
    connect(table2_config_action,&QAction::triggered,this,[=](){
        emit table2config->readConfig();
        table2config->show();
        connect(table2config,&Table2Config::configOfTable2Changed,this,[=](QString localeLanguageName,
                QString voiceType, float voiceRate, float voicePitch, float voiceVolume){
            tts->setLocale(QLocale(localeLanguageName));
            int voiceTypeIndex = 0;
            for(int i =0 ; i < availableVoicesList.size();i++)
            {
                if(voiceType == availableVoicesList.at(i).name())
                {
                    voiceTypeIndex = i;
                }
            }
            tts->setVoice(availableVoicesList.at(voiceTypeIndex));
            tts->setRate(voiceRate);
            tts->setPitch(voicePitch);
            tts->setVolume(voiceVolume);
            QSqlQuery sqlQuery;
            QString str_sql = "update config set table2_config_localeLanguageName = '"+
                    localeLanguageName + "', table2_config_voiceType = '"+ voiceType
                    +"', table2_config_voiceRate = '"+ QString::number(voiceRate,'f',1)
                    +"', table2_config_voicePitch = '"+ QString::number(voicePitch,'f',1)
                    +"', table2_config_voiceVolume = '"+ QString::number(voiceVolume,'f',1) +"' where id = 1";
            sqlQuery.exec(str_sql);
        });
    });

    QGridLayout *pLayout = new QGridLayout();//网格布局
    pLayout->setColumnStretch(this->width()/600, 1);
    pLayout->setSpacing(14);
    QList<QPushButton*> *pBtnList = new QList<QPushButton*>();
    for(int i = 0; i < word_index_list->size(); i++)
    {
        QPushButton *pBtn = new QPushButton();
        pBtnList->append(pBtn);
        // record(row) ，row最小为0，等于id -1。
        QString word = model_->record(word_index_list->at(i)-1).value(1).value<QString>();
        pBtn->setText(QString::number(i+1) +" " + word);
        pBtn->setFixedSize(QSize(300,30));
        pLayout->addWidget(pBtn);//把按钮添加到布局控件中

        connect(pBtn,&QPushButton::clicked,this,[=](){
            tts->say(word);
        });
    }
    ui->scrollArea->widget()->setLayout(pLayout);//把布局放置到QScrollArea的内部QWidget中


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
                //先清空words表
                QSqlQuery sqlQuery;
                QString str_sql = "DELETE FROM words";
                sqlQuery.exec(str_sql);
                QList<int> *insertFailedLineNumberList = new QList<int>();
                for(int i = 1; i < cellValues.size(); i++)
                {
                    str_sql = "INSERT INTO words VALUES ("+QString::number(i)+",";
                    for (int j = 0; j < cellValues[i].size(); j++)
                    {
                        str_sql += "?,";
                    }
                    str_sql.chop(1);
                    str_sql += ")";
                    sqlQuery.prepare(str_sql);
                    for (int j = 0; j < cellValues[i].size(); j++)
                    {
                        sqlQuery.addBindValue(cellValues[i][j]);
                    }
                    if(!sqlQuery.exec())
                    {
                        insertFailedLineNumberList->append(i+1);
                    }
                }
                if(insertFailedLineNumberList->size() > 0)
                {
                    QString failureText = "第";
                    for(int i = 0;i <insertFailedLineNumberList->size(); i++ )
                    {
                        failureText += QString::number(insertFailedLineNumberList->at(i),10) + ",";
                    }
                    failureText.chop(1);
                    failureText += "行数据插入失败";
                    QMessageBox::critical(this,"部分或全部数据插入失败",failureText);
                }
                model_->select();
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
    //若存在连接，直接返回原来的连接，故不会同时存在大量连接，本程序第一次连接后也就一直没有必要调用close()。
    if (QSqlDatabase::contains("qt_sql_default_connection"))
    {
        database = QSqlDatabase::database("qt_sql_default_connection");
    }
    else
    {
        //若不存在数据库连接，则创建连接
        database = QSqlDatabase::addDatabase("QSQLITE");
        database.setDatabaseName("MyDataBase.db");
        database.setUserName("admin");
        database.setPassword("123456");
    }
    //打开数据库
    bool ifOpened = database.open();
    if(!ifOpened)
    {
        QMessageBox::critical(this,"打开数据库失败",database.lastError().text());
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
