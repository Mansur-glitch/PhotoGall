#include "picture_model.hpp"

#include <QtQml/QQmlApplicationEngine>
#include <QGuiApplication>

#define STR(str) #str
#define STRING(str) STR(str)
#define MISS_DEFINITION(def) static_assert(false, "Missing definition: " STRING(def));

#ifndef MAIN_QML_PATH
MISS_DEFINITION(MAIN_QML_PATH)
#endif

#define APPLICATION_NAME PhotoGal

int main(int argc, char** argv)
{
  QCoreApplication::setApplicationName(STRING(APPLICATION_NAME));
  qmlRegisterType<PictureListModel>("localhost.PictureListModel", 1, 0, "PictureListModel");
  QGuiApplication app(argc, argv);
  QQmlApplicationEngine engine(STRING(MAIN_QML_PATH));
  return app.exec();
}
