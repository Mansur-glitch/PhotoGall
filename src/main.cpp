#include "picture_model.hpp"
#include "tree_model.hpp"

#include <QtQml/QQmlApplicationEngine>
#include <QGuiApplication>
#include <QStandardPaths>
#include <QSettings>
#include <QDir>
#include <QTranslator>
#include <qobject.h>
#include <qqml.h>
#include <stdexcept>

#define STR(str) #str
#define STRING(str) STR(str)
#define MISS_DEFINITION(def) static_assert(false, "Missing definition: " STRING(def));

#define MAIN_QML_PATH SDFJKLJJ
#ifndef MAIN_QML_PATH
MISS_DEFINITION(MAIN_QML_PATH)
#endif

#define APPLICATION_NAME PhotoGall

const char* confKeyStartDir = "startDirectory"; 

class Settings: public QObject
{
  Q_OBJECT
public:
  Settings(QQmlApplicationEngine* engine, QObject* parent = nullptr)
  : QObject(parent), m_engine(engine), m_translator()
  {} 
public slots:
  void handleChangeLanguage(QString newLang)
  {
    if (!m_translator.isEmpty()) {
        QCoreApplication::removeTranslator(&m_translator);
    }
    QString localizationFilePath = QCoreApplication::applicationDirPath() + "/"
                                   STRING(APPLICATION_NAME) + "_" + newLang + ".qm";
    bool result = m_translator.load(localizationFilePath);
    if (result) {
      QCoreApplication::installTranslator(&m_translator);
      m_engine->retranslate();
    } else {
      qDebug()<<"Translation not found";
    }
  }
private:
  QQmlApplicationEngine* m_engine;
  QTranslator m_translator;
};

int main(int argc, char** argv)
{
  try {
    QCoreApplication::setApplicationName(STRING(APPLICATION_NAME));
    qmlRegisterType<GroupedPictureModel>("localhost.PictureModel", 1, 0, "GroupedPictureModel");
    qmlRegisterType<PictureCollection>("localhost.PictureModel", 1, 0, "PictureCollection");
    qmlRegisterType<PictureProvider>("localhost.PictureModel", 1, 0, "PictureProvider");

    qmlRegisterType<DirectoryTreeModel>("localhost.DirectoryTreeModel", 1, 0, "DirectoryTreeModel");
    qmlRegisterType<DirectoryValidator>("localhost.DirectoryTreeModel", 1, 0, "DirectoryValidator");

    // QString appConfig = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::AppLocalDataLocation);
    // QString appCache = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::AppConfigLocation);

    QGuiApplication app(argc, argv);
    LoseFocusDetector::s_singletonInstance = LoseFocusDetector::construct(&app); 
    QQmlApplicationEngine engine;
    qmlRegisterSingletonInstance("localhost.DirectoryTreeModel", 1, 0, "LoseFocusDetector", LoseFocusDetector::s_singletonInstance);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     [](QObject* loadedObj)
                     {
                       if (! loadedObj) {
                        throw std::runtime_error("Failed to load Qml");
                       }
                     });
    engine.load(QUrl("qrc:/mainqml/main.qml"));

    Settings settings(&engine);
    QList<QObject*> rootObjects = engine.rootObjects();
    QObject* languageChangeList = rootObjects[0]->findChild<QObject*>("languageChangeList");
    QObject::connect(languageChangeList, SIGNAL(languageChanged(QString)),
                      &settings, SLOT(handleChangeLanguage(QString)));

    return app.exec();
  } catch (std::exception& e) {
    qDebug()<<e.what();
  }
}

#include "main.moc"
