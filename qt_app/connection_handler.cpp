#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtNetwork>
#include <unistd.h>
#include <QDebug>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 1234

void MainWindow::set_enabled_all(bool x)
{
    ui->address_button_goback->setEnabled(x);
    ui->connect_button->setEnabled(x);
    ui->server_address->setEnabled(x);
}

void MainWindow::try_to_connect()
{
    set_enabled_all(false);
    ui->statusbar->showMessage("Łączenie z serwerem..");
    tcpSocket->connectToHost(ui->server_address->text(), SERVER_PORT);
}

void MainWindow::connection_error_occured()
{
    set_enabled_all(true);
    ui->stackedWidget->setCurrentWidget(ui->page_cannot_connect);
    ui->statusbar->showMessage("Kliknij 'Wróć', aby powrócić do Menu");
}

void MainWindow::calcsCompleted()
{
    result_flag = 0;
    prog_bar_step_counter = 0;
    tcpSocket->close();
    ui->prog_button_goback->setEnabled(true);
    ui->prog_label_action->setText("Obliczenia skończone !");
    ui->statusbar->showMessage("Kliknij 'Wróć', aby powrócić do Menu");
    is_finished = 1;
}

void MainWindow::evaluateResult()
{
    int i;
    int j;
    char number[DSIZE];
    sscanf(resultBuf, "%d;%d;%s", &i, &j, number);
    if (ui->mats_matrix->item(i, j) == nullptr)
        ui->mats_matrix->setItem(i, j, new QTableWidgetItem(number));
    else
        ui->mats_matrix->item(i, j)->setText(number);
    resultBuf[0] = '\0';
    prog_bar_step_counter += spinNum;
    ui->prog_bar->setValue(int(prog_bar_step_counter*prog_bar_step_size));
    if (prog_bar_step_counter == prog_bar_steps_total)
        calcsCompleted();
}

void MainWindow::evaluateQueue()
{
    int queuePos;
    sscanf(queueBuf, "%d", &queuePos);
    if (queuePos == 0)
    {
        // wychodzimy z kolejki
        queue_flag = 0;
    }
    char str[10];
    sprintf(str, "%d", queuePos);
    ui->sfull_label_queue_pos->setText(str);
    ui->stackedWidget->setCurrentWidget(ui->page_server_full);
    queueBuf[0] = '\0';
}

void MainWindow::read_queue_pos()
{
    char ch;
    while (tcpSocket->bytesAvailable())
    {
        tcpSocket->getChar(&ch);
        if (ch == 'E')
        {
            printf("\n");
            fflush(stdout);
            evaluateQueue();
            continue;
        }
        strncat(queueBuf, &ch, 1);
        printf("%c", ch);
        fflush(stdout);
    }
}

void MainWindow::read_data()
{
    // klient w kolejce, ustalanie numerka :/
    if (queue_flag == 1)
    {
        read_queue_pos();
    }
    else if (!result_flag)
    {
        char checkpointBuf[2];
        tcpSocket->readLine(checkpointBuf, 2);
        if (checkpointBuf[0] == 'q')
        {
            ui->stackedWidget->setCurrentWidget(ui->page_server_full);
            ui->statusbar->showMessage("Kliknij 'Wróć', aby zerwać połączenie");
            printf("Im in a queue\n");
            fflush(stdout);
            queue_flag = 1;
            read_queue_pos();
        }
        else if (checkpointBuf[0] == 'o')
        {
            ui->stackedWidget->setCurrentWidget(ui->page_progress);
            ui->statusbar->showMessage("Obliczanie wyniku..");
            printf("Got o\n");
            fflush(stdout);
            connection_routine('o');
        }
        else if (checkpointBuf[0] == 'r')
        {
            printf("Got r\n");
            fflush(stdout);
            connection_routine('r');
        }
        else if (checkpointBuf[0] == 'c')
        {
            printf("Got c\n");
            fflush(stdout);
            connection_routine('c');
        }
    }
    else if (result_flag)
    {
        char ch;
        while (tcpSocket->bytesAvailable())
        {
            tcpSocket->getChar(&ch);
            if (ch == 'E')
            {
                printf("\n");
                fflush(stdout);
                evaluateResult();
                continue;
            }
            strncat(resultBuf, &ch, 1);
            printf("%c", ch);
            fflush(stdout);
        }
    }
}

void MainWindow::connection_routine(char ch)
{
    // wysłanie wartości stopnia macierzy
    if (ch == 'o')
    {
        char mat_order[ISIZE];
        sprintf(mat_order, "%d", spinNum);
        tcpSocket->write(mat_order);
        prog_bar_step_counter++;
        ui->prog_bar->setValue(int(prog_bar_step_counter*prog_bar_step_size));
    }
    // wysyłanie wiersza macierzy 1
    else if (ch == 'r')
    {
        for (int j=0; j<spinNum; j++)
        {
            tcpSocket->write(mat1[current_row_col_id][j].toStdString().c_str());
            tcpSocket->write("|");
        }
        prog_bar_step_counter++;
        ui->prog_bar->setValue(int(prog_bar_step_counter*prog_bar_step_size));
        current_row_col_id++;
        if (current_row_col_id == spinNum)
            current_row_col_id = 0;
    }
    // wysyłanie wiersza macierzy 2
    else if (ch == 'c')
    {
        for (int j=0; j<spinNum; j++)
        {
            tcpSocket->write(mat2[j][current_row_col_id].toStdString().c_str());
            tcpSocket->write("|");
        }
        prog_bar_step_counter++;
        ui->prog_bar->setValue(int(prog_bar_step_counter*prog_bar_step_size));
        current_row_col_id++;
        if (current_row_col_id == spinNum)
        {
            result_flag = 1;
            current_row_col_id = 0;
        }
    }
}

void MainWindow::state_changed()
{
    if (tcpSocket->state() == QTcpSocket::ConnectedState)
    {
        set_enabled_all(true);
        ui->main_button_result->setEnabled(true);
        qDebug() << "CONNECTED";
        is_finished = 0;
        ui->prog_button_goback->setEnabled(false);

        ui->prog_label_action->setText("Obliczanie wyniku mnożenia dwóch macierzy...");
        prog_bar_step_counter = 0;
        current_row_col_id = 0;
        result_flag = 0;
        queue_flag = 0;
        prog_bar_steps_total = 2*spinNum + 1+spinNum*spinNum*spinNum;
        prog_bar_step_size = 100.0/(prog_bar_steps_total);
        ui->prog_bar->setValue(0);

        ui->mats_matrix->setColumnCount(spinNum);
        ui->mats_matrix->setRowCount(spinNum);
    }
    else if (tcpSocket->state() == QTcpSocket::UnconnectedState)
    {
        set_enabled_all(true);
        qDebug() << "DISCONNECTED";
        //zerwano połączenie
        if (is_finished)
            ui->stackedWidget->setCurrentWidget(ui->page_cannot_connect);
    }

}
