#include <QTextEdit>
#include <QDebug>
#include <QMessageBox>
#include "scripteditor.h"
#include "chaiscript/chaiscript.hpp"
#include "motorinterface.h"
#include "chaiscript/chaiscript_stdlib.hpp"

void ScriptWorker::printProxy(const std::string & msg)
{
    emit print( QString(msg.c_str()) );
    QThread::currentThread()->msleep(2);
}

void ScriptWorker::print_str(const std::string & text)
{
    emit print( QString::fromStdString( text));
    QThread::currentThread()->msleep(2);
}

void ScriptWorker::print_int(const int & num)
{
    emit print( QString::number(num));
    QThread::currentThread()->msleep(2);
}

void ScriptWorker::print_uns(const unsigned & num)
{
    emit print( QString::number(num));
    QThread::currentThread()->msleep(2);
}

void ScriptWorker::print_float(const float & num)
{
    emit print( QString::number(num));
    QThread::currentThread()->msleep(2);
}

void ScriptWorker::print_double(const double & num)
{
    emit print( QString::number(num));
    QThread::currentThread()->msleep(2);
}

ScriptWorker::ScriptWorker()
{
    chai = 0;
}

void ScriptWorker::doWork(QString script)
{
    chai = new chaiscript::ChaiScript(chaiscript::Std_Lib::library());

    chai->add( chaiscript::fun(&ScriptWorker::print_str, this), "print");
    chai->add( chaiscript::fun(&ScriptWorker::print_uns, this), "print");
    chai->add( chaiscript::fun(&ScriptWorker::print_int, this), "print");
    chai->add( chaiscript::fun(&ScriptWorker::print_float, this), "print");
    chai->add( chaiscript::fun(&ScriptWorker::print_double, this), "print");

    chai->add(chaiscript::user_type<MotorInterface>(), "MotorInterface");
    chai->add(chaiscript::constructor<MotorInterface (int,int)>(), "MotorInterface");
    chai->add(chaiscript::fun(&MotorInterface::setTargetPosition), "setTargetPosition");
    chai->add(chaiscript::fun(&MotorInterface::getActualPosition), "getActualPosition");

    try {
        _kill_switch_ = false;
       chai->eval( script.toStdString().c_str() );

    } catch (const double num) {
        emit error( QString::number( num ) );
    } catch (int num) {
        emit error( QString::number( num ) );
    } catch (float num) {
        emit error( QString::number( num ) );
    } catch (const std::string &msg) {
        emit error( QString::fromStdString( msg ) );
    }catch (const std::exception  &e) {
        emit error( QString::fromStdString( e.what()) );
    }

    delete chai;
    chai = 0;

    emit finished();
}


