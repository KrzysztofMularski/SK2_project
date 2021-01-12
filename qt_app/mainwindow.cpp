#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QtNetwork>

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    tcpSocket(new QTcpSocket(this))
{
    ui->setupUi(this);

    connect(tcpSocket, &QIODevice::readyRead, this, &MainWindow::read_data);
    typedef void (QAbstractSocket::*QAbstractSocketErrorSignal)(QAbstractSocket::SocketError);
    connect(tcpSocket, static_cast<QAbstractSocketErrorSignal>(&QAbstractSocket::errorOccurred), this, &MainWindow::connection_error_occured);
    connect(tcpSocket, &QAbstractSocket::stateChanged, this, &MainWindow::state_changed);

    state = 0;

    ui->stackedWidget->setCurrentWidget(ui->page_main);
    ui->statusbar->setFont(QFont("Segoe UI", 10));

    ui->statusbar->showMessage(starting_text);
    //printf("%s", ui->statusbar->currentMessage().toStdString().c_str());

    ui->matl_matrix->horizontalHeader()->setVisible(true);
    ui->mats_matrix->horizontalHeader()->setVisible(true);
    ui->main_spinBox->setValue(2);
    spinNum = ui->main_spinBox->value();
    ui->main_button_calculate->setEnabled(true);
    ui->main_button_result->setEnabled(false);

    ui->matl_matrix->setColumnCount(spinNum);
    ui->matl_matrix->setRowCount(spinNum);
    ui->mats_matrix->setColumnCount(spinNum);
    ui->mats_matrix->setRowCount(spinNum);

    mat1 = new QString* [MAX_MAT_SIZE];
    mat2 = new QString* [MAX_MAT_SIZE];
    for (int i=0; i<MAX_MAT_SIZE; i++)
    {
        mat1[i] = new QString [MAX_MAT_SIZE];
        mat2[i] = new QString [MAX_MAT_SIZE];
    }

    init_headers_labels();

    resultBuf = (char*)malloc((ISIZE*2+DSIZE+4)*sizeof(char));
    resultBuf[0] = '\0';
    queueBuf = (char*)malloc(ISIZE*sizeof(char));
    //temp:
    //ui->main_button_calculate->setEnabled(true);
    //ui->main_button_result->setEnabled(true);
    //loadingFile(12);
}

MainWindow::~MainWindow()
{
    delete ui;
    for (int i=0; i<MAX_MAT_SIZE; i++)
    {
        delete [] mat1[i];
        delete [] mat2[i];
    }
    delete [] mat1;
    delete [] mat2;
}

void MainWindow::on_main_spinBox_valueChanged(int arg1)
{
    spinNum = arg1;
}

void MainWindow::on_main_button_edit1_clicked()
{
    state = 1;
    ui->statusbar->showMessage("Pobieranie danych Macierzy 1..");
    setArr(1);
    ui->stackedWidget->setCurrentWidget(ui->page_matrix_load);
    ui->statusbar->showMessage("Edytowanie Macierzy 1..");
}

void MainWindow::on_main_button_edit2_clicked()
{
    state = 2;
    ui->statusbar->showMessage("Pobieranie danych Macierzy 2..");
    setArr(2);
    ui->stackedWidget->setCurrentWidget(ui->page_matrix_load);
    ui->statusbar->showMessage("Edytowanie Macierzy 2..");
}

void MainWindow::on_main_button_load_clicked()
{
    state = 12;
    ui->stackedWidget->setCurrentWidget(ui->page_path_load);
    ui->statusbar->showMessage("Wybierz plik z wszystkimi danymi.");
}

void MainWindow::on_main_button_calculate_clicked()
{
    if (is_data_correct())
        try_to_connect(); //connection handling
    else
    {
        ui->stackedWidget->setCurrentWidget(ui->page_wrong_data);
        ui->statusbar->showMessage("Zmień dane i spróbuj ponownie.");
    }
}

void MainWindow::on_main_button_result_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_matrix_save);
    ui->statusbar->showMessage("Rozwiązanie mnożenia macierzy.");
}

void MainWindow::on_matl_button_load_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_path_load);
    if (state == 1)
    {
        ui->statusbar->showMessage("Wybierz plik z Macierzą 1, aby wczytać dane.");
    }
    else if (state == 2)
    {
        ui->statusbar->showMessage("Wybierz plik z Macierzą 2, aby wczytać dane.");
    }

}

void MainWindow::on_matl_button_save_clicked()
{
    if (state == 1)
    {
        for (int i=0; i<spinNum; i++)
            for (int j=0; j<spinNum; j++)
                if (ui->matl_matrix->item(i, j) == nullptr || (ui->matl_matrix->item(i, j) != nullptr && ui->matl_matrix->item(i, j)->text() == ""))
                    mat1[i][j] = "";
                else
                    mat1[i][j] = ui->matl_matrix->item(i, j)->text();
        ui->statusbar->showMessage("Pomyślnie zapisano Macierz 1!");
    }
    else if (state == 2)
    {
        for (int i=0; i<spinNum; i++)
            for (int j=0; j<spinNum; j++)
                if (ui->matl_matrix->item(i, j) == nullptr || (ui->matl_matrix->item(i, j) != nullptr && ui->matl_matrix->item(i, j)->text() == ""))
                    mat2[i][j] = "";
                else
                    mat2[i][j] = ui->matl_matrix->item(i, j)->text();
        ui->statusbar->showMessage("Pomyślnie zapisano Macierz 2!");
    }
}

void MainWindow::on_matl_button_goback_clicked()
{
    state = 0;
    ui->stackedWidget->setCurrentWidget(ui->page_main);
    ui->statusbar->showMessage(starting_text);
}

void MainWindow::on_mats_button_saveas_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_path_save);
    ui->statusbar->showMessage("Podaj nazwę pliku, aby zapisać rozwiązanie.");
}

void MainWindow::on_mats_button_goback_clicked()
{
    MainWindow::on_matl_button_goback_clicked();
}

void MainWindow::on_pathl_button_browse_clicked()
{
    QString filename = "";
    if (state == 1)
        filename = QFileDialog::getOpenFileName(this, "Wybierz plik z Macierzą 1, aby wczytać dane.", QDir::currentPath()+"/../qt_app");
    else if (state == 2)
        filename = QFileDialog::getOpenFileName(this, "Wybierz plik z Macierzą 2, aby wczytać dane.", QDir::currentPath()+"/../qt_app");
    else if (state == 12)
        filename = QFileDialog::getOpenFileName(this, "Wybierz plik z wszystkimi danymi.", QDir::currentPath()+"/../qt_app");
    ui->pathl_line->setText(filename);
}

void MainWindow::on_pathl_button_load_clicked()
{
    if (state == 1)
    {
        loadingFile(1);
        ui->stackedWidget->setCurrentWidget(ui->page_matrix_load);
        ui->statusbar->showMessage("Pomyślnie wczytano dane Macierzy 1! Edytowanie..");
    }
    else if (state == 2)
    {
        loadingFile(2);
        ui->stackedWidget->setCurrentWidget(ui->page_matrix_load);
        ui->statusbar->showMessage("Pomyślnie wczytano dane Macierzy 2! Edytowanie..");
    }
    else if (state == 12)
    {
        loadingFile(12);
        ui->stackedWidget->setCurrentWidget(ui->page_main);
        ui->statusbar->showMessage("Pomyślnie wczytano dane obu Macierzy!");
        state = 0;
    }
}

void MainWindow::on_pathl_button_goback_clicked()
{
    if (state == 1)
    {
        ui->stackedWidget->setCurrentWidget(ui->page_matrix_load);
        ui->statusbar->showMessage("Edytowanie Macierzy 1..");
    }
    else if (state == 2)
    {
        ui->stackedWidget->setCurrentWidget(ui->page_matrix_load);
        ui->statusbar->showMessage("Edytowanie Macierzy 2..");
    }
    else if (state == 12)
    {
        ui->stackedWidget->setCurrentWidget(ui->page_main);
        ui->statusbar->showMessage(starting_text);
    }
}

void MainWindow::on_paths_button_browse_clicked()
{
    QString filename = "";
    filename = QFileDialog::getSaveFileName(this, "Podaj nazwę pliku, aby zapisać rozwiązanie", QDir::currentPath()+"/../qt_app");
    ui->paths_line->setText(filename);
}

void MainWindow::on_paths_button_save_clicked()
{
    savingFile();
    ui->stackedWidget->setCurrentWidget(ui->page_main);
    ui->statusbar->showMessage("Pomyślnie zapisano rozwiązanie do pliku!");
}

void MainWindow::on_paths_button_goback_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_matrix_save);
    ui->statusbar->showMessage("Rozwiązanie mnożenia macierzy.");
}

void MainWindow::on_prog_button_goback_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_main);
    ui->statusbar->showMessage(starting_text);
}

void MainWindow::on_sfull_button_goback_clicked()
{
    on_prog_button_goback_clicked();
}

void MainWindow::on_scant_button_goback_clicked()
{
    on_prog_button_goback_clicked();
}

void MainWindow::on_wrdata_button_goback_clicked()
{
    on_prog_button_goback_clicked();
}
