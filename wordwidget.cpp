#include "wordwidget.h"
#include "ui_wordwidget.h"
#include <QTextToSpeech>
#include <QSqlQuery>
#include <QDebug>
#include <QClipboard>
#include <QGraphicsBlurEffect>
#include <QTimer>

WordWidget::WordWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WordWidget)
{
    ui->setupUi(this);
    QTextToSpeech  *tts = new QTextToSpeech(this);
    QSqlQuery sqlQuery;
    QString str_sql = "select * from config where id = 1";
    sqlQuery.exec(str_sql);
    float table2_config_voiceRate = 0.0;
    float table2_config_voicePitch = 0.0;
    float table2_config_voiceVolume = 1.0;
    while(sqlQuery.next())
    {
        table2_config_voiceRate = sqlQuery.value(6).toFloat();
        table2_config_voicePitch = sqlQuery.value(7).toFloat();
        table2_config_voiceVolume = sqlQuery.value(8).toFloat();
    }
    tts->setRate(table2_config_voiceRate);
    tts->setPitch(table2_config_voicePitch);
    tts->setVolume(table2_config_voiceVolume);


    // fix me. 理想状况应该是为英音和美音各设置一个窗口，可以选择每种声音
    connect(ui->pushButton_4,&QPushButton::clicked,this,[=](){
        tts->setLocale(QLocale("en_UK"));
        QList<QVoice> availableVoicesList = tts->availableVoices().toList();
        int voiceTypeIndex = 0;
        for(int i =0 ; i < availableVoicesList.size();i++)
        {
            if("Microsoft Hazel Desktop" == availableVoicesList.at(i).name())
            {
                voiceTypeIndex = i;
            }
        }
        tts->setVoice(availableVoicesList.at(voiceTypeIndex));
        tts->say(this->wordName);
    });
    connect(ui->pushButton_5,&QPushButton::clicked,this,[=](){
        tts->setLocale(QLocale("en_US"));
        QList<QVoice> availableVoicesList = tts->availableVoices().toList();
        int voiceTypeIndex = 0;
        for(int i =0 ; i < availableVoicesList.size();i++)
        {
            if("Microsoft Zira Desktop" == availableVoicesList.at(i).name())
            {
                voiceTypeIndex = i;
            }
        }
        tts->setVoice(availableVoicesList.at(voiceTypeIndex));
        tts->say(this->wordName);
    });


    connect(ui->pushButton,&QPushButton::clicked,this,[=](){
        QSqlQuery sqlQuery;
        QString str_sql = "update words set rememberType = '0' where id ="+QString::number(this->wordId,10);
        sqlQuery.exec(str_sql);
    });
    connect(ui->pushButton_2,&QPushButton::clicked,this,[=](){
        QSqlQuery sqlQuery;
        QString str_sql = "update words set rememberType = '0.9' where id ="+QString::number(this->wordId,10);
        sqlQuery.exec(str_sql);
    });
    connect(ui->pushButton_3,&QPushButton::clicked,this,[=](){
        QSqlQuery sqlQuery;
        QString str_sql = "update words set rememberType = '1' where id ="+QString::number(this->wordId,10);
        sqlQuery.exec(str_sql);
    });

    connect(ui->pushButton_6,&QPushButton::clicked,this,[=](){
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(this->wordName);
    });

    QGraphicsBlurEffect *blureffect = new QGraphicsBlurEffect();
    connect(this,&WordWidget::wordChanged,this,[=](){
        this->setWindowTitle(this->wordName);
        ui->label->setText(this->wordName);
        ui->label_2->setText(this->wordMeaning);
        this->blurRadius = 50;
        blureffect->setBlurRadius(blurRadius);	//数值越大，越模糊
        ui->label_2->setGraphicsEffect(blureffect);//设置模糊特效
        //右侧状态栏每隔100毫秒更新显示当前时间
        QTimer *timer1 = new QTimer(ui->label_2);
        connect(timer1,&QTimer::timeout,ui->label_2,[=](){
            if(this->blurRadius >0)
            {
                this->blurRadius = this->blurRadius - 1;
            }
            blureffect->setBlurRadius(this->blurRadius);
            if(this->blurRadius <= 0)
            {
                timer1->stop();
            }
        });
        timer1->start(17);
        QString wordRememberType = this->wordRememberType;
        if(wordRememberType == "0")
        {
            ui->label_3->setText("This is a new word.");
        }
        else if(wordRememberType == "0.5")
        {
            ui->label_3->setText("This is a old word.");
        }
        else if(wordRememberType == "0.9")
        {
            ui->label_3->setText("You have remembered this word today.");
        }
        else if(wordRememberType == "1")
        {
            ui->label_3->setText("You don't have to remember this word pointedly.");
        }
        ui->pushButton_4->setText(this->wordVoice_UK);
        ui->pushButton_5->setText(this->wordVoice_US);
    });
}

WordWidget::~WordWidget()
{
    delete ui;
}

void WordWidget::setWordId(int wordId){
    this->wordId = wordId;
}
void WordWidget::setWordName(QString wordName){
    this->wordName = wordName;
}
void WordWidget::setWordMeaning(QString wordMeaning){
    this->wordMeaning = wordMeaning;
}
void WordWidget::setWordVoice_UK(QString wordVoice_UK){
    this->wordVoice_UK = wordVoice_UK;
}
void WordWidget::setWordVoice_US(QString wordVoice_US){
    this->wordVoice_US = wordVoice_US;
}
void WordWidget::setWordRememberType(QString wordRememberType){
    this->wordRememberType = wordRememberType;
}
