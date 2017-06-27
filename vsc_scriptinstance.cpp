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
    m_scriptValid(t_scriptObject != nullptr)
  {
    VF_ASSERT(t_scriptObject != nullptr, "Invalid script object");
    VF_ASSERT(t_scriptData.isNull() == false, "Invalid script data");
  }

  ScriptInstance::~ScriptInstance()
  {
    m_scriptObject->deleteLater();
  }

  QString ScriptInstance::getScriptName() const
  {
    return m_scriptData.object().value("scriptName").toString();
  }

  bool ScriptInstance::scriptActive() const
  {
    return m_scriptActive;
  }

  QJsonDocument ScriptInstance::getScriptData() const
  {
    return m_scriptData;
  }

#if 0
  QJsonDocument ScriptInstance::getScriptState() const
  {
    const QMetaObject* mObject = m_scriptObject->metaObject();
    QJsonDocument jsonDocument;
    QJsonObject jsonObject;
    QJsonValue jsonValue;
    QString valueName;

    for(int i = mObject->propertyOffset(); i < mObject->propertyCount(); ++i)
    {
      const auto tmpName = mObject->property(i).name();
      valueName = tmpName;
      jsonValue = QJsonValue::fromVariant(m_scriptObject->property(tmpName));
      VF_ASSERT(jsonValue.isNull() == false, QStringC(QString("Unsupported type in script: %1 type: %2 value: %3").arg(getScriptName()).arg(QMetaType::typeName(m_scriptObject->property(tmpName).type())).arg(m_scriptObject->property(tmpName).toString())));
      jsonObject.insert(valueName, jsonValue);
    }

    jsonDocument.setObject(jsonObject);
    return jsonDocument;
  }
#endif

  void ScriptInstance::setScriptActive(bool t_active)
  {
    if(t_active != m_scriptActive)
    {
      m_scriptActive = t_active;
      emit scriptActiveChanged(this, m_scriptActive);
    }
  }
} // namespace VeinScript
