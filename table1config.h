#ifndef TABLE1CONFIG_H
#define TABLE1CONFIG_H

#include <QWidget>

namespace Ui {
class Table1Config;
}

class Table1Config : public QWidget
{
    Q_OBJECT

public:
    explicit Table1Config(QWidget *parent = nullptr);
    ~Table1Config();
    void setIfSaveAfterChanging(bool flag);
signals:
    void configOfTable1Changed(bool ifSaveAfterChanging);
    void initConfig();
private:
    Ui::Table1Config *ui;
    bool ifSaveAfterChanging;
};

#endif // TABLE1CONFIG_H
