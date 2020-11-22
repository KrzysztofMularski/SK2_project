#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_main_button_edit1_clicked();

    void on_main_button_edit2_clicked();

    void on_main_button_load_clicked();

    void on_main_button_calculate_clicked();

    void on_main_button_resoult_clicked();

    void on_matl_button_load_clicked();

    void on_matl_button_save_clicked();

    void on_matl_button_goback_clicked();

    void on_mats_button_saveas_clicked();

    void on_mats_button_goback_clicked();

    void on_pathl_button_browse_clicked();

    void on_pathl_button_load_clicked();

    void on_pathl_button_goback_clicked();

    void on_paths_button_browse_clicked();

    void on_paths_button_save_clicked();

    void on_paths_button_goback_clicked();

    void on_prog_button_stop_clicked();

    void on_prog_button_goback_clicked();

    void on_main_spinBox_valueChanged(int arg1);

    void on_sfull_button_try_again_clicked();

    void on_sfull_button_goback_clicked();

    void on_scant_button_try_again_clicked();

    void on_scant_button_goback_clicked();

    void on_wrdata_button_goback_clicked();

private:
    Ui::MainWindow *ui;
    QString starting_text = "Is it working?";
    QString filename1;  // plik z macierzą 1
    QString filename2;  // plik z macierzą 2
    QString filenameAll;    // plik ze wszystkimi danymi
    QString filenameSolution;   // plik z rozwiązaniem mnożenia macierzy
};
#endif // MAINWINDOW_H
