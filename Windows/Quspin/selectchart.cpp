#include "selectchart.h"
#include "ui_selectchart.h"
#include <QCheckBox>
#include <QDebug>



SelectChart::SelectChart(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SelectChart)
{
    ui->setupUi(this);
}

SelectChart::~SelectChart()
{
    delete ui;
}

QTableWidget* SelectChart::getTable()
{
    QTableWidget *res;
    res = ui->tableWidget;
    return res;
}

void SelectChart::on_pushButtonCheck_clicked()
{
    for (int i = 0; i < ui->tableWidget->rowCount(); i++)
    {
        QWidget * cellWidget = ui->tableWidget->cellWidget(i, 2);
        QCheckBox * checkbox = dynamic_cast<QCheckBox*>(cellWidget);
        checkbox->setChecked(true);
    }
}

void SelectChart::on_pushButtonUnCheck_clicked()
{
    for (int i = 0; i < ui->tableWidget->rowCount(); i++)
    {
        QWidget * cellWidget = ui->tableWidget->cellWidget(i, 2);
        QCheckBox * checkbox = dynamic_cast<QCheckBox*>(cellWidget);
        checkbox->setChecked(false);
    }
}

void SelectChart::on_pushButtonCancel_clicked()
{
    close();
}

void SelectChart::on_pushButtonOK_clicked()
{
    listChannels.clear();
    for (int i = 0; i < ui->tableWidget->rowCount(); i++)
    {
        QWidget * cellWidget = ui->tableWidget->cellWidget(i, 2);
        QCheckBox * checkbox = dynamic_cast<QCheckBox*>(cellWidget);
        if (checkbox->isChecked()) listChannels.append(i);
    }
    close();
}
