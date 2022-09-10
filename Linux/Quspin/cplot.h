#ifndef CPLOT_H
#define CPLOT_H

#include <QWidget>
#include "qcustomplot.h"
#include "Dsp.h"
#include "selectchart.h"


namespace Ui {
class CPlot;
}

class CPlot : public QWidget
{
    Q_OBJECT

public:
    explicit CPlot(QWidget *parent = nullptr);
    ~CPlot();
    void setPathForSaveGpg(QString myPath);
    void setListNumberChannelsForChart(QList<int> list);// Функция для получения порядковых номеров каналов для вывода на графики постфактум
    QString getChannelFromSensor(QString channel); // Функция берет номер канала "cDAQ3Mod1/ai1" и преобразует в вид "1Z"
    void getSetGain(QString str); // Функция для получения значения setGain из основной формы


private slots:
    void on_pushButtonClose_clicked();
    void on_pushButtonSetting_clicked();
    void on_pushButtonShow_clicked();
    void ActionTimerChart();
    void on_pushButtonRefresh_clicked();
    void on_pushButtonSave_clicked();
    void on_pushButtonOpen_clicked();
    void on_pushButtonShowPost_clicked();

    void on_pushButtonRefreshPost_clicked();

private:
    Ui::CPlot *ui;
    QList<QString>getListChannels();
    QList<int> getListNumberChannelsForChart();// Функция для получения порядковых номеров каналов для вывода на графики
    QList<int> listChannelsForCharts; // порядковые номера каналов, которые надо выводить на графики
    int quantityOfChannels = 0; // Количество каналов, которые измеряем
    QTimer *timerChart;
    int num; // Используем для графиков
    int dimensionX; // Это размерность шкалы Х графиков
    int dimensionY; // Это размерность шкалы Y графиков
    void loadSettings(); // Это функция для начальной загрузки установочных значений
    void saveSettings(); // Это функция для сохранения установочных значений
    QCustomPlot *customPlot; // в этой переменной хранится форма для графиков
    QList<QVector<double>> listData; // здесь храним серии
    QVector<double> xD; // для временной шкалы Х. Она едина для всех.
    float *dataFiltred[1]; // Здесь храним данные пропущенные через фильтр
    Dsp::Filter* filter; // сам объект фильтра
    QList<QString> listNameOfChannelsForCharts; // названия каналов, которые надо выводить на графики
    QString path; // Содержит директорию, в которую сохраняются результаты эксперимента и графики
    SelectChart *sChart; // форма выбора каналов, которые выводить на графики при постанализе
    void printStringToTable(QTableWidget *w, int y, int x, QString str); //заполнение таблицы
    void printCheckBoxToTable(QTableWidget *w, int y, int x, bool check);//заполнение таблицы
    QFile fileForPost; // файл, по которому строим графики постфактум
    QStringList listChannelsSet; // названия каналов, которые надо выводить на графики постфактум
    QString setGain; // Коэффициент умножения выходного сигнала сенсоров QUSPIN
    double coefficient(QString coeff); // Функция для получения коэффициента для расчета графиков

};

#endif // CPLOT_H
