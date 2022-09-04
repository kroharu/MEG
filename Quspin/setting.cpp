#include "setting.h"
#include "ui_setting.h"
#include <QSettings>
#include <QCheckBox>
#include <QDebug>
#include <QFile>
#include <QColorDialog>
#include <QTextCodec>
#include "NIDAQmx.h"
#include "cplot.h"


Setting::Setting(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Setting)
{
    ui->setupUi(this);
    QSettings *settings = new QSettings("./quspin.conf",QSettings::IniFormat );
    ui->labelQuantityChannels->setStyleSheet("QLabel {color : blue; }");
    ui->tableWidget->setColumnCount(5);
    int width = ui->tableWidget->width();
    ui->tableWidget->setColumnWidth(0,width*2/18);
    ui->tableWidget->setColumnWidth(1,width*6/18);
    ui->tableWidget->setColumnWidth(2,width*3/18);
    ui->tableWidget->setColumnWidth(3,width*2/18);
    ui->tableWidget->setColumnWidth(4,width*2/18);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() <<"N"<<"Sensor"<<"Channel"<<"On/Off"<<"Charts");
    ui->spinBoxDimensionX->setValue(settings->value("charts/dimensionX").toInt());
    ui->spinBoxDimensionY->setValue(settings->value("charts/dimensionY").toInt());
    ui->spinBoxInterval->setValue(settings->value("charts/interval").toInt());
    ui->labelFullPath->setText(settings->value("path/absolutPath").toString());

    int i = 0;
    QFile file("./channels.conf");
    if(QFileInfo::exists("./channels.conf"))
    {
        if (file.open(QFile::ReadOnly))
        {
            QTextStream streamData(&file);
            while (!streamData.atEnd())
            {
               QString line = streamData.readLine();
               QStringList list = line.split("\t"); // Парсим строки
               ui->tableWidget->setRowCount(i+1);
               ui->tableWidget->setRowHeight(i,10);

               printStringToTable(i,0,list[0]);
               printStringToTable(i,1,list[1]);
               printStringToTable(i,2,getChannelFromSensor(list[1]));

               if (list[2] == "true") printCheckBoxToTable(i,3,true);
               else printCheckBoxToTable(i,3,false);
               if (list[3] == "true") printCheckBoxToTable(i,4,true);
               else printCheckBoxToTable(i,4,false);

               i++;
            }
            file.close();
        }
    } else
    {
        file.open(QIODevice::ReadWrite | QIODevice::Text);
        file.write("");
        file.close();
    }
    checkQuantityCharts();
}

Setting::~Setting()
{
    delete ui;
}

void Setting::on_pushButtonCancel_clicked()
{
    close();
}

void Setting::on_pushButtonOK_clicked()
{
    QSettings *settings = new QSettings("./quspin.conf",QSettings::IniFormat );
    settings->setValue("charts/dimensionX", ui->spinBoxDimensionX->value());
    settings->setValue("charts/dimensionY", ui->spinBoxDimensionY->value());
    settings->setValue("charts/interval", ui->spinBoxInterval->value());
    settings->setValue("path/absolutPath", ui->labelFullPath->text());
    settings->sync();

    //Save list of channels in file
    QFile file("./channels.conf");

    file.open(QIODevice::WriteOnly);
    QTextStream streamData(&file);// Передали путь к файлу переменной stream
    for (int i = 0; i < ui->tableWidget->rowCount(); i++)
    {
        QString str = ui->tableWidget->item(i,0)->text() + "\t" + ui->tableWidget->item(i,1)->text() + "\t"; //QString::number(i+1)
        QWidget * cellWidget = ui->tableWidget->cellWidget(i, 3);
        QCheckBox * checkbox = dynamic_cast<QCheckBox*>(cellWidget);
        if (checkbox->isChecked()) str += "true";
        else str += "false";
        str +=  "\t";
        cellWidget = ui->tableWidget->cellWidget(i, 4);
        checkbox = dynamic_cast<QCheckBox*>(cellWidget);
        if (checkbox->isChecked()) str += "true";
        else str += "false";
        str +=  "\n";
        streamData << str;
   }
    file.close();
    close();
}


void Setting::printCheckBoxToTable(int y, int x, bool check)
{
    QCheckBox *chk = new QCheckBox();
    chk->setChecked(check);
    ui->tableWidget->setCellWidget(y,x,chk);
    if (x == 3) listCh1.insert(y,chk);
    if (x == 4) listCh2.insert(y,chk);
}

void Setting::changeCheckBoxToTable(int y, int x, bool check)
{
    QWidget * cellWidget = ui->tableWidget->cellWidget(y, x);
    QCheckBox * checkbox = dynamic_cast<QCheckBox*>(cellWidget);
    checkbox->setChecked(check);
}


void Setting::printStringToTable(int y, int x, QString str)
{
    QTableWidgetItem *itm = new QTableWidgetItem();
    itm->setText(str);
    if (x==0) itm->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    else itm->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->tableWidget->setItem(y,x,itm);
}

void Setting::on_Button_addString_clicked()
{
    int i = ui->tableWidget->rowCount();
    i++;
    ui->tableWidget->setRowCount(i);
    ui->tableWidget->setRowHeight(i-1,10);
    printCheckBoxToTable(i-1,3,true);
    printCheckBoxToTable(i-1,4,false);
    printStringToTable(i-1,0,QString::number(i));
}

void Setting::on_Button_deleteString_clicked()
{
    int currentIndex = ui->tableWidget->currentIndex().row();
    if (currentIndex > -1) ui->tableWidget->removeRow(currentIndex);
}

void Setting::on_pushButtonCheck_clicked()
{
    for (int i = 0; i < ui->tableWidget->rowCount(); i++)
    {
        changeCheckBoxToTable(i, 3, true);
    }
    if (setAxis != "DUAL") QMessageBox::warning(this,"Warning","Sensitive Axis is " + setAxis,1,1);
    checkQuantityCharts();
}

void Setting::on_pushButtonUncheck_clicked()
{
    for (int i = 0; i < ui->tableWidget->rowCount(); i++)
    {
        changeCheckBoxToTable(i, 3, false);
    }
    checkQuantityCharts();
}

void Setting::on_tableWidget_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    checkQuantityCharts();
   // qDebug() <<"Here!";
}

void Setting::checkQuantityCharts() //Было: считала количество графиков, которые выводить. Стало: добавил еще кол-во каналов, которые мерим
{
    int quantity = 0;
    int quantityChannels = 0;
    for (int i = 0; i < ui->tableWidget->rowCount(); i++)
    {
        QWidget * cellWidget = ui->tableWidget->cellWidget(i, 4);
        if (cellWidget == NULL) continue;
        QCheckBox * checkbox = dynamic_cast<QCheckBox*>(cellWidget);
        //Если мы выбираем канал для вывода на график, но сразу отмечаем что нужно мерить его
        QWidget * cellWidgetChannel = ui->tableWidget->cellWidget(i, 3);
        QCheckBox * checkboxChannel = dynamic_cast<QCheckBox*>(cellWidgetChannel);
        if (checkbox->isChecked())
        {
            checkboxChannel->setChecked(true);
            quantity++ ;
        }

        //Считаем количество каналов, которые мы будем измерять
        if (checkboxChannel->isChecked())
        {
            quantityChannels++;
        }

    }
    if (quantity >20)
    {
        ui->labelQuantityCharts->setStyleSheet("QLabel {color : red; }");
        ui->labelQuantityCharts->setText("Quantity over 20");
        ui->pushButtonOK->setEnabled(false);
    } else
    {
        ui->labelQuantityCharts->setStyleSheet("QLabel {color : blue; }");
        ui->labelQuantityCharts->setText(QString::number(quantity) + " charts will show");
        ui->pushButtonOK->setEnabled(true);
    }

    ui->labelQuantityChannels->setText("Channels are " + QString::number(quantityChannels));
}

void Setting::setSensAxis(QString myAxis)
{
    setAxis = myAxis;
}

void Setting::on_pushButtonZ_clicked()
{
    for (int i = 0; i < ui->tableWidget->rowCount(); i = i+2)
    {
        changeCheckBoxToTable(i, 3, false);
        changeCheckBoxToTable(i+1, 3, true);
    }
    if (setAxis != "Z") QMessageBox::warning(this,"Warning","Sensitive Axis is " + setAxis,1,1);
    checkQuantityCharts();
}

void Setting::on_pushButtonY_clicked()
{
    for (int i = 0; i < ui->tableWidget->rowCount(); i = i+2)
    {
        changeCheckBoxToTable(i, 3, true);
        changeCheckBoxToTable(i+1, 3, false);
    }
    if (setAxis != "Y") QMessageBox::warning(this,"Warning","Sensitive Axis is " + setAxis,1,1);
    checkQuantityCharts();
}

void Setting::on_pushButtonLoadChannels_clicked()
{
    int z = 0;
    ui->tableWidget->clear();
    char dataSet[1000];
    char channelsSet[1000];
    uInt32 bufferSizeSet = 100;
    DAQmxGetSysDevNames(dataSet, bufferSizeSet);
    QString DataStringSet = QTextCodec::codecForMib(106)->toUnicode(dataSet);
    QStringList listSet = DataStringSet.split(", ");
    for (int i = 0; i< listSet.size(); i++)
    {
        DAQmxGetDevAIPhysicalChans(listSet[i].toLocal8Bit(), channelsSet, 1000);
        QString DataChannels = QTextCodec::codecForMib(106)->toUnicode(channelsSet);
        QStringList listChannelsSet = DataChannels.split(", ");
        if (listChannelsSet.size() == 1) continue;
        for (int j = 0; j < listChannelsSet.size(); j++)
        {
            ui->tableWidget->setRowCount(z+1);
            ui->tableWidget->setRowHeight(z,10);
            printStringToTable(z,0,QString::number(z+1));
            printStringToTable(z,1,listChannelsSet[j]);
            printStringToTable(z,2,getChannelFromSensor(listChannelsSet[j]));
            printCheckBoxToTable(z,3,true);
            printCheckBoxToTable(z,4,false);
            z++;
        }

    }
    checkQuantityCharts();
}

QString Setting::getChannelFromSensor(QString channel)
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

void Setting::on_pushButton_clicked()
{
    QFileDialog dialog(this);

    dialog.setFileMode(QFileDialog::Directory);

    QString _OutputFolder = QFileDialog::getExistingDirectory(0, ("Select Output Folder"), QDir::currentPath());
    ui->labelFullPath->setText(_OutputFolder);
}
