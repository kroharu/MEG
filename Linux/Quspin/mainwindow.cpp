#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "workwithfiles.h"
#include "setting.h"
#include "ftd2xx.h"
#include "NIDAQmx.h"
#include <QDataStream>
#include <cplot.h>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    for (int i=0; i<100;i++)
    {
        statusLed[i] = true;
    }

    this->setWindowTitle("QUSPIN v1.005");
    ui->labelProgress->setText("00:00");
    QRegExp expP("[1-9]{1}[0-9]{0,3}");
    ui->lineEditPeriod->setValidator(new QRegExpValidator(expP));//Валидация полей
    QRegExp expD("[1-9]{1}[0-9]{0,3}");
    ui->lineEditDuration->setValidator(new QRegExpValidator(expD));//Валидация полей
    QRegExp expE("[a-zA-Z_0-9А-Яа-я]{0,}");
    QRegExpValidator *val = new QRegExpValidator(expE);
    ui->lineEditExperiment->setValidator(val);
    ui->lineEditObject->setValidator(val);
    myData = new QList<QString>; // данные с датчиков в строковом формате для записи в файл
    myConf = new QList<QString>; // данные о параметрах датчиков для записи в добавочный файл
    myLog = new QList<QString>; // данные о ошибках
    quantitySteps = 0;
    setForBegin();
    ui->labelDate->setText(QDate::currentDate().toString("dd.MM.yyyy"));
    timer = new QTimer(this);
    timer->setTimerType(Qt::CoarseTimer);
    timer->setInterval(1000);

    // Обработка нажатия кнопок в меню с фиксированными командами
    connect(ui->pushButtonAutoStart,&QPushButton::clicked,this,&MainWindow::sendFixCommand);
    connect(ui->pushButtonFieldZero,&QPushButton::clicked,this,&MainWindow::sendFixCommand);
    connect(ui->pushButtonCalibrate,&QPushButton::clicked,this,&MainWindow::sendFixCommand);
    connect(ui->pushButtonZ,&QPushButton::clicked,this,&MainWindow::sendFixCommand);
    connect(ui->pushButtonY,&QPushButton::clicked,this,&MainWindow::sendFixCommand);
    connect(ui->pushButtonDual,&QPushButton::clicked,this,&MainWindow::sendFixCommand);
    connect(ui->pushButton033X,&QPushButton::clicked,this,&MainWindow::sendFixCommand);
    connect(ui->pushButton1X,&QPushButton::clicked,this,&MainWindow::sendFixCommand);
    connect(ui->pushButton3X,&QPushButton::clicked,this,&MainWindow::sendFixCommand);
    connect(ui->pushButtonReboot,&QPushButton::clicked,this,&MainWindow::sendFixCommand);
    //***********************************
    connect(timer, SIGNAL(timeout()), this, SLOT(ActionTimer()));
    //Коннекты для организации отдельного потока для измерений

    connect(&thread, &QThread::started, &measur,&Measurement::run);
    connect(&measur, &Measurement::finished, &thread, &QThread::terminate);
    measur.moveToThread(&thread);
    loadLogbook();
    loadSetting();
    on_checkBoxInfinite_clicked(); // Делаем доступным поле "Длительность эксперимента"
    cpl = new CPlot();
    set = new Setting(this);
    if(!QFileInfo::exists("./channels.conf")) {
        QMessageBox::warning(this,"Can't find file channels.conf","Please  set channels for show charts",1,1);
    }
    ui->pushButtonStartStop->setStyleSheet("background-color: gray");
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::closeEvent(QCloseEvent *event){
   // Закрывает вторую форму с графиками при закрытии основной
    //можно было сделать по другому: grf->setParent(this, Qt::Window);
    //Но тогда вторая форма перекрывает первую и не дает первую вывести над второй
    delete cpl;
}



void MainWindow::setForBegin(){
    ui->tableWidget->setColumnCount(11);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() <<"Sensor"<<"M/S"<<"Laser On/Off"<<"Cell T Lock"<<"Laser Lock"<<"Field Zero"<<"B0 (pT)"<<"By (pT)"<<"Bz (pT)"<<"Cell T Error"<<"Sensor Status");

}

void MainWindow::resizeTablWidget(){
    int width = ui->tableWidget->width();
    ui->tableWidget->setColumnWidth(0,width*12/72);
    ui->tableWidget->setColumnWidth(1,width*1/72);
    ui->tableWidget->setColumnWidth(2,width*4/72);
    ui->tableWidget->setColumnWidth(3,width*4/72);
    ui->tableWidget->setColumnWidth(4,width*4/72);
    ui->tableWidget->setColumnWidth(5,width*4/72);
    ui->tableWidget->setColumnWidth(6,width*5/72);
    ui->tableWidget->setColumnWidth(7,width*5/72);
    ui->tableWidget->setColumnWidth(8,width*5/72);
    ui->tableWidget->setColumnWidth(9,width*6/72);
    ui->tableWidget->setColumnWidth(10,width*20/72);
}


QList<MainWindow::QuspinInf> MainWindow::scanComPorts(){
    QList<QuspinInf> list;
    QString serNum;
    QString portName;
    QuspinInf portInf;
    QList<QuspinDev> devAd = getAddInformComPorts();

    //поиск портов
    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
        {
            serNum = serialPortInfo.serialNumber().left(8); //Почему-то метод serialNumber() выводит на один знак больше (9),
                                                            //чем нативный драйвер FTD2XX, поэтому берем 8 символов
            portInf.Name = serialPortInfo.portName();
            portInf.ProductDescription = "";
            foreach (QuspinDev it, devAd)
            {
                if (it.SerialNumber == serNum) {
                    portInf.ProductDescription =  it.ProductDescription;
                    break;
                }
            }
            list.append(portInf);
        }
    //Сортировать по возрастанию названия сенсоров QUSPIN
    if (list.size() > 1)
    {
        bool endSort = true;
        while (endSort)
        {
            endSort = false;
            for (int i = 1; i < list.size(); i++)
            {
                if (QString::compare(list[i].ProductDescription,list[i-1].ProductDescription) < 0)
                {
                    QuspinInf str = list[i];
                    list[i] = list[i-1];
                    list[i-1] = str;
                    endSort = true;
                }
            }
        }
    }
   //***************************
    return list;
}

void MainWindow::clearTable(int i){
    printToTable(i, 1, "", Qt::white);
    printToTable(i, 2, "1", Qt::yellow);
    printToTable(i, 3, "2", Qt::yellow);
    printToTable(i, 4, "3", Qt::yellow);
    printToTable(i, 5, "4", Qt::yellow);
    printToTable(i, 6, "0", Qt::white);
    printToTable(i, 7, "0", Qt::white);
    printToTable(i, 8, "0", Qt::white);
    printToTable(i, 9, "0", Qt::white);
    printToTable(i, 10, "", Qt::white);
}

void MainWindow::on_pushButtonRescan_clicked()
{
    resizeTablWidget();


    // For TEST
    /*
    ui->tableWidget->setRowCount(27);
    for (int i = 0; i< 27; i++)
    {
        ui->tableWidget->setRowHeight(i,28); // Высота ячеек
        printStringToTable(i,0,"COMPORT" + QString::number(i));
    }
*/

    listPorts = scanComPorts();
    quantityComPorts = listPorts.size();
    ui->comboBoxSelectComPort->clear();
    ui->tableWidget->setRowCount(quantityComPorts);
    for (int i = 0; i < quantityComPorts; i++)
    {
        ui->comboBoxSelectComPort->addItem(listPorts[i].ProductDescription);
        printStringToTable(i,0,listPorts[i].ProductDescription);
        ui->tableWidget->setRowHeight(i,28); // Высота ячеек
        QSerialPort *sPort = new QSerialPort(this);
        serialPort[i] = sPort;
        clearTable(i);
        sPort->setPortName(listPorts[i].Name);
        sPort->setBaudRate(QSerialPort::Baud115200,QSerialPort::AllDirections);
        sPort->setDataBits(QSerialPort::Data8);
        sPort->setStopBits(QSerialPort::OneStop);
        sPort->setFlowControl(QSerialPort::NoFlowControl);
        sPort->setParity(QSerialPort::NoParity);

        if (!sPort->open(QIODevice::ReadWrite)) {
            QMessageBox::warning(this,"Can't coonect to " + listPorts[i].Name,"Please check connection ",1,1);
        }
         connect(sPort, &QSerialPort::readyRead, this, &MainWindow::readDataComPort);
    }



}



void MainWindow::printToTable(int y, int x, QString str, QColor color){
    QTableWidgetItem *itm = new QTableWidgetItem();
    itm->setText(str);
    itm->setBackgroundColor(color);
    if ((x>0)&&(x<=5)) itm->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    else itm->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->tableWidget->setItem(y,x,itm);

}
void MainWindow::printStringToTable(int y, int x, QString str){
    QTableWidgetItem *itm = new QTableWidgetItem();
    itm->setText(str);
    ui->tableWidget->setItem(y,x,itm);
}

void MainWindow::checkStatusSensor(int i, bool status){
    if (ui->pushButtonStartStop->text() == "Stop") {
        if (statusLed[i] != status) {
            statusLed[i] = status;
            int time = QTime::currentTime().msecsSinceStartOfDay() - msFromStart;
            QString st = status ? "ON" : "OFF";
            QString str = listPorts[i].ProductDescription + "\t" + QString::number(time) + "\t" + st + "\n";
            myLog->append(str);
        }
    }
}

void MainWindow::readDataComPort(){
    //Здесь регистрируется вся полученная инфа из Com-Port!!!!
    for (int i = 0;i < quantityComPorts; i++) {
        if (serialPort[i] == QObject::sender()){

            QByteArray rawData = serialPort[i]->readAll();
            QString DataAsString = QTextCodec::codecForMib(106)->toUnicode(rawData);
            if (forGetStringFromComPort[i] != "") // Если пришла строка, а у нас предыдущая была не полностью принята, то дополняем
            {                                     // предыдущей строкой вперед и парсим потом
                DataAsString = forGetStringFromComPort[i] + DataAsString;
                forGetStringFromComPort[i] = "";
            }
            if (DataAsString.right(2) != "\r\n") // Если строка пришла не полностью в порт, то запоминаем но не парсим, а делаем return
            {
                forGetStringFromComPort[i] = DataAsString;
                return;
            }
            QStringList list = DataAsString.split("\r\n"); // Парсим строки
            for (int ind=0; ind < list.size(); ind++)
               {
                if (list[ind].left(1) == "#") { // Sensor status
                    printStringToTable(i, 10, list[ind]);
                    if (list[ind].left(6) == "#Calib")
                    {
                        forGetValueOfCalibrate[i] = list[ind];
                    }
                }
                else if (list[ind].left(1) == "|") { // Indicators
                      QString num = list[ind].mid(1,2);
                    if (num == "10") {
                        printToTable(i, 2, "1", Qt::yellow);
                    } else if (num == "11"){
                        printToTable(i, 2, "1", Qt::green);
                    } else if (num == "20"){
                        //Если ошибка
                        checkStatusSensor(i, false);
                        printToTable(i, 3, "2", Qt::yellow);
                    } else if (num == "21"){
                        //Если все в порядке
                        checkStatusSensor(i, true);
                        printToTable(i, 3, "2", Qt::green);
                    } else if (num == "30"){
                        printToTable(i, 4, "3", Qt::yellow);
                    } else if (num == "31"){
                        printToTable(i, 4, "3", Qt::green);
                    } else if (num == "40"){
                        printToTable(i, 5, "4", Qt::yellow);
                    } else if (num == "41"){
                        printToTable(i, 5, "4", Qt::green);
                    } else if (num == "51"){
                        printToTable(i, 1, "S", Qt::white);
                    } else if (num == "50"){
                        printToTable(i, 1, "M", Qt::red);
                    }

                }
                else if (list[ind].left(1) == "~") { //Sensor parameter readout
                    QString header = list[ind].mid(1,2);
                    if (header == "07")
                    {
                        printStringToTable(i, 8, QString::number(list[ind].mid(3,rawData.indexOf("\r")-2).toFloat() - 32768));
                    } else if (header == "08")
                    {
                        printStringToTable(i, 7, QString::number(list[ind].mid(3,rawData.indexOf("\r")-2).toFloat() - 32768));
                    } else if (header == "09")
                    {
                        printStringToTable(i, 6, QString::number(list[ind].mid(3,rawData.indexOf("\r")-2).toFloat() - 32768));
                    } else if (header == "04")
                    {
                        float cellError = (list[ind].mid(3,rawData.indexOf("\r")-2).toFloat() - 8388608)/524288;
                        if (cellError >1.5) printToTable(i, 9, QString::number(cellError), Qt::red);
                        else printToTable(i, 9, QString::number(cellError), Qt::white);
                    }
                }
            }

        }
    }
}


void MainWindow::on_pushButtonCPlot_clicked()
{
    cpl->show();
    cpl->getSetGain(setGain);
}

void MainWindow::on_pushButtonStartStop_clicked()
{
    loadSetting();
    QFile rawFile;
    QString path = ui->lineEditExperiment->text()+"/"+ui->labelDate->text()+"/"+ui->lineEditObject->text();
    if (absolutPath == NULL) {
        QMessageBox::warning(this,"No folder fo data","Please set folder for save data ",1,1);
        return;
    }

    cpl->setPathForSaveGpg(absolutPath + "/" + path + "/");
//    cpl->setPathForSaveGpg("./DATA/"+path+"/");
    if (ui->pushButtonStartStop->text() == "Start")
    {
        if (!addDescription()){// Проверяем, заполнена ли таблица с датчиками. Делали подключение их или нет?
            QMessageBox::warning(this,"No sensors","Please push button Rescan ",1,1);
            return;
        }
        //Проверяем, заполнены ли поля имя эксперимента и испытуемого
        if ((ui->lineEditExperiment->text().isEmpty())||(ui->lineEditObject->text().isEmpty())){
            QMessageBox::warning(this,"Name of Experiment or Object is empty...","Please fill in the fields on the form ",1,1);
            ui->lineEditExperiment->setFocus();
            return;
        }

        ui->pushButtonStartStop->setStyleSheet("background-color: red");

        saveSetting(); //Сохраняем поля

        ui->progressBar->setValue(0); //Обнуляем прогресс бар
        duration = ui->lineEditDuration->text().toInt(); // Получаем длительность эксперимента, если он не infinite
        QList<QString> listChannels = getListChannels(); // Получаем список каналов, которые будем опрашивать
        quantityChannels = getQuantityChannels(listChannels); // Получаем количество каналов

        //Создаем файл для "сырых данных"
        rawFile.setFileName(QApplication::applicationDirPath() + "/data.raw");
        if(!rawFile.open(QFile::WriteOnly))
        {
             QMessageBox::warning(this,"Can't open file","Please check permissions",1,1);
             return;
        }
        rawFile.close();

        ui->pushButtonStartStop->setText("Stop");
        timer->start(); // Запускаем таймер, раз в секунду
        quantitySteps = 0; // Общее количество секунд, прошедших с начала

        QDate dateToday = QDate::currentDate();
        QTime timeNow = QTime::currentTime();
        QString recordLine = "Date: " + dateToday.toString("dd.MM.yy") + "  Time: " + timeNow.toString("hh:mm:ss") + "\n";

        //Записываем шапки в файлы qs1, qs2
        recordLine += "time, ms\t"; // шапка в файл с данными  listChannelsForFile
        for (int z = 0; z < listChannelsForFile.size(); z++)
        {
            recordLine += listChannelsForFile[z] + "(" + set->getChannelFromSensor(listChannelsForFile[z]) + ")\t";
        }
         recordLine += "\n";
         myData->append(recordLine);

         recordLine = "\n" + recordLine;
         recordLine += "Quantity channels: " + QString::number(quantityChannels) + "\n";
         myConf->append(recordLine);
         //При запуске эксперимента у нас потенциально все датчики в порядке, потому заполняем массив значениями "true"
         for (int i=0; i<100;i++)
         {
             statusLed[i] = true;
         }
         //*******************************************
         measur.setWork(true); // Устанавливаем переменную, разрешающую делать эксперименты
         measur.setChannels(listChannels); // Передаем список каналов, по которым пойдут измерения
         QTime timeN = QTime::currentTime();
         msFromStart = timeN.msecsSinceStartOfDay();
         thread.start(); // Запускаем измерение

    } else
    {
        ui->pushButtonStartStop->setStyleSheet("background-color: gray");
        totalOperacion = measur.getQuantityOperaciones();
        measur.setWork(false);
        ui->pushButtonStartStop->setText("Start");
        timer->stop();
        myData = getTextFromRaw(myData,QApplication::applicationDirPath() + "/data.raw", quantityChannels, totalOperacion, 1);

        if (ui->checkBoxSaveFile->isChecked()){
             WorkWithFiles *wr = new WorkWithFiles();
//             wr->writeData(path,*myData,*myConf,*myLog); // Записываем данные в файл
             wr->writeData(absolutPath + "/" + path,*myData,*myConf,*myLog); // Записываем данные в файл
          //   qDebug() << absolutPath + "/" + path;
             QString nameFileData = wr->fileDataName;
             wr->~WorkWithFiles();
         }

         myData->clear();
         myConf->clear();
         myLog->clear(); // Очищаем массив логов для нового эксперимента..
    }
}

void MainWindow::ActionTimer()
{
    QString m,s;
    int min = quantitySteps / 60;
    int sec = quantitySteps % 60;
    if (min < 10) m = "0"; else m = "";
    if (sec < 10) s = "0"; else s = "";
    ui->labelProgress->setText(m + QString::number(min)+ ":" + s + QString::number(sec));
    if (!ui->checkBoxInfinite->isChecked())
    {
        ui->progressBar->setValue(quantitySteps*100/duration);
        if (quantitySteps > duration) on_pushButtonStartStop_clicked();
    }
    quantitySteps++;
}

bool MainWindow::addDescription(){
    int row = ui->tableWidget->rowCount();
    if (row == 0) return false;
    QString str="Sensor\t\tM/S\tCell T Error\t\tSensor Status\n";
    myConf->append(str);

    for (int i = 0;i < row; i++){
        str = "";
        str = str + ui->tableWidget->item(i,0)->text() + "\t";
        str = str + ui->tableWidget->item(i,1)->text() + "\t";
        str = str + ui->tableWidget->item(i,9)->text() + "\t\t";
        QString valueCal = forGetValueOfCalibrate[i];
        str = str + valueCal + "\t";
        int begin = valueCal.indexOf(":")+1;
        int end = valueCal.indexOf("(z)");
        str = str + valueCal.mid(begin,end-begin) + "\t";
        str = str + "\n";
        myConf->append(str);
    }

    str = "===================================================\n";
    str += "Axis: " + setAxis + "\n";
    str += "Gain: " + setGain + "\n";
    if (ui->checkBoxInfinite->isChecked())
    {
        str += "Duration Experiment: Infinite \n";
    } else
    {
        str += "Duration Experiment: " + ui->lineEditDuration->text()+ " s.\n";
    }
    str += "Sampling rate: " + ui->lineEditPeriod->text()+ " Hz\n";
//    str += "Interval measurement: " + QString::number(intervalMeasurement)+ " s.\n";
    myConf->append(str);
    return true;
}

void MainWindow::resizeEvent(QResizeEvent* event)//При изменении размера окна
{
   resizeTablWidget();
   QMainWindow::resizeEvent(event);
}

void MainWindow::on_pushButtonLoad_clicked()
{
    if (ui->comboBoxSelectComPort->currentIndex() == -1) return;
    char cc[1] = {(char) ui->lineEditLoad->text().left(3).toInt()};
    serialPort[ui->comboBoxSelectComPort->currentIndex()]->write(cc);

    ui->lineEditLoad->setText("");
}

void MainWindow::on_pushButtonLoadAll_clicked()
{
    if (ui->comboBoxSelectComPort->currentIndex() == -1) return;
    for (int i = 0;i < quantityComPorts; i++) {
        char cc[1] = {(char) ui->lineEditLoadAll->text().left(3).toInt()};
        serialPort[i]->write(cc);
    }
    ui->lineEditLoadAll->setText("");
}

void MainWindow::sendFixCommand(){
    if (ui->comboBoxSelectComPort->currentIndex() == -1) return; // проверка на наличие доступных портов
    QString codCommand;
    if (QObject::sender() == ui->pushButtonAutoStart) codCommand = AUTO_START;
    if (QObject::sender() == ui->pushButtonFieldZero) {
        if (ui->pushButtonFieldZero->isChecked()){
            codCommand = FIELD_ZERO_ON;
        } else {
            codCommand = FIELD_ZERO_OFF;
        }
    }
    if (QObject::sender() == ui->pushButtonCalibrate) codCommand = CALIBRATE;
    if (QObject::sender() == ui->pushButtonZ)
    {
        codCommand = Z_AXIS_MODE;
        setAxis = "Z";
    }
    if (QObject::sender() == ui->pushButtonY)
    {
        codCommand = Y_AXIS_MODE;
        setAxis = "Y";
    }
    if (QObject::sender() == ui->pushButtonDual)
    {
        codCommand = DUAL_AXIS;
        setAxis = "DUAL";
    }
    if (QObject::sender() == ui->pushButton033X)
    {
        codCommand =MODE_033X ;
        setGain = "0.33X";
    }
    if (QObject::sender() == ui->pushButton1X)
    {
        codCommand = MODE_1X;
        setGain = "1X";
    }
    if (QObject::sender() == ui->pushButton3X)
    {
        codCommand = MODE_3X;
        setGain = "3X";
    }
    if (QObject::sender() == ui->pushButtonReboot) codCommand = REBOOT;

    for (int i = 0;i < quantityComPorts; i++) {
        serialPort[i]->write(codCommand.toLocal8Bit());
    }
    if (codCommand == REBOOT){
        for (int i = 0;i < quantityComPorts; i++) {
            clearTable(i);
        }
    }
}

void MainWindow::startStopCommand(QString command){
    for (int i = 0;i < quantityComPorts; i++) {
        serialPort[i]->write(command.toLocal8Bit());
    }
}

void MainWindow::saveSetting(){
    QSettings *settings = new QSettings("./quspin.conf",QSettings::IniFormat );
    settings->setValue("last_setting/nameOfExperiment", ui->lineEditExperiment->text());
    settings->setValue("last_setting/nameOfObject", ui->lineEditObject->text());
    settings->setValue("last_setting/duration", ui->lineEditDuration->text());
    settings->setValue("last_setting/period", ui->lineEditPeriod->text());
    settings->setValue("last_setting/infinite", ui->checkBoxInfinite->isChecked());
    settings->setValue("last_setting/saveFile", ui->checkBoxSaveFile->isChecked());
    settings->sync();
}
void MainWindow::loadSetting(){
    QSettings *settings = new QSettings("./quspin.conf",QSettings::IniFormat );
    ui->lineEditExperiment->setText(settings->value("last_setting/nameOfExperiment").toString());
    ui->lineEditObject->setText(settings->value("last_setting/nameOfObject").toString());
    ui->lineEditDuration->setText(settings->value("last_setting/duration").toString());
    ui->lineEditPeriod->setText(settings->value("last_setting/period").toString());
    ui->checkBoxInfinite->setChecked(settings->value("last_setting/infinite").toBool());
    ui->checkBoxSaveFile->setChecked(settings->value("last_setting/saveFile").toBool());
    absolutPath = settings->value("path/absolutPath").toString();
}


void MainWindow::on_actionSetting_triggered()
{
    set->setSensAxis(setAxis);
    set->show();
}

void MainWindow::on_pushButtonClose_clicked()
{
    close();
}


void MainWindow::on_tableWidget_cellDoubleClicked(int row, int column)
{
    // Возможно пригодится для каких-то дополнительных данных по сенсорам, удобно вызывать двойным кликом
 //   QModelIndex currentIndex = ui->tableWidget->currentIndex();
}

QList<QString> MainWindow::getListChannels() //Возвращает список каналов, с которых нужно собирать данные
{
    listChannelsForFile.clear();
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
           if (list[2] == "true")
           {
               listChannelsForFile.append(list[1]);

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

int MainWindow::getQuantityChannels(QList<QString> listChan)
{
    int qCh = 0;
    for (int i = 0; i < listChan.size(); i++)
    {
        QList<QString> list = listChan[i].split(", ");
         qCh += list.size();
    }
    return qCh;
}

QList<MainWindow::QuspinDev> MainWindow::getAddInformComPorts()
{
    QList<QuspinDev> result;

    FT_STATUS ftStatus;
    FT_DEVICE_LIST_INFO_NODE *devInfo;
    DWORD numDevs;
    // create the device information list
    ftStatus = FT_CreateDeviceInfoList(&numDevs);
    if (numDevs > 0) {
    // allocate storage for list based on numDevs
    devInfo = (FT_DEVICE_LIST_INFO_NODE*)malloc(sizeof(FT_DEVICE_LIST_INFO_NODE)*numDevs);
    // get the device information list
    ftStatus = FT_GetDeviceInfoList(devInfo,&numDevs);
        if (ftStatus == FT_OK) {
            for (DWORD i = 0; i < numDevs; i++) {
                QuspinDev devAd;
                devAd.SerialNumber = QString::fromLocal8Bit(devInfo[i].SerialNumber);
                devAd.ProductDescription = QString::fromLocal8Bit(devInfo[i].Description);
                result.append(devAd);
            }
        }
    }
    return result;
}

QList<QString>* MainWindow::getTextFromRaw(QList<QString> *textData, QString fileRaw, int quantityChannels, long long quantityMeasurement, int sampleRate)
{
    QString recordLine;
    long ms = 0;
    QFile inputFile;
    inputFile.setFileName(fileRaw);
    if(!inputFile.open(QFile::ReadOnly))
    {
       qDebug() << "Can't open file";
    }

    QDataStream inputStreamData(&inputFile);
    float64 outData;
    for (long z = 0; z < quantityMeasurement-1000; z++)
    {
        recordLine = QString::number(ms).rightJustified(7, '0') + "\t\t";
        for (int i = 0; i < quantityChannels; i++)
        {
            inputStreamData >> outData;
            recordLine += QString::number(outData,'f',6) + "\t\t";
        }
        recordLine = recordLine + "\n";
        textData->append(recordLine);
        ms += sampleRate;
    }

    inputFile.close();
    return textData;
}

void MainWindow::on_checkBoxInfinite_clicked()
{
    if (ui->checkBoxInfinite->isChecked())
    {
        ui->lineEditDuration->setEnabled(false);
    } else
    {
        ui->lineEditDuration->setEnabled(true);
    }
}

void MainWindow::on_pushButtonReboot_3_clicked()
{
    if (ui->comboBoxSelectComPort->currentIndex() == -1) return;
    serialPort[ui->comboBoxSelectComPort->currentIndex()]->write(REBOOT);
}

void MainWindow::on_pushButtonAStart_clicked()
{
    if (ui->comboBoxSelectComPort->currentIndex() == -1) return;
    serialPort[ui->comboBoxSelectComPort->currentIndex()]->write(AUTO_START);
}

void MainWindow::loadLogbook()
{
    QFile file("./logbook.jrn");

    if (file.open(QFile::ReadOnly))
    {
        QTextStream streamData(&file);
        while (!streamData.atEnd())
        {
           QString line = streamData.readLine();
           ui->textEditLogBook->append(line);
        }
        file.close();
    }

}

void MainWindow::on_pushButtonSave_clicked()
{
    QFile file("./logbook.jrn");

    if (file.open(QFile::WriteOnly))
    {
        QTextStream streamData(&file);
        streamData << ui->textEditLogBook->toPlainText();
        file.close();
    }

}

void MainWindow::on_pushButtonDateTime_clicked()
{
    QTime timeNow = QTime::currentTime();
    QDate dateNow = QDate::currentDate();
    QString line = dateNow.toString("dd.MM.yy") + " " + timeNow.toString("hh:mm:ss") + "\n";
    ui->textEditLogBook->append(line);
}



void MainWindow::on_actionCharts_triggered()
{
    on_pushButtonCPlot_clicked();
}

void MainWindow::on_actionClose_triggered()
{
    close();
}
