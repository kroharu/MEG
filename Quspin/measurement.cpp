#include "measurement.h"
#include "NIDAQmx.h"
#include <QDebug>
#include <QFile>
#include <QApplication>
#include <QDataStream>
#include <QBuffer>

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
bool work = true;
QFile rawFile;
// Создаем поток, куда будем записывать данные
QDataStream rawStreamData(&rawFile);
QList<QString> listOfChannels; // Лист каналов
int quantityChannels;
long long quantityOperaciones;



Measurement::Measurement(QObject *parent) : QObject(parent)
{

}

void Measurement::run()
{
    quantityOperaciones = 0;
    //Создаем файл для "сырых данных"
    rawFile.setFileName(QApplication::applicationDirPath() + "/data.raw");
    rawFile.open(QFile::Append);
    //Здесь запускаем сами измерения
    TaskHandle  taskHandle=0;
    DAQmxCreateTask("",&taskHandle);
    quantityChannels = 0;
    for (int i = 0; i < listOfChannels.size(); i++)
    {
        DAQmxCreateAIVoltageChan(taskHandle, listOfChannels[i].toLocal8Bit(), "", DAQmx_Val_RSE, -5.0, 5.0, DAQmx_Val_Volts, NULL);
        QList<QString> list = listOfChannels[i].split(", ");
         quantityChannels += list.size();
    }

    // 1000 - Sample Rate, 1000 - размер буфера,
    DAQmxCfgSampClkTiming(taskHandle,"",1000.0,DAQmx_Val_Rising,DAQmx_Val_ContSamps,1000);

    // 1000 - кол-во выборок, при котором срабатывает вызов callback функции
    DAQmxRegisterEveryNSamplesEvent(taskHandle,DAQmx_Val_Acquired_Into_Buffer,1000,0,EveryNCallback,NULL);
    DAQmxStartTask(taskHandle);

    emit finished();
}

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
    int32       read=0;
    float64     data[64000]; // поправить на количество каналов

    DAQmxReadAnalogF64(taskHandle,-1,10.0,DAQmx_Val_GroupByScanNumber,data,1000 * quantityChannels ,&read,NULL);

    // Здесь данные надо складывать в файл
//    qDebug() << read;
    for (long long z = 0; z < read * quantityChannels; z++)
     {
        rawStreamData << data[z];
     }
    quantityOperaciones += read;

    if (!work)
    {
        DAQmxStopTask(taskHandle);
        DAQmxClearTask(taskHandle);
        rawFile.close();
//        qDebug() << "STOP";
        return 0;
    }
    return 0;
}

void Measurement::setWork(bool set)
{
    work = set;
}

void Measurement::setChannels(QList<QString> listOfCh)
{
    listOfChannels = listOfCh;
}

long long Measurement::getQuantityOperaciones()
{
    return quantityOperaciones;
}

