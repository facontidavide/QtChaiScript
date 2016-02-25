QT += widgets
CONFIG += c++11

HEADERS         = highlighter.h \
                  mainwindow.h \
                  motorinterface.h \
                  scripteditor.h
SOURCES         = highlighter.cpp \
                  mainwindow.cpp \
                  main.cpp \
                  motorinterface.cpp \
                  bindchaiscript.cpp \
                  scripteditor.cpp

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/richtext/syntaxhighlighter
INSTALLS += target

QMAKE_CXXFLAGS += -O3

wince*: {
   addFiles.files = main.cpp mainwindow.cpp
   addFiles.path = .
   DEPLOYMENT += addFiles
}
