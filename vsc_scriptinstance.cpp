#include "vsc_scriptinstance.h"
#include <QQuickItem>
#include <vh_logging.h>
#include <QJsonObject>
#include <QMetaObject>

namespace VeinScript
{
  ScriptInstance::ScriptInstance(QQuickItem *t_scriptObject, const QJsonDocument &t_scriptData, QObject *t_parent) :
    QObject(t_parent),
    m_scriptObject(t_scriptObject),
    m_scriptData(t_scriptData),
    m_scriptValid(t_scriptObject != nullptr && t_scriptData.isNull() == false)
  {
    VF_ASSERT(m_scriptValid == true, "Script is not valid, please check the source");
  }

  ScriptInstance::~ScriptInstance()
  {
    m_scriptObject->deleteLater();
  }

  QString ScriptInstance::getScriptName() const
  {
    return m_scriptData.object().value("scriptName").toString();
  }

  bool ScriptInstance::isValid() const
  {
    return m_scriptValid;
  }

  QJsonDocument ScriptInstance::getScriptData() const
  {
    return m_scriptData;
  }
} // namespace VeinScript
