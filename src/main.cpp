#include "picture_model.hpp"

#include <QtQml/QQmlApplicationEngine>
#include <QGuiApplication>
#include <QStandardPaths>
#include <QSettings>
#include <QDir>
#include <QTranslator>

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
  QCoreApplication::setApplicationName(STRING(APPLICATION_NAME));
  qmlRegisterType<GroupedPictureModel>("localhost.PictureModel", 1, 0, "GroupedPictureModel");
  qmlRegisterType<PictureCollection>("localhost.PictureModel", 1, 0, "PictureCollection");
  qmlRegisterType<PictureProvider>("localhost.PictureModel", 1, 0, "PictureProvider");

  // QString appConfig = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::AppLocalDataLocation);
  // QString appCache = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::AppConfigLocation);

  QGuiApplication app(argc, argv);

  QQmlApplicationEngine engine(QUrl(u"qrc:/mainqml/main.qml"_qs));

  Settings settings(&engine);
  QList<QObject*> rootObjects = engine.rootObjects();
  QObject*languageChangeList = rootObjects[0]->findChild<QObject*>("languageChangeList");
  QObject::connect(languageChangeList, SIGNAL(languageChanged(QString)),
                    &settings, SLOT(handleChangeLanguage(QString)));

  return app.exec();
}

#include "main.moc"
