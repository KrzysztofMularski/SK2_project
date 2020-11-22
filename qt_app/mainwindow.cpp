#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <unistd.h>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentWidget(ui->page_main);
    ui->statusbar->setFont(QFont("Segoe UI", 12));

    ui->statusbar->showMessage(starting_text);
    //printf("%s", ui->statusbar->currentMessage().toStdString().c_str());

    ui->matl_matrix->horizontalHeader()->setVisible(true);
    ui->mats_matrix->horizontalHeader()->setVisible(true);

    ui->prog_button_stop->setEnabled(true);

    //temp:
    ui->main_button_calculate->setEnabled(true);
    ui->main_button_resoult->setEnabled(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_main_spinBox_valueChanged(int arg1)
{
    //printf("%i\n",arg1);
    //fflush(stdout);
    // to do: ustawianie wartości
}

void MainWindow::on_main_button_edit1_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_matrix_load);
    ui->statusbar->showMessage("Edytowanie Macierzy 1..");
}

void MainWindow::on_main_button_edit2_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_matrix_load);
    ui->statusbar->showMessage("Edytowanie Macierzy 2..");
}

void MainWindow::on_main_button_load_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_path_load);
    ui->statusbar->showMessage("Wybierz plik z wszystkimi danymi.");
}

void MainWindow::on_main_button_calculate_clicked()
{
    bool server_on = true; // server_on = is_server_on();
    int queue_pos = 1; // queue_pos = queue_current_pos();
    bool data_correct = true; // data_correct = is_data_correct();
    if (data_correct)
    {
        if(server_on)
        {
            if(queue_pos <= 0)
            {
                ui->stackedWidget->setCurrentWidget(ui->page_progress);
                ui->statusbar->showMessage("Obliczanie wyniku..");
                ui->prog_bar->setValue(0);

                // DO ZMIANY:
                // nowy wątek do ogarniania postępu, aby można było używać przycisku "Przerwij"
                for (int i=0; i<=100; i++)
                {
                    usleep(20000);
                    ui->prog_bar->setValue(i);
                }
                ui->prog_button_stop->setEnabled(false);

                ui->statusbar->showMessage("Ukończono! Kliknij \"Wróć\" aby wyjść");
                //obliczanie wyniku

            }
            else
            {
                ui->stackedWidget->setCurrentWidget(ui->page_server_full);
                ui->statusbar->showMessage("Serwer pełny! Spróbuj ponownie za kilka minut.");
            }
        }
        else
        {
            ui->stackedWidget->setCurrentWidget(ui->page_cannot_connect);
            ui->statusbar->showMessage("Spróbuj ponownie za kilka minut.");
        }
    }
    else
    {
        ui->stackedWidget->setCurrentWidget(ui->page_wrong_data);
        ui->statusbar->showMessage("Zmień dane i spróbuj ponownie.");
    }
}

void MainWindow::on_main_button_resoult_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_matrix_save);
    ui->statusbar->showMessage("Rozwiązanie mnożenia macierzy.");
}

void MainWindow::on_matl_button_load_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_path_load);
    if (ui->statusbar->currentMessage().toStdString() == "Edytowanie Macierzy 1..")
    {
        ui->statusbar->showMessage("Wybierz plik z Macierzą 1, aby wczytać dane.");
    }
    else if (ui->statusbar->currentMessage().toStdString() == "Edytowanie Macierzy 2..")
    {
        ui->statusbar->showMessage("Wybierz plik z Macierzą 2, aby wczytać dane.");
    }

}

void MainWindow::on_matl_button_save_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_main);
    if (ui->statusbar->currentMessage().toStdString() == "Edytowanie Macierzy 1..")
    {
        //to do: zapisywanie danych macierzy 1
        ui->statusbar->showMessage("Pomyślnie zapisano Macierz 1.");
    }
    else if (ui->statusbar->currentMessage().toStdString() == "Edytowanie Macierzy 2..")
    {
        //to do: zapisywanie danych macierzy 2
        ui->statusbar->showMessage("Pomyślnie zapisano Macierz 2.");
    }
}

void MainWindow::on_matl_button_goback_clicked()
{
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
    if (ui->statusbar->currentMessage().toStdString() == "Wybierz plik z Macierzą 1, aby wczytać dane.")
    {
        filename1 = QFileDialog::getSaveFileName(this, "Wybierz plik z Macierzą 1, aby wczytać dane.");
        ui->pathl_line->setText(filename1);
    }
    else if (ui->statusbar->currentMessage().toStdString() == "Wybierz plik z Macierzą 2, aby wczytać dane.")
    {
        filename2 = QFileDialog::getSaveFileName(this, "Wybierz plik z Macierzą 2, aby wczytać dane.");
        ui->pathl_line->setText(filename2);
    }
    else if (ui->statusbar->currentMessage().toStdString() == "Wybierz plik z wszystkimi danymi.")
    {
        filenameAll = QFileDialog::getSaveFileName(this, "Wybierz plik z wszystkimi danymi.");
        ui->pathl_line->setText(filenameAll);
    }
}

void MainWindow::on_pathl_button_load_clicked()
{
    //to do: ładowanie danych z pliku
    if (ui->statusbar->currentMessage().toStdString() == "Wybierz plik z Macierzą 1, aby wczytać dane.")
    {
        ui->stackedWidget->setCurrentWidget(ui->page_matrix_load);
        ui->statusbar->showMessage("Pomyślnie wczytano dane Macierzy 1! Edytowanie..");
    }
    else if (ui->statusbar->currentMessage().toStdString() == "Wybierz plik z Macierzą 2, aby wczytać dane.")
    {
        ui->stackedWidget->setCurrentWidget(ui->page_matrix_load);
        ui->statusbar->showMessage("Pomyślnie wczytano dane Macierzy 2! Edytowanie..");
    }
    else if (ui->statusbar->currentMessage().toStdString() == "Wybierz plik z wszystkimi danymi.")
    {
        ui->stackedWidget->setCurrentWidget(ui->page_main);
        ui->statusbar->showMessage("Pomyślnie wczytano dane obu Macierzy!");
    }
}

void MainWindow::on_pathl_button_goback_clicked()
{

    if (ui->statusbar->currentMessage().toStdString() == "Wybierz plik z Macierzą 1, aby wczytać dane.")
    {
        MainWindow::on_main_button_edit1_clicked();
    }
    else if (ui->statusbar->currentMessage().toStdString() == "Wybierz plik z Macierzą 2, aby wczytać dane.")
    {
        MainWindow::on_main_button_edit2_clicked();
    }
    else if (ui->statusbar->currentMessage().toStdString() == "Wybierz plik z wszystkimi danymi.")
    {
        ui->stackedWidget->setCurrentWidget(ui->page_main);
        ui->statusbar->showMessage(starting_text);
    }
}

void MainWindow::on_paths_button_browse_clicked()
{
    filenameSolution = QFileDialog::getSaveFileName(this, "Podaj nazwę pliku, aby zapisać rozwiązanie");
    ui->paths_line->setText(filenameSolution);
}

void MainWindow::on_paths_button_save_clicked()
{
    //to do: zapis do pliku
    ui->stackedWidget->setCurrentWidget(ui->page_main);
    ui->statusbar->showMessage("Pomyślnie zapisano rozwiązanie do pliku!");
}

void MainWindow::on_paths_button_goback_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_matrix_save);
    ui->statusbar->showMessage("Rozwiązanie mnożenia macierzy.");
}

void MainWindow::on_prog_button_stop_clicked()
{
    //to do: przerwanie obliczania, żeby móc wrócić
    ui->prog_button_goback->setEnabled(true);
}

void MainWindow::on_prog_button_goback_clicked()
{
    // to do: jak kliknie wróć to się rozłącza z serwerem
    ui->stackedWidget->setCurrentWidget(ui->page_main);
    ui->statusbar->showMessage(starting_text);
}

void MainWindow::on_sfull_button_try_again_clicked()
{
    MainWindow::on_main_button_calculate_clicked();
}

void MainWindow::on_sfull_button_goback_clicked()
{
    MainWindow::on_prog_button_goback_clicked();
}

void MainWindow::on_scant_button_try_again_clicked()
{
    MainWindow::on_main_button_calculate_clicked();
}

void MainWindow::on_scant_button_goback_clicked()
{
    MainWindow::on_prog_button_goback_clicked();
}

void MainWindow::on_wrdata_button_goback_clicked()
{
    MainWindow::on_prog_button_goback_clicked();
}
