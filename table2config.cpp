#include "table2config.h"
#include "ui_table2config.h"
#include <QTextToSpeech>
#include <QInputDialog>
#include <QDebug>

Table2Config::Table2Config(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Table2Config)
{
    ui->setupUi(this);
    this->setWindowTitle("Table2设置");
    ui->label->setText("语言类型");
    ui->pushButton->setText("添加语言类型");
    connect(ui->pushButton,&QPushButton::clicked,this,[=](){
        bool flag;
        // 获取字符串
        QString inputString = QInputDialog::getText(this, "添加语言", "请输入一种语言（格式为zh_CN）：", QLineEdit::Normal,"en_UK", &flag);
        if(flag && ui->comboBox->findText(inputString) == -1)
        {
            ui->comboBox->addItem(inputString);
        }
    });

    ui->label_2->setText("语音类型");
    ui->label_3->setText("语速");
    ui->label_4->setText("音高");
    ui->label_5->setText("音量");

    QTextToSpeech  *tts = new QTextToSpeech(this);
    connect(ui->comboBox,&QComboBox::currentTextChanged,ui->comboBox_2,[=](){
        if(ui->comboBox->currentText() != "")
        {
            tts->setLocale(QLocale(ui->comboBox->currentText()));
            QList<QVoice> availableVoicesList = tts->availableVoices().toList();
            ui->comboBox_2->clear();
            for(int i =0 ; i < availableVoicesList.size();i++)
            {
                ui->comboBox_2->addItem(availableVoicesList.at(i).name());
            }
        }
    });
    connect(this,&Table2Config::readConfig,this,[=](){
        ui->comboBox->clear();
        for(int i = 0; i < this->localeLanguageNameList->size(); i++)
        {
            ui->comboBox->addItem(this->localeLanguageNameList->at(i));
        }
        if(ui->comboBox->findText(this->localeLanguageName) == -1)
        {
            ui->comboBox->addItem(this->localeLanguageName);
        }
        ui->comboBox->setCurrentText(this->localeLanguageName);
        ui->comboBox_2 ->setCurrentText(this->voiceType);
        ui->doubleSpinBox->setValue(this->voiceRate);
        ui->doubleSpinBox_2->setValue(this->voicePitch);
        ui->doubleSpinBox_3->setValue(this->voiceVolume);
    });
    connect(ui->buttonBox,&QDialogButtonBox::accepted,this,[=](){
        this->localeLanguageName = ui->comboBox->currentText();
        this->voiceType = ui->comboBox_2->currentText();
        this->voiceRate = (float)ui->doubleSpinBox->value();
        this->voicePitch = (float)ui->doubleSpinBox_2->value();
        this->voiceVolume = (float)ui->doubleSpinBox_3->value();
        emit this->configOfTable2Changed(ui->comboBox->currentText(),ui->comboBox_2->currentText(),
                                         (float)ui->doubleSpinBox->value(),(float)ui->doubleSpinBox_2->value(),
                                         (float)ui->doubleSpinBox_3->value());
        this->close();
    });
    connect(ui->buttonBox,&QDialogButtonBox::rejected,this,[=](){
        this->close();
    });

    ui->pushButton_2->setText("保存设置");
    connect(ui->pushButton_2,&QPushButton::clicked,this,[=](){
        this->localeLanguageName = ui->comboBox->currentText();
        this->voiceType = ui->comboBox_2->currentText();
        this->voiceRate = (float)ui->doubleSpinBox->value();
        this->voicePitch = (float)ui->doubleSpinBox_2->value();
        this->voiceVolume = (float)ui->doubleSpinBox_3->value();
        emit this->configOfTable2Changed(ui->comboBox->currentText(),ui->comboBox_2->currentText(),
                                         (float)ui->doubleSpinBox->value(),(float)ui->doubleSpinBox_2->value(),
                                         (float)ui->doubleSpinBox_3->value());
    });

    ui->pushButton_3->setText("语音朗诵测试");
    connect(ui->pushButton_3,&QPushButton::clicked,this,[=](){
        bool flag;
        // 获取字符串
        QString inputString = QInputDialog::getText(this, "语音朗诵测试", "请输入一段话或一个单词以进行朗诵：", QLineEdit::Normal,"Lieutenant", &flag);
        if(flag && ui->comboBox->findText(inputString) == -1)
        {
            emit readText(inputString);
        }
    });
}

Table2Config::~Table2Config()
{
    delete ui;
}

void Table2Config::setLocaleLanguageName(QString localeLanguageName)
{
    this->localeLanguageName = localeLanguageName;
}
void Table2Config::setLocaleLanguageNameList(QList<QString> *localeLanguageNameList){
    this->localeLanguageNameList = localeLanguageNameList;
}
void Table2Config::setVoiceType(QString voiceType){
    this->voiceType = voiceType;
}
void Table2Config::setVoiceRate(float voiceRate){
    this->voiceRate =  voiceRate;
}
void Table2Config::setVoicePitch(float voicePitch){
    this->voicePitch = voicePitch;
}
void Table2Config::setVoiceVolume(float voiceVolume){
    this->voiceVolume =  voiceVolume;
}
