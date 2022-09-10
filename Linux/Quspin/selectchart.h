#ifndef SELECTCHART_H
#define SELECTCHART_H

#include <QWidget>
#include <QTableWidget>

namespace Ui {
class SelectChart;
}

class SelectChart : public QWidget
{
    Q_OBJECT

public:
    explicit SelectChart(QWidget *parent = nullptr);
    ~SelectChart();
    QTableWidget* getTable();
    QList<int> listChannels;


private slots:
    void on_pushButtonCheck_clicked();
    void on_pushButtonUnCheck_clicked();
    void on_pushButtonCancel_clicked();
    void on_pushButtonOK_clicked();

private:
    Ui::SelectChart *ui;
};

#endif // SELECTCHART_H
