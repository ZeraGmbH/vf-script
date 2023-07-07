#include "vsc_scriptsystem.h"
#include "vsc_scriptinstance.h"

#include <vh_logging.h>
#include <ve_eventdata.h>
#include <ve_commandevent.h>
#include <vcmp_componentdata.h>
#include <vcmp_entitydata.h>
#include <vcmp_introspectiondata.h>
#include <vcmp_errordata.h>

#include <QFile>
#include <QHash>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQuickItem>
#include <QJsonDocument>
#include <QJsonObject>

namespace VeinScript
{
  class ScriptSystemPrivate
  {
    ScriptSystemPrivate(ScriptSystem *t_qPtr) : m_qPtr(t_qPtr), m_component(&m_engine)
    {
    }
    ~ScriptSystemPrivate() {}

    void initOnce()
    {
      Q_ASSERT(m_initDone == false);
      if(m_initDone == false)
      {
        VeinComponent::EntityData *systemData = new VeinComponent::EntityData();
        systemData->setCommand(VeinComponent::EntityData::Command::ECMD_ADD);
        systemData->setEntityId(s_entityId);

        VeinEvent::CommandEvent *systemEvent = new VeinEvent::CommandEvent(VeinEvent::CommandEvent::EventSubtype::NOTIFICATION, systemData);

        emit m_qPtr->sigSendEvent(systemEvent);

        VeinComponent::ComponentData *introspectionData = nullptr;

        QHash<QString, QVariant> componentData;
        componentData.insert(s_entityNameComponentName, s_entityName);
        ///@todo load from persistent data?
        componentData.insert(s_scriptsComponentName, QVariant());
        componentData.insert(s_addScriptComponentName, QVariant());

        for(const QString &componentName : componentData.keys())
        {
          introspectionData = new VeinComponent::ComponentData();
          introspectionData->setEntityId(s_entityId);
          introspectionData->setCommand(VeinComponent::ComponentData::Command::CCMD_ADD);
          introspectionData->setComponentName(componentName);
          introspectionData->setNewValue(componentData.value(componentName));
          introspectionData->setEventOrigin(VeinEvent::EventData::EventOrigin::EO_LOCAL);
          introspectionData->setEventTarget(VeinEvent::EventData::EventTarget::ET_ALL);

          systemEvent = new VeinEvent::CommandEvent(VeinEvent::CommandEvent::EventSubtype::NOTIFICATION, introspectionData);
          emit m_qPtr->sigSendEvent(systemEvent);
        }

        m_initDone = true;
      }
    }

    void sendError(const QString &t_errorString, VeinEvent::EventData *t_data)
    {
      VeinComponent::ErrorData *errData = new VeinComponent::ErrorData();

      errData->setEntityId(t_data->entityId());
      errData->setOriginalData(t_data);
      errData->setEventOrigin(VeinEvent::EventData::EventOrigin::EO_LOCAL);
      errData->setEventTarget(t_data->eventTarget());
      errData->setErrorDescription(t_errorString);

      VeinEvent::CommandEvent *cEvent = new VeinEvent::CommandEvent(VeinEvent::CommandEvent::EventSubtype::NOTIFICATION, errData);
      emit m_qPtr->sigSendEvent(cEvent);
    }

    ScriptInstance *scriptFromJson(QJsonDocument t_jsonDoc, VeinComponent::ComponentData *t_errorData = nullptr)
    {
      VF_ASSERT(t_jsonDoc.isEmpty() == false, "Invalid json document");
      VF_ASSERT(t_jsonDoc.isObject() == true, "JSON document has no object");
      QJsonObject jsonObject = t_jsonDoc.object();
      QQuickItem *tmpItem = nullptr;

      QByteArray scriptSignature;
      QByteArray scriptData;
      scriptSignature.append(jsonObject.value(ScriptSystemPrivate::s_scriptJsonSignatureKey).toString());
      scriptSignature = QByteArray::fromBase64(scriptSignature);
      /// @todo check script signature with QCA openssl dgst verify

      scriptData = jsonObject.value(ScriptSystemPrivate::s_scriptJsonDataKey).toString().toUtf8();
      m_component.setData(scriptData, QUrl(jsonObject.value(ScriptSystemPrivate::s_scriptJsonNameKey).toString()));
      tmpItem = qobject_cast<QQuickItem *>(m_component.beginCreate(m_engine.rootContext()));
      m_component.completeCreate();
      if(m_component.errors().isEmpty() == false && t_errorData != nullptr)
      {
        sendError(m_component.errorString(), t_errorData);
      }
      ScriptInstance * tmpScript = new ScriptInstance(tmpItem, t_jsonDoc, m_qPtr);
      return tmpScript;
    }

    void scriptInserted(const QString &t_name, ScriptInstance *t_instance)
    {
      QStringList scriptListEntityValue;
      if(m_scriptHash.contains(t_name))
      {
        //remove the old script
        m_scriptHash.value(t_name)->deleteLater();
      }

      m_scriptHash.insert(t_name, t_instance);


      for(const auto tmpScript : m_scriptHash.values())
      {
        scriptListEntityValue.append(QString::fromUtf8(tmpScript->getScriptData().toJson(QJsonDocument::Compact)));
      }

      if(scriptListEntityValue.isEmpty() == false)
      {
        VeinComponent::ComponentData *scriptListCompData = new VeinComponent::ComponentData();
        scriptListCompData->setEntityId(s_entityId);
        scriptListCompData->setCommand(VeinComponent::ComponentData::Command::CCMD_SET);
        scriptListCompData->setComponentName(s_scriptsComponentName);
        scriptListCompData->setNewValue(QVariant(scriptListEntityValue));
        scriptListCompData->setEventOrigin(VeinEvent::EventData::EventOrigin::EO_LOCAL);
        scriptListCompData->setEventTarget(VeinEvent::EventData::EventTarget::ET_ALL);

        VeinEvent::CommandEvent *scriptListEvent = new VeinEvent::CommandEvent(VeinEvent::CommandEvent::EventSubtype::NOTIFICATION, scriptListCompData);
        m_qPtr->sigSendEvent(scriptListEvent);
      }
    }

    bool m_initDone=false;

    static constexpr int s_entityId = 1;
    //entity name
    static const QLatin1String s_entityName;
    //component names
    static const QLatin1String s_entityNameComponentName;
    static const QLatin1String s_scriptsComponentName;
    static const QLatin1String s_addScriptComponentName;
    //script json keys
    static const QLatin1String s_scriptJsonNameKey;
    static const QLatin1String s_scriptJsonDataKey;
    static const QLatin1String s_scriptJsonSignatureKey;

    ScriptSystem *m_qPtr;
    QQmlComponent m_component;
    QQmlEngine m_engine;
    QHash<QString, ScriptInstance *> m_scriptHash;
    friend class ScriptSystem;
  };

  //entity name
  const QLatin1String ScriptSystemPrivate::s_entityName = QLatin1String("_ScriptSystem");
  //component names
  const QLatin1String ScriptSystemPrivate::s_entityNameComponentName = QLatin1String("EntityName");
  const QLatin1String ScriptSystemPrivate::s_scriptsComponentName = QLatin1String("Scripts");
  const QLatin1String ScriptSystemPrivate::s_addScriptComponentName = QLatin1String("addScript()");
  //script json keys
  const QLatin1String ScriptSystemPrivate::s_scriptJsonNameKey = QLatin1String("scriptName");
  const QLatin1String ScriptSystemPrivate::s_scriptJsonDataKey = QLatin1String("scriptData");
  const QLatin1String ScriptSystemPrivate::s_scriptJsonSignatureKey = QLatin1String("scriptSignature");


  ScriptSystem::ScriptSystem(QObject *t_parent) : VeinEvent::EventSystem(t_parent), m_dPtr(new ScriptSystemPrivate(this))
  {
    connect(this, &ScriptSystem::sigAttached, [this](){ m_dPtr->initOnce(); });
  }

  QStringList ScriptSystem::listScripts()
  {
    QStringList retVal;
    for(const QString &scriptName : m_dPtr->m_scriptHash.keys())
    {
      retVal.append(scriptName);
    }

    return retVal;
  }

  bool ScriptSystem::loadScriptFromFile(const QString &t_fileName, const QString &t_signatureFileName)
  {
    bool retVal = false;

    const bool signatureRequired = t_signatureFileName.isEmpty() == false && QFile::exists(t_signatureFileName);
    QByteArray scriptSignature;

    if(signatureRequired)
    {
      QFile signatureFile(t_signatureFileName);
      signatureFile.open(QFile::ReadOnly);
      scriptSignature = signatureFile.readAll();
      signatureFile.close();
      scriptSignature = QByteArray::fromBase64(scriptSignature);
    }


    if(QFile::exists(t_fileName))
    {
      QFile scriptFile(t_fileName);
      QByteArray scriptData;
      QJsonDocument jDoc;
      QJsonObject jObj;

      scriptFile.open(QFile::ReadOnly | QFile::Text);
      scriptData = scriptFile.readAll();
      scriptFile.close();


      jObj.insert(ScriptSystemPrivate::s_scriptJsonNameKey, t_fileName);
      jObj.insert(ScriptSystemPrivate::s_scriptJsonDataKey, QString::fromUtf8(scriptData));
      jObj.insert(ScriptSystemPrivate::s_scriptJsonSignatureKey, QString::fromUtf8(scriptSignature));
      jDoc.setObject(jObj);

      ScriptInstance *tmpInstance = m_dPtr->scriptFromJson(jDoc);
      if(tmpInstance != nullptr && tmpInstance->isValid())
      {
        m_dPtr->scriptInserted(t_fileName, tmpInstance);
        retVal = true;
      }
    }

    return retVal;
  }

  void ScriptSystem::processEvent(QEvent *t_event)
  {
    if(t_event->type() == VeinEvent::CommandEvent::eventType())
    {
      bool validated=false;
      VeinEvent::CommandEvent *cEvent = nullptr;
      cEvent = static_cast<VeinEvent::CommandEvent *>(t_event);
      Q_ASSERT(cEvent != nullptr);

      if(cEvent->eventSubtype() != VeinEvent::CommandEvent::EventSubtype::NOTIFICATION && //we do not need to process notifications
         cEvent->eventData()->type() == VeinComponent::ComponentData::dataType())
      {
        VeinComponent::ComponentData *cData=nullptr;
        cData = static_cast<VeinComponent::ComponentData *>(cEvent->eventData());
        Q_ASSERT(cData!=nullptr);

        if(cData->eventCommand() == VeinComponent::ComponentData::Command::CCMD_SET &&
           cData->entityId() == ScriptSystemPrivate::s_entityId)
        {
          if(cData->componentName() == ScriptSystemPrivate::s_addScriptComponentName) ///@todo change addScript() into an RPC
          {
            QJsonParseError jsonScriptError;
            QJsonDocument tmpScript = QJsonDocument::fromJson(cData->newValue().toString().toUtf8(), &jsonScriptError);

            if(jsonScriptError.error == QJsonParseError::NoError)
            {
              ScriptInstance *tmpInstance = m_dPtr->scriptFromJson(tmpScript, cData);
              if(tmpInstance != nullptr && tmpInstance->isValid())
              {
                m_dPtr->scriptInserted(tmpInstance->getScriptName(), tmpInstance);
              }
            }
            else
            {
              //send error message
              m_dPtr->sendError(jsonScriptError.errorString(), cData);
            }
          }
        }
      }

      if(validated == true)
      {
        ///@todo @bug remove inconsistent behavior by sending a new event instead of rewriting the current event
        cEvent->setEventSubtype(VeinEvent::CommandEvent::EventSubtype::NOTIFICATION);
        cEvent->eventData()->setEventOrigin(VeinEvent::EventData::EventOrigin::EO_LOCAL);
        cEvent->eventData()->setEventTarget(VeinEvent::EventData::EventTarget::ET_ALL);
      }
    }
  }

} // namespace VeinScript
