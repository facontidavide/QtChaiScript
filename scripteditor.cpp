#include "scripteditor.h"
#include <QSettings>
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QFileInfo>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QSplitter>
#include <QStringListModel>
#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QKeyEvent>
#include <QDebug>
#include <QStringListModel>

ScriptEditor::ScriptEditor(QWidget *parent) : QWidget(parent)
{
    _editor = new CompletedText(this);
    _console  = new QTextEdit(this);

    QHBoxLayout *layout = new QHBoxLayout(this);
    QSplitter*  splitter = new QSplitter(this);

    splitter->setOrientation(Qt::Vertical);
    splitter->addWidget(editor());
    splitter->addWidget(_console);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);

    layout->addWidget(splitter);
    this->setLayout(layout);

    _console->setReadOnly( true );
    _console->setStyleSheet("QTextEdit { color: white; background-color: rgb(20, 20, 20) }");

    //------------------------
    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(9);

    editor()->setFont(font);
    editor()->setAcceptRichText( false );

    //----------------------------------
    _running = false;
    ScriptWorker *worker = new ScriptWorker( &_dispatcher  );
    worker->moveToThread(&_workerThread);

    connect(&_workerThread, &QThread::finished,     worker, &QObject::deleteLater);
    connect(this,   &ScriptEditor::executeScript,   worker, &ScriptWorker::doWork);
    connect(worker, &ScriptWorker::finished,        this,   &ScriptEditor::doneRunning);
    connect(worker, &ScriptWorker::print,           this,   &ScriptEditor::printOutput);
    connect(worker, &ScriptWorker::error,           this,   &ScriptEditor::errorBox);

    _workerThread.start();
}

void ScriptEditor::newFile()
{
    static int sequence = 1;
    _isUntitled = true;
    _currentFile = tr("newScript%1.chai").arg(sequence++);

    this->setWindowTitle(tr("%1[*]").arg(_currentFile));

    QSettings settings("Pal", "ChaiScript");
    settings.beginGroup("ChaiEditor");

    QString programmer = settings.value("programmer").toString().toLatin1();
    settings.endGroup();

    editor()->setText(tr("// default test "));
    _modified = (false);
    connect(editor(), SIGNAL(textChanged()), this, SLOT(setDocumentModified()));
}

bool ScriptEditor::openFile(const QString &filename)
{
    QFile f(filename);
    if(!f.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this, "Unable to open file!",
                             tr("Could not read file %1.\nError string: %2")
                             .arg(_currentFile, f.errorString()), QMessageBox::Ok,
                             QMessageBox::NoButton);
        return false;
    }
    QTextStream stream(&f);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    editor()->setText(stream.readAll());
    QApplication::restoreOverrideCursor();

    setCurrentFile( filename );

    connect(editor(), SIGNAL(textChanged()), this, SLOT(setDocumentModified()));
    return true;
}

void ScriptEditor::setCurrentFile(const QString &file)
{
    _currentFile = QFileInfo(file).canonicalFilePath();
    _isUntitled = false;
    _modified = false;
    this->setWindowTitle(tr("%1[*]").arg(_currentFile));
}

void ScriptEditor::doneRunning()
{
    _running = false;
}

bool ScriptEditor::save()
{
    if(_isUntitled)
        return saveAs();
    else
        return saveFile(_currentFile);
}

bool ScriptEditor::saveAs()
{
    QString file = QFileDialog::getSaveFileName(this, "Save As...", "", "ChaiScripts (*.chai)");

    if(file.isEmpty())
        return false;

    return saveFile(file);
}

bool ScriptEditor::saveFile(const QString &name)
{
    QFile file(name);
    if(!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this, "Unable to save file!",
                             tr("Could not save file %1.\nError string: %2")
                             .arg(name, file.errorString()), QMessageBox::Ok,
                             QMessageBox::NoButton);
        return false;
    }
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QTextStream stream(&file);
    stream << editor()->toPlainText();
    setCurrentFile(name);
    QApplication::restoreOverrideCursor();
    return true;
}

bool _kill_switch_ = false;

void ScriptEditor::run()
{

    if( _running ){
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(0, "Warning", "Previous script still running.\nDo you want to stop it?",  QMessageBox::Yes| QMessageBox::No);

        if( reply == QMessageBox::Yes){
            qDebug() << "kill";
            _kill_switch_ = true;
        }
        else{
            _dispatcher.invoke(666);
        }
    }
    else{
        _running = true;
        _console->clear();
        emit executeScript( _editor->toPlainText() );
    }
}

void ScriptEditor::printOutput(QString output_message)
{
    _console->append( output_message );
}

void ScriptEditor::errorBox(QString error_msg)
{
    QMessageBox::warning(0,"Error", error_msg, QMessageBox::Ok);
}

void ScriptEditor::setDocumentModified()
{
    _modified = true;
}


QMessageBox::StandardButton ScriptEditor::askToSave()
{
    if( _modified )
    {
        QMessageBox::StandardButton ret =
                QMessageBox::warning(0, "Please confirm!",
                                     tr("Your script with name %1 has been modified\nWould you like to save the changes you've made?").arg(_currentFile),
                                     QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        return ret;
    }
    return QMessageBox::NoButton;
}

void ScriptEditor::messageReceived()
{
    //dispatch to all the
}

//-------------------------------------------------------

CompletedText::CompletedText(QWidget *parent): QTextEdit(parent)
{
    this->setTabStopWidth( 20 );

    QStringList keywords;
    keywords << "while" << "var" << "def" << "print"
             << "try" << "catch" << "if" << "true" << "false"
             << "for" << "else" << "return"
             << "attr" << "break" << "fun" << "class" ;

    // don't forget this
    keywords.sort();

    _completer = new QCompleter(keywords, parent);

    _completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    _completer->setCaseSensitivity(Qt::CaseInsensitive);

    _completer->setWrapAround(false);
    _completer->setWidget( this);
    _completer->setCompletionMode(QCompleter::PopupCompletion);

    QObject::connect(_completer, SIGNAL(activated(QString)),
                     this, SLOT(insertCompletion(QString)));
    //-----------------------------------------
    QStringList keywordsPattern;

    foreach (const QString & word, keywords) {
        keywordsPattern << (QString("\\b") + word + QString("\\b") );
    }

    _highlighter = new Highlighter( &keywordsPattern, this->document());

}

void CompletedText::addWordsToCompleter(QStringList new_words)
{
    QStringListModel* model = dynamic_cast<QStringListModel*>( _completer->model() );
    QStringList new_stringlist = model->stringList();
    new_stringlist += new_words;
    new_stringlist.sort();
    model->setStringList( new_stringlist );

    foreach (const QString & word, new_words) {
        QString pattern = (QString("\\b") + word + QString("\\b") );
        _highlighter->appendKeyword( pattern );
    }
}

void CompletedText::keyPressEvent(QKeyEvent *e)
{
    QCompleter *c = _completer;

    if (c && c->popup()->isVisible()) {
        // The following keys are forwarded by the completer to the widget
        switch (e->key()) {
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Escape:
            case Qt::Key_Tab:
            case Qt::Key_Backtab:
                e->ignore();
                return; // let the completer do default behavior
            default:
                break;
        }
    }

    bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E); // CTRL+E
    if (!isShortcut) // do not process the shortcut when we have a completer
    {
        QTextEdit::keyPressEvent(e);
    }

    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if ( ctrlOrShift && e->text().isEmpty() )
    {
        return;
    }

    static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
    bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();

    if (!isShortcut && (hasModifier || e->text().isEmpty()||
                        completionPrefix.length() < 2 ||
                        eow.contains(e->text().right(1))))
    {
        c->popup()->hide();
        return;
    }

    if (completionPrefix != c->completionPrefix())
    {
        c->setCompletionPrefix(completionPrefix);
        c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
    }
    QRect cr = this->cursorRect();
    cr.setWidth(c->popup()->sizeHintForColumn(0)
                + c->popup()->verticalScrollBar()->sizeHint().width());

    c->complete(cr); // popup it up!
}

void CompletedText::insertCompletion(const QString& completion)
{
    QTextCursor tc = textCursor();

    int extra = completion.length() - _completer->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);
}

QString CompletedText::textUnderCursor()
{
    QTextCursor tc = this->textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

void CompletedText::focusInEvent(QFocusEvent *e)
{
    _completer->setWidget( this );
    QTextEdit::focusInEvent(e);
}


