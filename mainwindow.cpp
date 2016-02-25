
#include <QtWidgets>

#include "mainwindow.h"
#include "scripteditor.h"



//! [0]
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    scriptArea = new ScriptEditor(this);
    setupFileMenu();

    setCentralWidget(scriptArea);
    setWindowTitle(tr("Syntax Highlighter"));
}
//! [0]


void MainWindow::run()
{
    scriptArea->run();
}

void MainWindow::openFile()
{
    QString file = QFileDialog::getOpenFileName(this, "Open Script", "", "Chai Scripts (*.chai)");
    if(!file.isEmpty())
    {
        scriptArea->openFile( file );
    }
}

void MainWindow::setupFileMenu()
{
    QMenu *fileMenu = new QMenu(tr("&File"), this);
    menuBar()->addMenu(fileMenu);

    fileMenu->addAction(tr("&New"), scriptArea, SLOT(newFile()), QKeySequence::New);
    fileMenu->addAction(tr("&Open..."), this, SLOT(openFile()), QKeySequence::Open);
    fileMenu->addAction(tr("&Save"), scriptArea, SLOT(save()), QKeySequence::Save);
    fileMenu->addAction(tr("&Save as..."), scriptArea, SLOT(saveAs()), QKeySequence::SaveAs  );
    fileMenu->addAction(tr("&Run"), this, SLOT(run() ) );
    fileMenu->addAction(tr("E&xit"), qApp, SLOT(quit()), QKeySequence::Quit);
}

