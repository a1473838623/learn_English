#include "table1config.h"
#include "ui_table1config.h"

Table1Config::Table1Config(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Table1Config)
{
    ui->setupUi(this);
    this->setWindowTitle("Table1设置");
    ui->checkBox->setText("编辑后是否保存至数据库");
    connect(this,&Table1Config::initConfig,this,[=](){
        if(this->ifSaveAfterChanging)
        {
            ui->checkBox->setCheckState(Qt::Checked);
        }
        else
        {
            ui->checkBox->setCheckState(Qt::Unchecked);
        }
    });
    connect(ui->buttonBox,&QDialogButtonBox::accepted,this,[=](){
        emit this->configOfTable1Changed(ui->checkBox->isChecked());
        this->ifSaveAfterChanging = ui->checkBox->isChecked();
        this->close();
    });
    connect(ui->buttonBox,&QDialogButtonBox::rejected,this,[=](){
        this->close();
    });
}

Table1Config::~Table1Config()
{
    delete ui;
}

void Table1Config::setIfSaveAfterChanging(bool flag)
{
    this->ifSaveAfterChanging = flag;
}
