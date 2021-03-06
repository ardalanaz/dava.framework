#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class QtMainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
   explicit QtMainWindow(QWidget *parent = 0);
   ~QtMainWindow();
    
private:
    void SetupMainMenu();
    
	void SetupToolBar();
	void DecorateWithIcon(QAction *decoratedAction, const QString &iconFilename);

    void SetupProjectPath();
    void SetupDockWidgets();
    
    void closeEvent(QCloseEvent *event);

    void SetupCustomColorsDock();
    void SetCustomColorsDockControlsEnabled(bool enabled);
        
private slots:

    void MenuFileWillShow();
	
	//reference
	void ApplyReferenceNodeSuffix();
        
private:
    Ui::MainWindow *ui;
};


#endif // MAINWINDOW_H
