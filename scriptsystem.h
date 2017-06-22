#ifndef VEINSCRIPT_SCRIPTSYSTEM_H
#define VEINSCRIPT_SCRIPTSYSTEM_H

#include <ve_eventsystem.h>
#include "vein-script_global.h"

namespace VeinScript
{
  class ScriptInstance;
  class ScriptSystemPrivate;
  class VEINSCRIPTSHARED_EXPORT ScriptSystem : public VeinEvent::EventSystem
  {
    Q_OBJECT
  public:
    explicit ScriptSystem(QObject *t_parent=0);
    virtual ~ScriptSystem();

    ScriptInstance *scriptFromJson(QJsonDocument t_jsonDoc);
    QJsonDocument scriptToJson(ScriptInstance *t_instance);
  public slots:
    void initSystem();

    // EventSystem interface
  public:
    bool processEvent(QEvent *t_event) override;


  private:
    ScriptSystemPrivate *m_dPtr=0;
  };
} // namespace VeinScript
#endif // VEINSCRIPT_SCRIPTSYSTEM_H
