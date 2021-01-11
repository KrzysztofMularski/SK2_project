#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <string.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <QDateTime>

bool MainWindow::is_data_correct()
{
    clearMat(1, spinNum);
    clearMat(2, spinNum);
    QRegExp re("^-?\\d+([.]\\d+)?$");
    std::string filename = "../qt_app/debug.txt";
    std::ofstream debugFile(filename.c_str());
    QString qdate = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss,zzz");
    std::string date = qdate.toStdString();
    int startFlag = 1;
    int error_flag = 0;

    for (int i=0; i<spinNum; i++)
        for (int j=0; j<spinNum; j++)
        {
            if (mat1[i][j].size() == 0)
                mat1[i][j] = "0";
            if (mat2[i][j].size() == 0)
                mat2[i][j] = "0";
        }
    for (int i=0; i<spinNum; i++)
        for (int j=0; j<spinNum; j++)
            if (!re.exactMatch(mat1[i][j]))
            {
                if(startFlag)
                {
                    debugFile << date << '\n';
                    startFlag = 0;
                }
                debugFile << "mat1[" << i << "][" << j << "] = ";
                debugFile << mat1[i][j].toStdString() << " is not a number\n";
                error_flag = 1;
            }
    for (int i=0; i<spinNum; i++)
        for (int j=0; j<spinNum; j++)
            if (!re.exactMatch(mat2[i][j]))
            {
                if(startFlag)
                {
                    debugFile << date << '\n';
                    startFlag = 0;
                }
                debugFile << "mat2[" << i << "][" << j << "] = ";
                debugFile << mat2[i][j].toStdString() << " is not a number\n";
                error_flag = 1;
            }
    debugFile.close();
    if (error_flag)
        return false;
    else
        return true;
}

bool MainWindow::savingFile()
{
    std::string filename = ui->paths_line->text().toStdString();
    if (!filename.size())
        return false;
    std::ofstream myfile(filename.c_str());
    int n = ui->mats_matrix->rowCount();
    if (myfile.is_open())
    {
        for (int i=0; i<n; i++)
        {
            for (int j=0; j<n; j++)
            {
                myfile << ui->mats_matrix->item(i, j)->text().toStdString();
                myfile << ';';
            }
            myfile << '\n';
        }
        myfile.close();
        return true;
    }
    else
        return false;
}

bool MainWindow::loadingFile(int matN)
{
    std::string filename = ui->pathl_line->text().toStdString();
    if (!filename.size())
        return false;
    std::ifstream myfile(filename.c_str());
    if (myfile.is_open())
    {
        std::string line;
        std::string number;
        std::istringstream iss;
        int state_flag = 0;
        if (matN == 1 || matN == 2)
            for (int i=0; getline(myfile, line); i++)
            {
                if(line[0] == ';')
                    continue;
                replace(line.begin(), line.end(), ';', ' ');
                iss.clear();
                iss.str(line);
                for (int j=0; j<spinNum; j++)
                {
                    iss >> number;
                    if (ui->matl_matrix->item(i, j) == nullptr)
                        ui->matl_matrix->setItem(i, j, new QTableWidgetItem(number.c_str()));
                    else
                        ui->matl_matrix->item(i, j)->setText(number.c_str());
                }
            }
        else if (matN == 12)
            for (int i=0; getline(myfile, line); i++)
            {
                if(line[0] == ';')
                    continue;
                replace(line.begin(), line.end(), ';', ' ');
                iss.clear();
                iss.str(line);

                if (state_flag == 1)
                {
                    for (int j=0; j<spinNum; j++)
                    {
                        iss >> number;
                        mat1[i-1][j] = number.c_str();
                    }
                    if (i >= spinNum)
                        state_flag = 2;
                }
                else if (state_flag == 2)
                {
                    for (int j=0; j<spinNum; j++)
                    {
                        iss >> number;
                        mat2[i-spinNum-1][j] = number.c_str();
                    }
                    if (i >= spinNum*2)
                        state_flag = 3;
                }
                else if (state_flag == 0)
                {
                    iss >> number;
                    spinNum = std::stoi(number);
                    ui->main_spinBox->setValue(spinNum);
                    state_flag = 1;
                }
                else if (state_flag == 3)
                    break;
            }
        myfile.close();
        return true;
    }
    else
        return false;
}

void MainWindow::clearMat(int n, int spinN)
{
    if (n==1)
    {
        for (int i=0; i<MAX_MAT_SIZE; i++)
            for (int j=0; j<MAX_MAT_SIZE; j++)
                if (i>=spinN || j>=spinN)
                    mat1[i][j] = "";
    }
    else
    {
        for (int i=0; i<MAX_MAT_SIZE; i++)
            for (int j=0; j<MAX_MAT_SIZE; j++)
                if (i>=spinN || j>=spinN)
                    mat2[i][j] = "";
    }
}

void MainWindow::setArr(int n)
{
    ui->matl_matrix->setColumnCount(spinNum);
    ui->matl_matrix->setRowCount(spinNum);
    ui->matl_matrix->setHorizontalHeaderLabels(hor_labels);
    ui->matl_matrix->setVerticalHeaderLabels(ver_labels);

    if (n==1)
    {
        clearMat(1, spinNum);
        for (int i=0; i<spinNum; i++)
            for (int j=0; j<spinNum; j++)
                if (ui->matl_matrix->item(i, j) == nullptr)
                    ui->matl_matrix->setItem(i, j, new QTableWidgetItem(mat1[i][j]));
                else
                    ui->matl_matrix->item(i, j)->setText(mat1[i][j]);
    }
    else
    {
        clearMat(2, spinNum);
        for (int i=0; i<spinNum; i++)
            for (int j=0; j<spinNum; j++)
                if (ui->matl_matrix->item(i, j) == nullptr)
                    ui->matl_matrix->setItem(i, j, new QTableWidgetItem(mat2[i][j]));
                else
                    ui->matl_matrix->item(i, j)->setText(mat2[i][j]);
    }
}

void MainWindow::init_headers_labels()
{
    QChar col_first = 0;
    QChar col_second = 0;
    QString new_col;
    for (int i=1; i<100; i++)
    {
        ver_labels.append(QString::number(i));
        new_col.append(QString::number(i)+'(');
        col_first = (i-1)/26+'A'-1;
        col_second = (i-1)%26+'A';
        if (col_first!='A'-1)
        {
            new_col.append(col_first);
            new_col.append(col_second);
        }
        else
            new_col.append(col_second);
        new_col.append(')');
        hor_labels.append(new_col);
        new_col = "";
    }
}
