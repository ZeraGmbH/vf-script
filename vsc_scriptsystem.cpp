#include "vsc_scriptsystem.h"
#include "vsc_scriptinstance.h"

#include <vh_logging.h>
#include <ve_eventdata.h>
#include <ve_commandevent.h>
#include <vcmp_componentdata.h>
#include <vcmp_entitydata.h>
#include <vcmp_introspectiondata.h>
#include <vcmp_errordata.h>

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

    void initOnce()
    {
      if(m_initDone == false)
      {
        VeinComponent::EntityData *systemData = new VeinComponent::EntityData();
        systemData->setCommand(VeinComponent::EntityData::Command::ECMD_ADD);
        systemData->setEntityId(m_entityId);

        VeinEvent::CommandEvent *systemEvent = new VeinEvent::CommandEvent(VeinEvent::CommandEvent::EventSubtype::NOTIFICATION, systemData);

        emit m_qPtr->sigSendEvent(systemEvent);

        VeinComponent::ComponentData *introspectionData = nullptr;

        //component name
        introspectionData = new VeinComponent::ComponentData();
        introspectionData->setEntityId(m_entityId);
        introspectionData->setCommand(VeinComponent::ComponentData::Command::CCMD_ADD);
        introspectionData->setComponentName(s_entityNameComponentName);
        introspectionData->setNewValue(s_entityName);
        introspectionData->setEventOrigin(VeinEvent::EventData::EventOrigin::EO_LOCAL);
        introspectionData->setEventTarget(VeinEvent::EventData::EventTarget::ET_ALL);

        systemEvent = new VeinEvent::CommandEvent(VeinEvent::CommandEvent::EventSubtype::NOTIFICATION, introspectionData);
        emit m_qPtr->sigSendEvent(systemEvent);

        //scripts
        introspectionData = new VeinComponent::ComponentData();
        introspectionData->setEntityId(m_entityId);
        introspectionData->setCommand(VeinComponent::ComponentData::Command::CCMD_ADD);
        introspectionData->setComponentName(s_scriptsComponentName);
        introspectionData->setNewValue(QVariant());
        introspectionData->setEventOrigin(VeinEvent::EventData::EventOrigin::EO_LOCAL);
        introspectionData->setEventTarget(VeinEvent::EventData::EventTarget::ET_ALL);

        systemEvent = new VeinEvent::CommandEvent(VeinEvent::CommandEvent::EventSubtype::NOTIFICATION, introspectionData);
        emit m_qPtr->sigSendEvent(systemEvent);
        //addScript function
        introspectionData = new VeinComponent::ComponentData();
        introspectionData->setEntityId(m_entityId);
        introspectionData->setCommand(VeinComponent::ComponentData::Command::CCMD_ADD);
        introspectionData->setComponentName(s_addScriptComponentName);
        introspectionData->setNewValue(QVariant());
        introspectionData->setEventOrigin(VeinEvent::EventData::EventOrigin::EO_LOCAL);
        introspectionData->setEventTarget(VeinEvent::EventData::EventTarget::ET_ALL);

        systemEvent = new VeinEvent::CommandEvent(VeinEvent::CommandEvent::EventSubtype::NOTIFICATION, introspectionData);
        emit m_qPtr->sigSendEvent(systemEvent);

        introspectionData = new VeinComponent::ComponentData();
        introspectionData->setEntityId(m_entityId);
        introspectionData->setCommand(VeinComponent::ComponentData::Command::CCMD_ADD);
        introspectionData->setComponentName(s_scriptMessageComponentName);
        introspectionData->setNewValue(QVariant());
        introspectionData->setEventOrigin(VeinEvent::EventData::EventOrigin::EO_LOCAL);
        introspectionData->setEventTarget(VeinEvent::EventData::EventTarget::ET_ALL);

        systemEvent = new VeinEvent::CommandEvent(VeinEvent::CommandEvent::EventSubtype::NOTIFICATION, introspectionData);
        emit m_qPtr->sigSendEvent(systemEvent);


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

    QJsonDocument scriptToJson(ScriptInstance *t_instance)
    {
      return t_instance->getScriptData();
    }

    ScriptInstance *scriptFromJson(QJsonDocument t_jsonDoc, VeinComponent::ComponentData *t_errorData)
    {
      VF_ASSERT(t_jsonDoc.isEmpty() == false, "Invalid json document");
      VF_ASSERT(t_jsonDoc.isObject() == true, "JSON document has no object");
      QJsonObject jsonObject = t_jsonDoc.object();

      QByteArray scriptSignature;
      scriptSignature.append(jsonObject.value(ScriptSystemPrivate::s_scriptJsonSignatureKey).toString());
      scriptSignature = QByteArray::fromBase64(scriptSignature);
      //@todo: check script signature with QCA openssl dgst verify

      QQuickItem *tmpItem = nullptr;
      m_component.setData(jsonObject.value(ScriptSystemPrivate::s_scriptJsonDataKey).toString().toUtf8(), QUrl(jsonObject.value(ScriptSystemPrivate::s_scriptJsonNameKey).toString()));
      tmpItem = qobject_cast<QQuickItem *>(m_component.beginCreate(m_engine.rootContext()));
      m_component.completeCreate();
      if(m_component.errors().isEmpty() == false)
      {
        sendError(m_component.errorString(), t_errorData);
      }
      ScriptInstance * tmpScript = new ScriptInstance(tmpItem, t_jsonDoc, m_qPtr);
      return tmpScript;
    }

    const int m_entityId = 1;
    bool m_initDone=false;

    //entity name
    static constexpr char const *s_entityName = "_ScriptSystem";
    //component names
    static constexpr char const *s_entityNameComponentName = "EntityName";
    static constexpr char const *s_scriptsComponentName = "Scripts";
    static constexpr char const *s_addScriptComponentName = "addScript";
    static constexpr char const *s_scriptMessageComponentName = "notifyScript";
    //script json keys
    static constexpr char const *s_scriptJsonNameKey = "scriptName";
    static constexpr char const *s_scriptJsonDataKey = "scriptData";
    static constexpr char const *s_scriptJsonSignatureKey = "scriptSignature";
    //message json keys
    static constexpr char const *s_messageJsonScriptNameKey = "scriptName";
    static constexpr char const *s_messageJsonDataKey = "messageData";

    ScriptSystem *m_qPtr;
    QQmlComponent m_component;
    QQmlEngine m_engine;
    QHash<QString, ScriptInstance *> m_scriptHash;
    friend class ScriptSystem;
  };


  ScriptSystem::ScriptSystem(QObject *t_parent) : VeinEvent::EventSystem(t_parent), m_dPtr(new ScriptSystemPrivate(this))
  {
#if 0
    connect(this, &ScriptSystem::sigSendScriptMessage, this, &ScriptSystem::sendScriptMessage);
#endif
  }

  void ScriptSystem::initSystem()
  {
    m_dPtr->initOnce();
  }

  bool ScriptSystem::processEvent(QEvent *t_event)
  {
    bool retVal = false;

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
           cData->entityId() == m_dPtr->m_entityId)
        {
          if(cData->componentName() == ScriptSystemPrivate::s_addScriptComponentName)
          {
            QJsonParseError *jsonScriptError = nullptr;
            QJsonDocument tmpScript = QJsonDocument::fromJson(cData->newValue().toString().toUtf8(), jsonScriptError);

            if(jsonScriptError->error == QJsonParseError::NoError)
            {
              ScriptInstance *tmpInstance = m_dPtr->scriptFromJson(tmpScript, cData);
              m_dPtr->m_scriptHash.insert(tmpInstance->getScriptName(), tmpInstance);
            }
            else
            {
              //send error message
              m_dPtr->sendError(jsonScriptError->errorString(), cData);
            }
          }
          else if(cData->componentName() == ScriptSystemPrivate::s_scriptMessageComponentName)
          {
            QJsonParseError *jsonMessageError = nullptr;
            const QString tmpMessage = cData->newValue().toString();
            QJsonDocument messageParsed = QJsonDocument::fromJson(tmpMessage.toUtf8(), jsonMessageError);
            if(jsonMessageError->error == QJsonParseError::NoError)
            {
              QJsonObject messageObject = messageParsed.object();
              const QString scriptReceiverName = messageObject.value(ScriptSystemPrivate::s_messageJsonScriptNameKey).toString();
              ScriptInstance *messageScript = m_dPtr->m_scriptHash.value(scriptReceiverName, nullptr);

              if(messageScript != nullptr)
              {
                messageScript->scriptMessageReceived(tmpMessage);
              }
              else
              {
                m_dPtr->sendError(QString("Unable to transmit message to script, no script found with name: %1").arg(scriptReceiverName), cData);
              }
            }
            else
            {
              //send error message
              m_dPtr->sendError(jsonMessageError->errorString(), cData);
            }
          }
        }
      }

      if(validated == true)
      {
        retVal = true;
        cEvent->setEventSubtype(VeinEvent::CommandEvent::EventSubtype::NOTIFICATION);
        cEvent->eventData()->setEventOrigin(VeinEvent::EventData::EventOrigin::EO_LOCAL);
        cEvent->eventData()->setEventTarget(VeinEvent::EventData::EventTarget::ET_ALL);
      }
    }
    return retVal;
  }

#if 0
  void ScriptSystem::sendScriptMessage(const QString &t_message)
  {
    QJsonParseError *jsonMessageError = nullptr;
    QJsonDocument tmpMessage = QJsonDocument::fromJson(t_message.toUtf8(), jsonMessageError);

    if(jsonMessageError->error == QJsonParseError::NoError &&
       tmpMessage.isObject() == true)
    {
      VeinComponent::ComponentData *errData = new VeinComponent::ComponentData();
      errData->setCommand(VeinComponent::ComponentData::Command::CCMD_SET);
      errData->setComponentName(ScriptSystemPrivate::s_scriptMessageComponentName);
      errData->setEventOrigin(VeinEvent::EventData::EventOrigin::EO_LOCAL);
      errData->setEventTarget(VeinEvent::EventData::EventTarget::ET_ALL);
      errData->setNewValue(t_message);

      VeinEvent::CommandEvent *cEvent = new VeinEvent::CommandEvent(VeinEvent::CommandEvent::EventSubtype::NOTIFICATION, errData);
      emit sigSendEvent(cEvent);
    }
    else
    {
      //send error message?
    }
  }
#endif

} // namespace VeinScript
