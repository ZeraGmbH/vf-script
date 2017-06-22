#include "scriptsystem.h"
#include "scriptinstance.h"

#include <vh_logging.h>
#include <ve_eventdata.h>
#include <ve_commandevent.h>
#include <vcmp_componentdata.h>
#include <vcmp_entitydata.h>
#include <vcmp_introspectiondata.h>
#include <vcmp_errordata.h>

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
        systemEvent=0;
        systemData=0;

        VeinComponent::ComponentData *introspectionData=0;

        //component name
        introspectionData = new VeinComponent::ComponentData();
        introspectionData->setEntityId(m_entityId);
        introspectionData->setCommand(VeinComponent::ComponentData::Command::CCMD_ADD);
        introspectionData->setComponentName(m_entitynNameComponentName);
        introspectionData->setNewValue(m_entityName);
        introspectionData->setEventOrigin(VeinEvent::EventData::EventOrigin::EO_LOCAL);
        introspectionData->setEventTarget(VeinEvent::EventData::EventTarget::ET_ALL);

        systemEvent = new VeinEvent::CommandEvent(VeinEvent::CommandEvent::EventSubtype::NOTIFICATION, introspectionData);
        emit m_qPtr->sigSendEvent(systemEvent);
        systemEvent = 0;
        //scripts
        introspectionData = new VeinComponent::ComponentData();
        introspectionData->setEntityId(m_entityId);
        introspectionData->setCommand(VeinComponent::ComponentData::Command::CCMD_ADD);
        introspectionData->setComponentName(m_scriptsComponentName);
        introspectionData->setNewValue(QVariant());
        introspectionData->setEventOrigin(VeinEvent::EventData::EventOrigin::EO_LOCAL);
        introspectionData->setEventTarget(VeinEvent::EventData::EventTarget::ET_ALL);

        systemEvent = new VeinEvent::CommandEvent(VeinEvent::CommandEvent::EventSubtype::NOTIFICATION, introspectionData);
        emit m_qPtr->sigSendEvent(systemEvent);
        //addScript function
        introspectionData = new VeinComponent::ComponentData();
        introspectionData->setEntityId(m_entityId);
        introspectionData->setCommand(VeinComponent::ComponentData::Command::CCMD_ADD);
        introspectionData->setComponentName(m_addScriptComponentName);
        introspectionData->setNewValue(m_addScriptPlaceholder);
        introspectionData->setEventOrigin(VeinEvent::EventData::EventOrigin::EO_LOCAL);
        introspectionData->setEventTarget(VeinEvent::EventData::EventTarget::ET_ALL);

        systemEvent = new VeinEvent::CommandEvent(VeinEvent::CommandEvent::EventSubtype::NOTIFICATION, introspectionData);
        emit m_qPtr->sigSendEvent(systemEvent);

        introspectionData = new VeinComponent::ComponentData();
        introspectionData->setEntityId(m_entityId);
        introspectionData->setCommand(VeinComponent::ComponentData::Command::CCMD_ADD);
        introspectionData->setComponentName(m_scriptMessageComponentName);
        introspectionData->setNewValue(QVariant(m_scriptMessagePlaceholder));
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

    const int m_entityId = 1;
    bool m_initDone=false;

    const QString m_entityName = "_ScriptSystem";
    const QString m_entitynNameComponentName = "EntityName";
    const QString m_scriptsComponentName = "Scripts";
    const QString m_addScriptComponentName = "addScript";
    const QString m_addScriptPlaceholder = "send scripts to this component";
    const QString m_scriptMessageComponentName = "scriptMessage";
    const QString m_scriptMessagePlaceholder = "send script messages to this component";



    ScriptSystem *m_qPtr;
    QQmlComponent m_component;
    QQmlEngine m_engine;
    QVector<ScriptInstance *> m_scriptVector;
    friend class ScriptSystem;
  };

  ScriptSystem::ScriptSystem(QObject *t_parent) : VeinEvent::EventSystem(t_parent), m_dPtr(new ScriptSystemPrivate(this))
  {
  }

  ScriptInstance *ScriptSystem::scriptFromJson(QJsonDocument t_jsonDoc)
  {
    VF_ASSERT(t_jsonDoc.isEmpty() == false, "Invalid json document");
    VF_ASSERT(t_jsonDoc.isObject() == true, "JSON document has no object");
    QJsonObject jsonObject = t_jsonDoc.object();

    QByteArray scriptSignature;
    scriptSignature.append(jsonObject.value("sha256signature").toString());
    scriptSignature = QByteArray::fromBase64(scriptSignature);
    //@todo: check script signature with QCA openssl dgst verify

    QQuickItem *tmpItem = nullptr;
    m_dPtr->m_component.setData(jsonObject.value("scriptData").toString().toUtf8(), QUrl(jsonObject.value("scriptName").toString()));
    tmpItem = qobject_cast<QQuickItem *>(m_dPtr->m_component.beginCreate(m_dPtr->m_engine.rootContext()));
    m_dPtr->m_component.completeCreate();
    ScriptInstance * tmpScript = new ScriptInstance(tmpItem, t_jsonDoc, this);
    m_dPtr->m_scriptVector.append(tmpScript);
    return tmpScript;
  }

  QJsonDocument ScriptSystem::scriptToJson(ScriptInstance *t_instance)
  {
    return t_instance->getScriptData();
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
      if(cEvent != nullptr &&
         cEvent->eventSubtype() != VeinEvent::CommandEvent::EventSubtype::NOTIFICATION && //we do not need to process notifications
         cEvent->eventData()->type() == VeinComponent::ComponentData::dataType())
      {
        VeinComponent::ComponentData *cData=nullptr;
        cData = static_cast<VeinComponent::ComponentData *>(cEvent->eventData());
        Q_ASSERT(cData!=nullptr);

        //validate fetch requests
        if(cData->eventCommand() == VeinComponent::ComponentData::Command::CCMD_FETCH) /// @todo maybe add roles/views later
        {
          validated = true;
        }
        else if(cData->eventCommand() == VeinComponent::ComponentData::Command::CCMD_SET &&
                cData->entityId() == m_dPtr->m_entityId)
        {
          if(cData->componentName() == m_dPtr->m_addScriptComponentName)
          {
            QJsonParseError *jsonScriptError = nullptr;
            QJsonDocument tmpScript = QJsonDocument::fromJson(cData->newValue().toString().toUtf8(), jsonScriptError);
            if(jsonScriptError->error == QJsonParseError::NoError)
            {

            }
            else
            {
              //send error message
              m_dPtr->sendError(jsonScriptError->errorString(), cData);
            }
          }
          else if(cData->componentName() == m_dPtr->m_scriptMessageComponentName)
          {
            QJsonParseError *jsonMessageError = nullptr;
            QJsonDocument tmpMessage = QJsonDocument::fromJson(cData->newValue().toString().toUtf8(), jsonMessageError);
            if(jsonMessageError->error == QJsonParseError::NoError)
            {

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
        cEvent->eventData()->setEventOrigin(VeinEvent::EventData::EventOrigin::EO_LOCAL); //the validated answer is authored from the system that runs the validator (aka. this system)
        cEvent->eventData()->setEventTarget(VeinEvent::EventData::EventTarget::ET_ALL); //inform all users (may or may not result in network messages)
      }
    }
    return retVal;
  }
} // namespace VeinScript
