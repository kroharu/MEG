#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include <QObject>
#include "NIDAQmx.h"
#include <stdio.h>
#include <QFile>
#include <QBuffer>


class Measurement : public QObject
{
    Q_OBJECT

    //переменные класса

public:
    explicit Measurement(QObject *parent = nullptr);
signals:
    void finished();

public slots:
    void run(); //Основная функция измерения
    // Так как запускается в отдельном потоке, сделал функции для общения с другими формами
    void setWork(bool set); // Служит для остановки процесса измерения
    void setChannels(QList<QString> listOfCh); // Здесь передаем список каналов для измерения
    long long getQuantityOperaciones(); // здесьполучаем количество сделанных измерений
private:


};

#endif // MEASUREMENT_H
