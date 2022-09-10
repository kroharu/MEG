#include "cplot.h"
#include "ui_cplot.h"
#include "setting.h"
#include "NIDAQmx.h"
#include "qcustomplot.h"
#include "Dsp.h"
#include "selectchart.h"


CPlot::CPlot(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CPlot)
{
    ui->setupUi(this);
    listChannelsForCharts = getListNumberChannelsForChart();
    loadSettings();
    timerChart = new QTimer(this);
    timerChart->setTimerType(Qt::CoarseTimer);
    timerChart->setInterval(1500); // /ui->spinBoxFrequency->text().toUInt());
    connect(timerChart, SIGNAL(timeout()), this, SLOT(ActionTimerChart()));
    customPlot = ui->customPlot;
    customPlot->legend->setVisible(true); // Разрешает вывод легенды на графике
    customPlot->xAxis->setLabel("Time, ms");
    customPlot->yAxis->setLabel("fT");
    dataFiltred[0] = new float[1000];
    filter = new Dsp::FilterDesign
      <Dsp::Butterworth::Design::BandPass <2>, 1, Dsp::DirectFormI>;
    sChart = new SelectChart();

}

CPlot::~CPlot()
{
    delete ui;
}

void CPlot::setPathForSaveGpg(QString myPath)
{
    path = myPath;
}

void CPlot::getSetGain(QString str)
{
    setGain = str;
}

void CPlot::loadSettings()
{
    QSettings *settings = new QSettings("./quspin.conf",QSettings::IniFormat );
    dimensionX = settings->value("charts/dimensionX").toInt();
    dimensionY = settings->value("charts/dimensionY").toInt();
    ui->spinBoxValueX->setValue(dimensionX);
    ui->lineEditCenterFreq->setText(settings->value("charts/CenterFreq").toString());
    ui->lineEditBandWidth->setText(settings->value("charts/BandWidth").toString());
    ui->LabelChartsQuantity->setText(QString::number(listNameOfChannelsForCharts.size()) +" plots");
}

void CPlot::saveSettings()
{
    QSettings *settings = new QSettings("./quspin.conf",QSettings::IniFormat );
     settings->setValue("charts/dimensionX", ui->spinBoxValueX->value());
     settings->setValue("charts/CenterFreq", ui->lineEditCenterFreq->text());
     settings->setValue("charts/BandWidth", ui->lineEditBandWidth->text());
     settings->sync();
}

void CPlot::on_pushButtonClose_clicked()
{
    timerChart->stop();
    ui->pushButtonShow->setText("Show");
    close();
}

void CPlot::on_pushButtonSetting_clicked()
{
    Setting *set = new Setting(this);
    set->show();
}

void CPlot::on_pushButtonRefresh_clicked()
{
    listChannelsForCharts = getListNumberChannelsForChart();
    loadSettings();
}

double CPlot::coefficient(QString coeff)
{
    double result = 0;
    if (coeff == "0.33X")
    {
        result = 1000000/0.9;
    }
    else if (coeff == "1X")
    {
        result = 1000000/2.7;
    }
    else if (coeff == "3X")
    {
        result = 1000000/8.1;
    }
    return result;
}

void CPlot::on_pushButtonShow_clicked()
{
    bool ok;
    int forColor = 0;
    QColor color[] = {QColor(127,0,0),QColor(255,0,0),QColor(0,127,0),QColor(127,127,0),QColor(255,127,0),QColor(0,255,0),
                      QColor(127,255,0),QColor(255,255,0),QColor(0,0,127),QColor(0,0,255),QColor(127,0,255),QColor(255,0,255),
                      QColor(255,127,255),QColor(0,0,0)};
    listChannelsForCharts = getListNumberChannelsForChart();
    if (ui->pushButtonShow->text() == "Show")
    {
        Dsp::Params params;
        params[0] = 1000; // sample rate
        params[1] = 2; // order
        params[2] = ui->lineEditCenterFreq->text().toFloat(&ok); //  20; // center frequency
        if (!ok)
        {
            QMessageBox::warning(this,"Warning","Field 'Center frequency' is wrong. Please  set value",1,1);
            return;
        }
        params[3] = ui->lineEditBandWidth->text().toFloat(&ok); //39.34; // band width
        if (!ok)
        {
            QMessageBox::warning(this,"Warning","Field 'Band width' is wrong. Please  set value",1,1);
            return;
        }
        saveSettings();
        filter->setParams (params);

        ui->pushButtonShow->setText("Stop");
        num = 0;
        timerChart->start();
        dimensionX = ui->spinBoxValueX->value();
        customPlot->clearGraphs();
        customPlot->yAxis->setRange(0, 20);
        customPlot->xAxis->setRange(0, dimensionX);
        customPlot->replot();
        //Очищаем старые значения
        for (int i = 0; i < listData.size(); i++)
        {
            listData[i].clear();
        }
        listData.clear();
        xD.clear();
        //**********************
        for (int z = 0; z < listChannelsForCharts.size(); z++)
        {
            customPlot->addGraph();
            QVector<double> *v = new QVector<double>;
            listData.append(*v);
            customPlot->graph(z)->setName(QString::number(z+1) + ". " + listNameOfChannelsForCharts[z]);
            QPen pen;
          // pen.setStyle(Qt::DotLine);
            pen.setWidth(2);
            pen.setColor(color[forColor]);
            customPlot->graph(z)->setPen(pen);
            forColor++;
            if (forColor > 14) forColor = 0;
        }
    } else
    {
        timerChart->stop();
        ui->pushButtonShow->setText("Show");
    }
}

void CPlot::ActionTimerChart()
{
    TaskHandle  taskHandle = 0;
    int32       read;
    float64     data[20000];
    DAQmxCreateTask("", &taskHandle);
    QList<QString> chan = getListChannels();
    float64 min;
    float64 max;

    for (int i = 0; i < chan.size(); i++)
    {
        DAQmxCreateAIVoltageChan(taskHandle, chan[i].toLocal8Bit(), "", DAQmx_Val_RSE, -5.0, 5.0, DAQmx_Val_Volts, NULL);
    }
    DAQmxCfgSampClkTiming(taskHandle, "", 1000, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, 1000);
    DAQmxStartTask(taskHandle);
    DAQmxReadAnalogF64(taskHandle, -1, 10.0, DAQmx_Val_GroupByChannel , data, listChannelsForCharts.size() * 1000, &read, NULL);
    DAQmxStopTask(taskHandle);
    DAQmxClearTask(taskHandle);

    for (int i = 0; i < listChannelsForCharts.size(); i++)
    {
        int k = i*1000; // k - это начало каждой серии для каждого канала, если канала 4 то: 1000,2000,3000,4000

        if (ui->checkBoxFilter->isChecked())
        {
            for (int z = 0; z < 1000; z++)
            {
                dataFiltred[0][z] = data[k+z] * coefficient(setGain);
            }
            filter->process(1000, dataFiltred);
            min = dataFiltred[0][0];
            max = dataFiltred[0][0];

            for (int j = 0; j < 1000; j++)
            {
                //Для каждого канала вычисляем минимальное, максимальное и среднее значение
                if (min > dataFiltred[0][j]) min = dataFiltred[0][j];
                if (max < dataFiltred[0][j]) max = dataFiltred[0][j];
               if (i == 0) xD.append(j + num); // шкала Х для всех каналов одинаковая, поэтому заполняем один раз
                listData[i].append(dataFiltred[0][j]);
            }

        } else
        {
            min = data[0] * coefficient(setGain);
            max = data[0] * coefficient(setGain);
            for (int j = 0; j < 1000; j++)
            {
                //Для каждого канала вычисляем минимальное, максимальное и среднее значение
                if (min > (data[j+k] * coefficient(setGain))) min = data[j+k] * coefficient(setGain);
                if (max < (data[j+k] * coefficient(setGain))) max = data[j+k] * coefficient(setGain);
               if (i == 0) xD.append(j + num); // шкала Х для всех каналов одинаковая, поэтому заполняем один раз
                  listData[i].append(data[j+k] * coefficient(setGain));
            }
        }
        customPlot->graph(i)->setData(xD, listData[i]);
    }
    customPlot->yAxis->setRange(min, max);
       customPlot->replot();
       num += 1000;

    if (num > dimensionX)
    {
        customPlot->xAxis->setRange(num - dimensionX, num);
        for (int i = 0; i < listChannelsForCharts.size(); i++)
        {
            customPlot->graph(i)->data()->clear();
                listData[i].remove(0,1000);
                if (i == 0) xD.remove(0,1000); // Тоже только один раз удаляем, так как шкала Х общая
        }
    }
}



QList<QString>CPlot::getListChannels() //Возвращает список каналов, с которых нужно собирать данные
{
    QString result = "";
    QList<QString> listTmp;
    QList<QString> listResult;
    //Читаем данные с файла во временную таблицу
    QFile file("./channels.conf");
    if (file.open(QFile::ReadOnly))
    {
        QTextStream streamData(&file);
        while (!streamData.atEnd())
        {
            listTmp.append(streamData.readLine());
        }
        file.close();

        QString nameDev = "";
         for (int i = 0; i < listTmp.size(); i++)
        {
           QString line = listTmp[i];
           QStringList list = line.split("\t"); // Парсим строки
           if (list[3] == "true")
           {
               if(nameDev != list[1].left(list[1].indexOf("/")))
               {
                   listResult.append(result.left(result.length()-2));
                   result = list[1] + ", ";
                   nameDev = list[1].left(list[1].indexOf("/"));
               } else
               {
                   result = result + list[1] + ", ";
               }
          }
        }
        listResult.append(result.left(result.length()-2));
        listResult.removeFirst();
    }
    return listResult;
}

QList<int> CPlot::getListNumberChannelsForChart() //Возвращает список каналов, с которых нужно собирать данные
{
    quantityOfChannels = 0;
    int i = 0;
    QString oneString = "";
    QList<int> listResult;
    listNameOfChannelsForCharts.clear();
    //Читаем данные с файла во временную таблицу
    QFile file("./channels.conf");
    if (file.open(QFile::ReadOnly))
    {
        QTextStream streamData(&file);
        while (!streamData.atEnd())
        {
            oneString = streamData.readLine();
            QStringList list = oneString.split("\t"); // Парсим строки
            if (list[2] == "true") quantityOfChannels++;
            if (list[3] == "true")
            {
                listResult.append(i);
                listNameOfChannelsForCharts.append(list[1]);// список названия каналов для печати на графиках
            }
        }
        file.close();
    }
    return listResult;
}


void CPlot::on_pushButtonSave_clicked()
{
    QString str;

    if (path == NULL)
    {
        str = QFileDialog::getSaveFileName(0,"FileName","./DATA/","*.jpg");
    } else
    {
        str = QFileDialog::getSaveFileName(0,"FileName",path,"*.jpg");
    }
     QFile file(str);

     if (file.open(QIODevice::WriteOnly))
     {
         customPlot->saveJpg(str);
     }

}

void CPlot::printCheckBoxToTable(QTableWidget *w, int y, int x, bool check)
{
    QCheckBox *chk = new QCheckBox();
    chk->setChecked(check);
    w->setCellWidget(y,x,chk);
}


void CPlot::printStringToTable(QTableWidget *w, int y, int x, QString str)
{
    QTableWidgetItem *itm = new QTableWidgetItem();
    itm->setText(str);
    itm->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    w->setItem(y,x,itm);
}




void CPlot::on_pushButtonOpen_clicked()
{
    QFile fileConfig;
    QTableWidget *tabl = sChart->getTable();
    tabl->setColumnCount(3);
    tabl->setColumnWidth(0,150);
    tabl->setColumnWidth(1,70);
    tabl->setColumnWidth(2,70);
    tabl->setHorizontalHeaderLabels(QStringList() <<"Sensor"<<"Channel"<<"On/Off");
    QString str;
    if (path == NULL)
    {
        str = QFileDialog::getOpenFileName(0, "Open Dialog", "./DATA/", "*.qs1");
    } else
    {
        str = QFileDialog::getOpenFileName(0, "Open Dialog", path, "*.qs1");
    }

    if (str == NULL) return; // Если нажали "Отмена"
    fileForPost.setFileName(str);
    fileConfig.setFileName(str.left(str.size()-1)+"2"); // *.qs2
    if (fileConfig.open(QIODevice::ReadOnly))
    {
        QTextStream streamConfig(&fileConfig);
        while (!streamConfig.atEnd())
        {
            QString line = streamConfig.readLine();
            if (line.left(5) == "Gain:")
            {
                setGain = line.mid(6,20); // 20- чтоб гарантировано до конца строки
                ui->label_Gain->setText("Gain: " + setGain);
                break;
            }

        }
    }
    fileConfig.close();

    if (fileForPost.open(QIODevice::ReadOnly))
    {
        QTextStream streamData(&fileForPost);
        QString line = streamData.readLine();
        listChannelsSet = line.split("\t");
        for (int i = 1; i < listChannelsSet.size()-1; i++)
        {
            tabl->setRowCount(i);
            printStringToTable(tabl, i-1, 0, listChannelsSet[i]);
            printStringToTable(tabl, i-1, 1, getChannelFromSensor(listChannelsSet[i]));
            printCheckBoxToTable(tabl, i-1, 2, true);
        }

    }
    fileForPost.close();
    sChart->show();
}

void CPlot::setListNumberChannelsForChart(QList<int> list)
{
    listChannelsForCharts = list;
}

QString CPlot::getChannelFromSensor(QString channel)
{

    QString channelsToSensors[] = {"1Y","1Z","2Y","2Z","3Y","3Z","4Y","4Z","9Y","9Z","10Y","10Z",
                                   "11Y","11Z","12Y","12Z","5Y","5Z","6Y","6Z","7Y","7Z","8Y",
                                   "8Z","13Y","13Z","14Y","14Z","15Y","15Z","16Y","16Z"};
    bool res = false;
    int index = channel.right(2).toInt(&res,10); // Пробуем преобразовать в число два последних знака
    if (!res) index = channel.right(1).toInt(&res,10); // Если не получается, пробуем один крайний символ (каналы 0-9)
    if (!res) return "---"; // Если не удалось преобразовать, значит какая-то ошибка, не можем определить канал
    //Если все ок, то выводим значение
    return channelsToSensors[index];
}

void CPlot::on_pushButtonShowPost_clicked()
{
    int forColor = 0;
    QColor color[] = {QColor(127,0,0),QColor(255,0,0),QColor(0,127,0),QColor(127,127,0),QColor(255,127,0),QColor(0,255,0),
                      QColor(127,255,0),QColor(255,255,0),QColor(0,0,127),QColor(0,0,255),QColor(127,0,255),QColor(255,0,255),
                      QColor(255,127,255),QColor(0,0,0)};
    QList<int> list = sChart->listChannels;
    listData.clear(); // хранятся серии для графиков по оси Y
    xD.clear(); // хранятся серии для графиков по оси X
    customPlot->clearGraphs();
    double min = 100000000;
    double max = -100000000;
    ui->progressBar->setValue(0);
    QList<QString> listString;


    for (int i = 0; i < list.size(); i++)
    {

     //Создадим Plots по количеству каналов
        QVector<double> *v = new QVector<double>;
        listData.append(*v);
        customPlot->addGraph();
        customPlot->graph(i)->setName(QString::number(i+1) + ". " + listChannelsSet[i+1]);
        QPen pen;
        pen.setWidth(2);
        pen.setColor(color[forColor]);
        customPlot->graph(i)->setPen(pen);
        forColor++;
        if (forColor > 14) forColor = 0;

    }
    bool first = true;
    int z = 0;
    listString.clear();
    if (fileForPost.open(QIODevice::ReadOnly))
    {
        QTextStream streamData(&fileForPost);
        while (!streamData.atEnd())
        {
            QString line = streamData.readLine();
            if (first) // пропускаем шапку
             {
                 first = false;
                 continue;
             }
            listString.append(line);
         }
        for (int j = 0; j < listString.size(); j++)
        {
            QStringList oneString = listString[j].split("\t");
            for (int i = 0; i < list.size(); i++)
            {
                 //Получаем данные по каналам и добавляем в серию list[i] - номер нужного канала
                double zn = oneString[(list[i]+1)*2].toDouble() * coefficient(setGain); // в файле данные разделил двумя табуляциями
                listData[i].append(zn);
                if (min > zn) min = zn;
                if (max < zn) max = zn;
            }
            xD.append(z);
            z++;
            ui->progressBar->setValue(z*100/(listString.size()+1));
        }
    }
    for (int i = 0; i < list.size(); i++)
    {
        customPlot->graph(i)->setData(xD, listData[i]);
    }
    ui->progressBar->setValue(100);
    customPlot->yAxis->setRange(min, max);
    ui->spinBoxYmin->setValue(min);
    ui->spinBoxYmax->setValue(max);
    customPlot->xAxis->setRange(0, z);
    ui->spinBoxTo->setValue(z);
    ui->spinBoxFrom->setValue(0);
    customPlot->replot();
    fileForPost.close();
}

void CPlot::on_pushButtonRefreshPost_clicked()
{
    if (ui->spinBoxFrom->value() >= ui->spinBoxTo->value()) {
        QMessageBox::warning(this,"Warning","Value 'To' <= 'From' ",1,1);
        return;
    }
    customPlot->xAxis->setRange(ui->spinBoxFrom->value(), ui->spinBoxTo->value());
    customPlot->yAxis->setRange((double) ui->spinBoxYmin->value(), (double) ui->spinBoxYmax->value());
    customPlot->replot();
}
