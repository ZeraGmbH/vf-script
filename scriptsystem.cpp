#include "scriptsystem.h"


namespace VeinScript {

  ScriptSystem::ScriptSystem(QObject *t_parent) : VeinEvent::EventSystem(t_parent)
  {
  }

  bool ScriptSystem::processEvent(QEvent *t_event)
  {
    return false;
  }

} // namespace VeinScript
