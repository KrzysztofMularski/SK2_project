#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>

#define MAX_MAT_SIZE 100
#define ISIZE 5
#define DSIZE 20

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void connection_routine(char ch);
    void testing();
    void try_to_connect();
    void calcsCompleted();
    void evaluateResult();
    void evaluateQueue();
    void read_queue_pos();
    void read_data();
    void connection_error_occured();
    void state_changed();

    void set_enabled_all(bool x);

    bool is_data_correct();
    bool savingFile();
    bool loadingFile(int matN);
    void clearMat(int n, int spinN);
    void setArr(int n);
    void init_headers_labels();

private slots:
    void on_main_button_edit1_clicked();
    void on_main_button_edit2_clicked();
    void on_main_button_load_clicked();
    void on_main_button_calculate_clicked();
    void on_main_button_result_clicked();
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
    void on_prog_button_goback_clicked();
    void on_main_spinBox_valueChanged(int arg1);
    void on_sfull_button_goback_clicked();
    void on_scant_button_goback_clicked();
    void on_wrdata_button_goback_clicked();

private:
    Ui::MainWindow *ui;
    QTcpSocket *tcpSocket;
    int state;
    QString starting_text = "Witaj w aplikacji!";
    int spinNum;
    QString** mat1;
    QString** mat2;
    QStringList hor_labels;
    QStringList ver_labels;

    double prog_bar_step_size = 1;
    int prog_bar_step_counter = 0;
    int prog_bar_steps_total = 25;

    int current_row_col_id = 0;
    char* resultBuf;
    char* queueBuf;
    int result_flag = 0;
    int queue_flag = 0;
    int is_finished = 0;
};
#endif // MAINWINDOW_H
