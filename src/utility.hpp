#pragma  once

#include <QValidator>
#include <QQmlEngine>
#include <QGuiApplication>
#include <QQuickItem>

class DirectoryValidator: public QValidator
{
  Q_OBJECT
public:
  QValidator::State validate(QString &input, int &pos) const override;
};

class StrictIntValidator: public QValidator
{
  Q_OBJECT
public:
  StrictIntValidator(int minVal = 0, int maxVal = 0, QObject* parent = nullptr);
  QValidator::State validate(QString &input, int &pos) const override;
  int getMinVal() const noexcept;
  void setMinVal(int val);
  int getMaxVal() const noexcept;
  void setMaxVal(int val);

  Q_PROPERTY(int minVal READ getMinVal WRITE setMinVal NOTIFY minValChanged);
  Q_PROPERTY(int maxVal READ getMaxVal WRITE setMaxVal NOTIFY maxValChanged);
signals:
  void minValChanged();
  void maxValChanged();

private:
  int m_minVal;
  int m_maxVal;
  int m_maxChars;
};

class LoseFocusDetector: public QObject
{
  Q_OBJECT
  QML_FOREIGN(LoseFocusDetector)
  QML_SINGLETON

public:
  inline static LoseFocusDetector* s_singletonInstance = nullptr;
  static LoseFocusDetector* construct(QGuiApplication* app);
  bool eventFilter(QObject *obj, QEvent *event) override;
  QObject* getFocusedObject() const;
  void setFocusedObject(QObject* obj);
  Q_PROPERTY(QObject* focusedObject READ getFocusedObject WRITE setFocusedObject NOTIFY focusedObjectChanged);

signals:
  void focusedObjectChanged();

private:
  LoseFocusDetector(){}
  QQuickItem* m_focusedObject {nullptr};
};
