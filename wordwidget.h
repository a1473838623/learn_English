#ifndef WORDWIDGET_H
#define WORDWIDGET_H

#include <QWidget>

namespace Ui {
class WordWidget;
}

class WordWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WordWidget(QWidget *parent = nullptr);
    ~WordWidget();
    void setWordId(int);
    void setWordName(QString);
    void setWordMeaning(QString);
    void setWordVoice_UK(QString);
    void setWordVoice_US(QString);
    void setWordRememberType(QString);
signals:
    void wordChanged();
private:
    Ui::WordWidget *ui;
    int wordId;
    QString wordName;
    QString wordMeaning;
    QString wordVoice_UK;
    QString wordVoice_US;
    QString wordRememberType;
    int blurRadius;
};

#endif // WORDWIDGET_H
