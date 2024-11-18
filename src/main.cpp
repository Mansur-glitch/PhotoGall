#include "picture_model.hpp"

#include <QtQml/QQmlApplicationEngine>
#include <QGuiApplication>
#include <QStandardPaths>
#include <QSettings>
#include <QDir>

#define STR(str) #str
#define STRING(str) STR(str)
#define MISS_DEFINITION(def) static_assert(false, "Missing definition: " STRING(def));

#ifndef MAIN_QML_PATH
MISS_DEFINITION(MAIN_QML_PATH)
#endif

#define APPLICATION_NAME PhotoGal

const char* confKeyStartDir = "startDirectory"; 


int main(int argc, char** argv)
{
  QCoreApplication::setApplicationName(STRING(APPLICATION_NAME));
  qmlRegisterType<GroupedPictureModel>("localhost.PictureModel", 1, 0, "GroupedPictureModel");
  qmlRegisterType<PictureCollection>("localhost.PictureModel", 1, 0, "PictureCollection");
  qmlRegisterType<PictureProvider>("localhost.PictureModel", 1, 0, "PictureProvider");

  // QString appConfig = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::AppLocalDataLocation);
  // QString appCache = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::AppConfigLocation);

  QGuiApplication app(argc, argv);
  QQmlApplicationEngine engine(STRING(MAIN_QML_PATH));
  return app.exec();
}
