#include "picture_provider.hpp"
#include "tree_model.hpp"
#include "picture_list_model.hpp"
#include "utility.hpp"

#include <QtQml/QQmlApplicationEngine>
#include <QGuiApplication>
#include <QStandardPaths>
#include <QSettings>
#include <QDir>
#include <QTranslator>
#include <qnamespace.h>
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

  Q_INVOKABLE bool getToolPanelPosition() const noexcept
  {
    if (! m_settings.contains("toolPanelPosition")) {
      return true;
    }
    return m_settings.value("toolPanelPosition").toBool();
  }
  Q_INVOKABLE void setToolPanelPosition(bool beforeMainPanel)
  {
    m_settings.setValue("toolPanelPosition", beforeMainPanel);
  }
  Q_INVOKABLE bool getLayoutOrientationAutoChange() const noexcept
  {
    if (! m_settings.contains("layoutOrientationAutoChange")) {
      return true;
    }
    return m_settings.value("layoutOrientationAutoChange").toBool();
  }
  Q_INVOKABLE void setLayoutOrientationAutoChange(bool changeAutomatically)
  {
    m_settings.setValue("layoutOrientationAutoChange", changeAutomatically);
  }
  Q_INVOKABLE Qt::Orientation getLayoutOrientation() const noexcept
  {
    if (! m_settings.contains("layoutOrientation")) {
      return Qt::Orientation::Horizontal;
    }
    return QVariant(m_settings.value("layoutOrientation")).value<Qt::Orientation>();
  }
  Q_INVOKABLE void setLayoutOrientation(Qt::Orientation orientation)
  {
    m_settings.setValue("layoutOrientation", orientation);
  }
  Q_INVOKABLE int getLanguageNum() const noexcept
  {
    if (! m_settings.contains("languangeNum")) {
      return 0;
    }
    return m_settings.value("languangeNum").toInt();
  }
  Q_INVOKABLE void setLanguageNum(int num)
  {
    m_settings.setValue("languangeNum", num);
  }

  void applyTranslation()
  {
    static constexpr const char* languageList[2] = {"en", "ru"};
    handleChangeLanguage(languageList[getLanguageNum()]);
  }

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
  QSettings m_settings;
};

int main(int argc, char** argv)
{
  try {
    QCoreApplication::setApplicationName(STRING(APPLICATION_NAME));
    QCoreApplication::setOrganizationName("Useless soft 112233");
    qmlRegisterType<PictureListModel>("localhost.PictureModel", 1, 0, "PictureListModel");
    qmlRegisterType<CollectionManager>("localhost.PictureModel", 1, 0, "PictureCollection");
    qmlRegisterType<PictureProvider>("localhost.PictureModel", 1, 0, "PictureProvider");

    qmlRegisterType<DirectoryTreeModel>("localhost.DirectoryTreeModel", 1, 0, "DirectoryTreeModel");
    qmlRegisterType<DirectoryValidator>("localhost.Utility", 1, 0, "DirectoryValidator");
    qmlRegisterType<StrictIntValidator>("localhost.Utility", 1, 0, "StrictIntValidator");

    QGuiApplication app(argc, argv);
    LoseFocusDetector::s_singletonInstance = LoseFocusDetector::construct(&app); 
    QQmlApplicationEngine engine;
    Settings settings(&engine);
    qmlRegisterSingletonInstance("localhost.Utility", 1, 0, "LoseFocusDetector", LoseFocusDetector::s_singletonInstance);
    qmlRegisterSingletonInstance("localhost.Utility", 1, 0, "Settings", &settings);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     [](QObject* loadedObj)
                     {
                       if (! loadedObj) {
                        throw std::runtime_error("Failed to load Qml");
                       }
                     });
    engine.load(QUrl("qrc:/mainqml/main.qml"));
    settings.applyTranslation();

    return app.exec();
  } catch (std::exception& e) {
    qDebug()<<e.what();
  }
}

#include "main.moc"
