#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMenu>
#include <QAction>
#include <QTextStream>
#include <QFileDialog>
#include <QFile>
#include <QUrl>
#include <QDesktopServices>
#include <QProcess>
#include <QStorageInfo>

namespace fs = std::filesystem;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->listWidget->setViewMode(QListWidget::IconMode);
    ui->listWidget->setResizeMode(QListWidget::Adjust);
    ui->listWidget->setMovement(QListWidget::Static);
    ui->listWidget->setSpacing(15);
    current_dir = QDir::home();
    ui->searchPath->setText(current_dir.absolutePath());
    all_files = false;
    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::onFileItemDoubleClicked);
    connect(ui->listWidget, &QListWidget::customContextMenuRequested, this, &MainWindow::showContextMenu);
    updateDisplay();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateDisplay()
{
    ui->listWidget->clear();
    const int MAX_LEN = 8;
    QDir::Filters filters = QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden;
    QFileInfoList allEntries = current_dir.entryInfoList(filters, QDir::DirsFirst | QDir::Name);
    QFileInfoList entriesToDisplay;
    int totalCount = allEntries.count();
    int hidden = 0;
    for (const QFileInfo& entry: allEntries){
        if (entry.isHidden()){
            ++hidden;
            if (all_files){
                entriesToDisplay.append(entry);
            }
        } else {
            entriesToDisplay.append(entry);
        }
    }
    int listCount = entriesToDisplay.count();
    QString statusText;
    if (all_files){
        statusText = QString("%1 items").arg(totalCount);
    } else {
        if (hidden > 0){
            statusText = QString("%1 items (%2 hidden)").arg(listCount).arg(hidden);
        } else {
            statusText = QString("%1 items").arg(listCount);
        }
    }
    for (const QFileInfo& entry: entriesToDisplay){
        QString originalName = entry.fileName();
        QString displayName = originalName;
        if (displayName.length() > MAX_LEN){
            displayName = displayName.left(MAX_LEN) + "...";
        }
        QListWidgetItem* item = new QListWidgetItem(displayName, ui->listWidget);
        item->setData(Qt::UserRole, originalName);
        if (entry.isDir()){
            item->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
        } else {
            item->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
        }
    }
    ui->listCount->setText(statusText);
    ui->searchPath->setText(current_dir.absolutePath());
    setMemoryInfo();
    /*
    QDir::Filters filters = QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot;

    if (all_files){
        filters |= QDir::Hidden;
    }
    QFileInfoList entries = current_dir.entryInfoList(
        filters,
        QDir::DirsFirst | QDir::Name
    );
    int countTotal = current_dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden).count();
    int onlyHidden = current_dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden).count()
                     - current_dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot).count();
    QString statusText;
    int listCount = entries.count();
    if (all_files){
        statusText = QString("%1 items").arg(countTotal);
    } else {
        if (onlyHidden > 0)
            statusText = QString("%1 items (%2 hidden)").arg(listCount).arg(onlyHidden);
        else
            statusText = QString("%1 items").arg(listCount);
    }
    for (const QFileInfo& entry: entries){
        QString originalName = entry.fileName();
        QString displayName = originalName;

        if (displayName.length() > MAX_LEN){
            displayName = displayName.left(MAX_LEN) + "...";
        }
        QListWidgetItem* item = new QListWidgetItem(displayName, ui->listWidget);
        item->setData(Qt::UserRole, originalName);

        if (entry.isDir()){
            item->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
        } else {
            item->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
        }
    }*/
}

void MainWindow::setMemoryInfo()
{
    QStorageInfo storage(current_dir.absolutePath());
    storage.refresh();

    if (storage.isValid() && storage.isReady()){
        double memory_total = storage.bytesTotal() / 1024.0 / 1024.0 / 1024.0;
        double memory_available = storage.bytesAvailable() / 1024.0 / 1024.0 / 1024.0;
        ui->fileInfo->setText(QString("Free space: %1 GB (Total %2 GB)")
                                  .arg(memory_available)
                                  .arg(memory_total)
        );
    } else {
        ui->fileInfo->setText("Storage metrics unavailable");
    }
}

void MainWindow::on_radioButton_toggled(bool checked)
{
    all_files = checked;
    updateDisplay();
}

void MainWindow::onFileItemDoubleClicked(QListWidgetItem* item)
{
    if (!ui->listWidget->currentItem()) return;
    QString target = item->data(Qt::UserRole).toString();
    if (current_dir.cd(target)){
        updateDisplay();
    }

    if (!(ui->listWidget->count())){
        new QListWidgetItem("Nothing to show", ui->listWidget);
    }
}




void MainWindow::on_backButton_clicked()
{
    current_dir.cdUp();
    updateDisplay();
}





void MainWindow::on_searchPath_returnPressed()
{
    QString userInput = ui->searchPath->text();
    QDir newDir(userInput);
    if (newDir.exists(userInput)){
        current_dir = newDir;
        updateDisplay();
    } else {
        ui->searchPath->setText(current_dir.absolutePath());
    }
}

bool MainWindow::openf(const QFileInfo& fileInfo, const QString& realName){
    QString suffix = fileInfo.suffix().toLower();
    QString workdir = current_dir.absolutePath();

    QStringList code_ext = {"c", "py", "", "rs", "ts", "js", "cpp"};
    if (code_ext.contains(suffix)){
        QString appVi = "/usr/bin/vi";
        QString lxterminal = "/usr/bin/lxterminal";
        if (QFile::exists(lxterminal)){
            QStringList arguments;
            arguments << "-e" << appVi << realName;
            return QProcess::startDetached(lxterminal, arguments, workdir);
        }
    }
    QStringList docs_ext = {"doc", "docx", "docs"};
    if (docs_ext.contains(suffix)){
        return QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absoluteFilePath()));
    }

    if (suffix == "pdf" || suffix == "bin") {
        return QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absoluteFilePath()));
    }

    return QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absoluteFilePath()));
}


void MainWindow::showContextMenu(const QPoint& pos)
{
    QListWidgetItem* item = ui->listWidget->itemAt(pos);
    if (!item) return;

    QString realName = item->data(Qt::UserRole).toString();
    QMenu menu(this);
    QAction* openAction = menu.addAction("Open");
    //QAction* deleteAction = menu.addAction("Delete");
    QAction* selectedAction = menu.exec(ui->listWidget->mapToGlobal(pos));
    QString fullPath = current_dir.absoluteFilePath(realName);
    QFileInfo fileInfo(fullPath);

    if (selectedAction==openAction){
        if (fileInfo.isDir()){
            if (current_dir.cd(realName)){
                current_dir = fullPath;
                updateDisplay();
            }
        } else {
            bool success = openf(fileInfo, realName);
            if (!success) {
                qDebug() << "Failed to open the file. No default app found or access denied.";
            }
        }
    }

}
