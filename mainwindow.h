#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QFileInfo>
#include <QDir>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onFileItemDoubleClicked(QListWidgetItem* item);
    void on_radioButton_toggled(bool checked);
    void on_backButton_clicked();
    void showContextMenu(const QPoint& pos);
    void on_searchPath_returnPressed();
    bool openf(const QFileInfo& fileInfo, const QString& realName);
    void setMemoryInfo();

private:
    Ui::MainWindow *ui;
    QDir current_dir;
    void updateDisplay();
    bool all_files;
};
#endif // MAINWINDOW_H
