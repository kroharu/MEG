#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//Объявляем контстанты для фиксированных команд
#define AUTO_START ">"
#define FIELD_ZERO_ON "D"
#define FIELD_ZERO_OFF "E"
#define X_Y_Z "i"
#define X_Y "h"
#define FIELD_RESET "V"
#define CALIBRATE "9"
#define Z_AXIS_MODE "C"
#define Y_AXIS_MODE "F"
#define DUAL_AXIS "B"
#define MODE_033X "a"
#define MODE_1X "`"
#define MODE_3X "b"
#define REBOOT "e"
#define PRINT_ON "7"
#define PRINT_OFF "8"



#include <QApplication>
#include <QWidget>
#include <QMainWindow>
#include <QTimer>
#include <QTime>
#include <QDate>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QtSerialPort/QtSerialPortDepends>
#include <QtSerialPort/QtSerialPortVersion>
#include <QByteArray>
#include <QSettings>
#include "workwithfiles.h"
#include "NIDAQmx.h"
#include <QThread>
#include "measurement.h"
#include "cplot.h"
#include "setting.h"




QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    Ui::MainWindow *ui;
    CPlot *cpl = nullptr; //Форма для графиков
    Setting *set; //Форма для сеттингов

    struct QuspinDev // Получаем из драйвера FTD2XX
    {
        QString SerialNumber;
        QString ProductDescription;
    };

    struct QuspinInf // Для таблицы соответствия "COM25 - QZFM-00IV"
    {
        QString Name;
        QString ProductDescription;
    };

    float64* dataArray;

protected:
    void closeEvent(QCloseEvent *event);

private slots:

    void readDataComPort(); // Чтение данных из COM Port
    void ActionTimer(); // Фунция вызывается для каждого измерения
    void on_pushButtonRescan_clicked(); // Сканирование портов на наличие сенсоров
    void resizeEvent(QResizeEvent* event); // Функция сабатывает при изменении размера формы
    void on_pushButtonStartStop_clicked(); // Функция для запуска и остановки эксперимента
    void on_pushButtonLoad_clicked(); // Функция посылает команду в заданный Com Port
    void on_pushButtonLoadAll_clicked(); // Функция посылает команду во все порты
    void sendFixCommand(); // Функция вызывается при нажатии кнопок - команд для сенсоров
    void saveSetting(); // Сохраняет настройки в файл
    void loadSetting(); // Загружает настройки из файла
    void on_actionSetting_triggered(); // открывает форму настроек
    void on_pushButtonClose_clicked(); // пока оставил для разного тестирования. По умолчанию не активна
    void on_pushButtonCPlot_clicked(); // Показывает форму с графиками
    void on_tableWidget_cellDoubleClicked(int row, int column); // Вызывается по двойному щелчку по основной таблице.Пока не использую
    void on_checkBoxInfinite_clicked();
    void on_pushButtonReboot_3_clicked();
    void on_pushButtonAStart_clicked();
    void on_pushButtonSave_clicked();
    void on_pushButtonDateTime_clicked();

    void on_actionCharts_triggered();

    void on_actionClose_triggered();

private:
    QTimer *timer; // таймер для измерений, задает частоту измерений
    QList<QString> *myData; // здесь хранятся результаты экспериментов для записи в файл
    QList<QString> *myConf; // здесь хранятся данные о сенсорах, для записи в файл
    int quantitySteps; // число прошедших секунд с начала эксперимента, нужно для индикатора "прогрессии"
    long long totalOperacion; // сколько всего должно быть сделано измерений, исходя из длительности эксперимента и периода опроса
    int quantityComPorts; // сколько сенсоров подключено к системе
    int quantityChannels; // сколько каналов аналогового сигнала будет измеряться в эксперименте
    int duration; // продолжительность эксперимента в секундах
    void setForBegin(); // настраивает первоначальный вид таблицы, кол-во столбцов и шапку. Возможно будет модифицироваться..
    void resizeTablWidget(); // настраивает вид таблицы при изменении размера окна
    void printStringToTable(int y, int x, QString str);// Вносит данные в таблицу
    void printToTable(int y, int x, QString str, QColor color); // Вносит данные и цвет в таблицу
    bool addDescription(); // готовит данные о сенсорах из таблицы для записи в добавочный файл expХХХ.qs2
    QList<QuspinInf> scanComPorts(); // фнкция для получения списка названий и описаний доступных портов
    QList<QuspinInf> listPorts; // список названий и описаний доступных портов, нужен для обращения к ним
    QSerialPort *serialPort[100]; // список COM портов
    void startStopCommand(QString command); // посылает выбранную команду во все порты
    QList<QString> getListChannels(); // Возвращает  QList<QString> со списком каналов для DAQ NI-9205
    QList<QString> listChannelsForFile; // Возвращает список каналов, которые потом записываем в шапки файлов
    int getQuantityChannels(QList<QString> listChan); // Возвращает количество каналов
    void clearTable(int i);
    QList<QuspinDev> getAddInformComPorts(); // Здесь получаем название сенсоров
    QList<int> listChannelsForCharts; // порядковые номера каналов, которые надо выводить на графики
    QString forGetStringFromComPort[100]; // Для чтения из COM портов, когда строки приходят не полностью, здесь запоминаем
    QString setAxis = "Z"; // Для записи в дополнительный файл эксперимента значения оси измерений
    QString setGain = "1X";// Для записи в дополнительный файл эксперимента значения делителя выходного напряжения
    QString forGetValueOfCalibrate[100]; // Здесь запоминаем значения калибровки сенсоров для записи в файл *.qs2
    // Это функция из файла с "сырыми данными" преобразует информацию в текст и добавляет в QList для записи в окончательный файл
    QList<QString>* getTextFromRaw(QList<QString> *textData, QString fileRaw, int quantityChannels, long long quantityMeasurement, int sampleRate);
//    QList<int> correctTime; // здесь заносим временные интервалы, которые затрачивает программа на обработку данных с DAQ
    void loadLogbook(); // Загрузка логбука при старте программы
    //Вводим переменные для организации отдельного потока для измерений
    QThread thread; // Измерения запускаем в отдельном потоке, чтобы не блокировался интерфейс
    Measurement measur; // Объект для осуществления измерений

    //переменные для регистрации событий ошибок и записи в *.log файл
    bool statusLed[100]; // здесь будет храниться предыдущее состояние сенсора, для сравнения с текущим, для регистрации события изменения состояния
    int msFromStart; // эта переменная нужна для получения значения текущего времени в миллисекундах при старте эксперимента, нужна для расчета времени наступления событий
    QList<QString> *myLog; // здесь хранятся события когда сенсор переходит в нерабочий режим для записи потом в файл....
    void checkStatusSensor(int i, bool status);
    QString absolutPath; // здесь хранится полный путь до папки, где будут записываться результаты эксперимента
};
#endif // MAINWINDOW_H
