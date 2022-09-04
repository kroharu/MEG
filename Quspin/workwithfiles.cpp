#include "workwithfiles.h"
#include <QTextEdit>


WorkWithFiles::WorkWithFiles()
{

}

bool WorkWithFiles::writeData(QString path, QList<QString> data, QList<QString> conf, QList<QString> logf){
    //Создание новой директории
    QDir dir;
    QFile mFile;
    QFile mFileExt;
    QFile mFileLog;
//    QString myPath = "./DATA/"+path; // Пусть куда записываем файлы
    QString myPath = path; // Пусть куда записываем файлы
    dir.setPath(myPath);
    if (!dir.exists()){ //Если нет директории
        dir.setPath("");
        if (!dir.mkpath(myPath)){ //Создаем и проверяем создали или нет?
            QMessageBox *bx = new QMessageBox();
            bx->setText("Can't create directory: "+myPath);
            bx->show();
            return false;
        }
        dir.setPath(myPath);
    }
    if (dir.isEmpty()){//Если директория пустая, делаем первые два файла
        mFile.setFileName(myPath + "/exp001.qs1");
        mFileExt.setFileName(myPath + "/exp001.qs2");
        mFileLog.setFileName(myPath + "/exp001.log");
    } else{
        QStringList filter;
        filter.append("exp*.qs1"); //Создаем фильтр для поиска нужных файлов
        QStringList files = dir.entryList(filter,QDir::Files);
        QString lastNumFile = files.last().remove(0,3).remove(3,4);
        int nFile = lastNumFile.toInt()+1; //Присваиваем следующий номер
        QString numberOfFile;
        if (nFile < 10) numberOfFile = "00"+QString::number(nFile);
        else if (nFile < 100) numberOfFile = "0"+QString::number(nFile);
        else numberOfFile = QString::number(nFile);
        mFile.setFileName(myPath + "/exp" + numberOfFile + ".qs1");
        mFileExt.setFileName(myPath + "/exp" + numberOfFile + ".qs2");
        mFileLog.setFileName(myPath + "/exp" + numberOfFile + ".log");
        fileDataName = myPath + "/exp" + numberOfFile + ".qs1";
    }

    //Создание файлов

    if((!mFile.open(QFile::WriteOnly))||(!mFileExt.open(QFile::WriteOnly))||(!mFileLog.open(QFile::WriteOnly)))
    {
        QMessageBox *bx = new QMessageBox();
        bx->setText("Path not correct!");
        bx->show();
        return false;
    }
    QTextStream streamData(&mFile);// Передали путь к файлу переменной stream
    QTextStream streamConf(&mFileExt);// Передали путь к файлу переменной stream
    QTextStream streamLog(&mFileLog);// Передали путь к файлу переменной stream
    //Записываем все что надо...
    for (QString str : data){
        streamData << str;
    }
    mFile.close();
    for (QString str : conf){
        streamConf << str;
    }
    mFileExt.close();
    for (QString str : logf){
        streamLog << str;
    }
    mFileLog.close();

    return true;
}
