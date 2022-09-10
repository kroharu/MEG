#ifndef SETTING_H
#define SETTING_H

#include <QDialog>
#include <QCheckBox>

namespace Ui {
class Setting;
}

class Setting : public QDialog
{
    Q_OBJECT

public:
    explicit Setting(QWidget *parent = nullptr);
    ~Setting();
    void setSensAxis(QString myAxis);
    QString getChannelFromSensor(QString channel);
private slots:
    void on_pushButtonCancel_clicked();
    void on_pushButtonOK_clicked();
    void on_Button_addString_clicked();
    void on_Button_deleteString_clicked();
    void on_pushButtonCheck_clicked();
    void on_pushButtonUncheck_clicked();
    void on_tableWidget_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void on_pushButtonZ_clicked();
    void on_pushButtonY_clicked();
    void on_pushButtonLoadChannels_clicked();

    void on_pushButton_clicked();

private:
    Ui::Setting *ui;
    void printCheckBoxToTable(int y, int x, bool check);
    void changeCheckBoxToTable(int y, int x, bool check);
    void printStringToTable(int y, int x, QString str);
    void checkQuantityCharts();
    QList<QCheckBox *> listCh1;
    QList<QCheckBox *> listCh2;
    QString setAxis;

};

#endif // SETTING_H
