#ifndef TABLE2CONFIG_H
#define TABLE2CONFIG_H

#include <QWidget>

namespace Ui {
class Table2Config;
}

class Table2Config : public QWidget
{
    Q_OBJECT

public:
    explicit Table2Config(QWidget *parent = nullptr);
    ~Table2Config();
    void setLocaleLanguageName(QString);
    void setLocaleLanguageNameList(QList<QString> *);
    void setVoiceType(QString);
    void setVoiceRate(float);
    void setVoicePitch(float);
    void setVoiceVolume(float);
signals:
    void configOfTable2Changed(QString,QString,float,float,float);
    void readConfig();
    void readText(QString);
private:
    Ui::Table2Config *ui;
    QString localeLanguageName;
    QList<QString> *localeLanguageNameList;
    QString voiceType;
    float voiceRate;
    float voicePitch;
    float voiceVolume;
};

#endif // TABLE2CONFIG_H
