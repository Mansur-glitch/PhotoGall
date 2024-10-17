#include <QtQml/QQmlApplicationEngine>
#include <QGuiApplication>

#define STR(str) #str
#define STRING(str) STR(str)
#ifndef MAIN_QML_PATH
#define MAIN_QML_PATH MISSING_PATH
#endif

int main(int argc, char** argv)
{
  QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine(STRING(MAIN_QML_PATH));
  return app.exec();
}
