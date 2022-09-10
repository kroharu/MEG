#ifndef WORKWITHFILES_H
#define WORKWITHFILES_H

//#include "mainwindow.h"
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>


class WorkWithFiles
{
public:
    WorkWithFiles();
    bool writeData(QString path, QList<QString> data, QList<QString> conf, QList<QString> logf); // Записывает данные в файл
    QString fileDataName = "";
};

#endif // WORKWITHFILES_H
