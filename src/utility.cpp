#include "utility.hpp"

#include <QDir>

QValidator::State DirectoryValidator::validate(QString& input, int& pos) const {
  QDir qdir(input);
  if (qdir.exists()) {
    return State::Acceptable;
  }
  return State::Intermediate;
}

StrictIntValidator::StrictIntValidator(int minVal, int maxVal, QObject* parent)
    : QValidator(parent), m_minVal(minVal), m_maxVal(maxVal) {
  m_maxChars = std::log10(m_maxVal) + 1;
}

QValidator::State StrictIntValidator::validate(QString& input, int& pos) const {
  if (input.size() > m_maxChars) {
    return State::Invalid;
  }
  if (input.isEmpty()) {
    return State::Intermediate;
  }
  bool status = true;
  int value = input.toInt(&status);
  if (!status) {
    return State::Invalid;
  }
  if (value < m_minVal || value > m_maxVal) {
    return State::Intermediate;
  }
  return State::Acceptable;
}

int StrictIntValidator::getMinVal() const noexcept {
  return m_minVal;
}

void StrictIntValidator::setMinVal(int val) {
  if (m_minVal != val) {
    m_minVal = val;
    emit minValChanged();
  }
}

int StrictIntValidator::getMaxVal() const noexcept {
  return m_maxVal;
}

void StrictIntValidator::setMaxVal(int val) {
  if (m_maxVal != val) {
    m_maxVal = val;
    m_maxChars = std::log10(m_maxVal) + 1;
    emit maxValChanged();
  }
}

LoseFocusDetector* LoseFocusDetector::construct(QGuiApplication* app) {
  static LoseFocusDetector detector;
  app->installEventFilter(&detector);
  return &detector;
}

bool LoseFocusDetector::eventFilter(QObject* obj, QEvent* event) {
  if (event->type() == QEvent::MouseButtonRelease) {
    if (m_focusedObject == nullptr) {
      return false;
    }
    if (!m_focusedObject->hasFocus()) {
      setFocusedObject(nullptr);
      return false;
    }
    QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
    QPointF coords = m_focusedObject->mapToGlobal(0, 0);
    QRectF bounds =
        QRectF(coords.x(), coords.y(), m_focusedObject->bindableWidth().value(),
               m_focusedObject->bindableHeight().value());
    if (!bounds.contains(mouseEvent->globalPosition())) {
      m_focusedObject->setFocus(false, Qt::FocusReason::MouseFocusReason);
      setFocusedObject(nullptr);
    }
  }
  return false;
}

QObject* LoseFocusDetector::getFocusedObject() const {
  return m_focusedObject;
}

void LoseFocusDetector::setFocusedObject(QObject* obj) {
  if (m_focusedObject != obj) {
    if (obj != nullptr) {
      m_focusedObject = qobject_cast<QQuickItem*>(obj);
      Q_ASSERT(m_focusedObject);
    } else {
      m_focusedObject = nullptr;
    }
    emit focusedObjectChanged();
  }
}
