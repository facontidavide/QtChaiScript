#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QObject>
#include <QWidget>
#include <QCompleter>
#include <QTextEdit>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include <QMessageBox>
#include "highlighter.h"
#include "callbackdispatcher.h"
#include "motorinterface.h"

class CompletedText: public QTextEdit
{
    Q_OBJECT
public:
    CompletedText(QWidget *parent = 0);

    void addWordsToCompleter(QStringList new_words);

protected:
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void focusInEvent(QFocusEvent *e) Q_DECL_OVERRIDE;
private slots:
    void insertCompletion(const QString &completion);
private:
    QCompleter*   _completer;
    Highlighter*  _highlighter;
    QString textUnderCursor();
};

namespace chaiscript{
 class ChaiScript;
}


class ScriptWorker : public QObject
{
    Q_OBJECT

public:
    ScriptWorker(SimpleCallbackDispatcher* dispatcher);

public slots:
    void doWork(QString script);

signals:
    void error(QString error_msg);
    void finished();
    void print(const QString &msg);

private:
    chaiscript::ChaiScript* chai;
    void printProxy(const std::string & msg);

    void print_str(const std::string & text);
    void print_int(const int & num);
    void print_uns(const unsigned & num);
    void print_float(const float & num);
    void print_double(const double & num);
    MotorInterface* createMotorInterface(int node, int channel);

    SimpleCallbackDispatcher *_dispatcher;
};


class ScriptEditor : public QWidget
{
    Q_OBJECT
public:
    explicit ScriptEditor(QWidget *parent = 0);

signals:
    void executeScript(const QString &script);

public slots:
    void newFile();
    bool openFile(const QString &filename);
    bool save();
    bool saveAs();
    bool saveFile(const QString &name);
    QString currentFileP()const { return _currentFile; }
    void run();


    QTextEdit* editor()  { return _editor; }
    QTextEdit* console() { return _console; }

    void setDocumentModified();
private slots:
    void setCurrentFile(const QString &file);
    void doneRunning();
    void printOutput(QString output_message);
    void errorBox( QString error_msg);

    QMessageBox::StandardButton askToSave();

    void messageReceived();

private:

    QThread         _workerThread;
    CompletedText*  _editor;
    QTextEdit*      _console;
    QString         _currentFile;

    bool   _isUntitled;
    bool   _modified;
    bool   _running;

    SimpleCallbackDispatcher _dispatcher;

signals:
    void forwardFocusInEvent(QFocusEvent *e);

};

#endif // SCRIPTEDITOR_H
