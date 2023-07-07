#ifndef VEINSCRIPT_SCRIPTSYSTEM_H
#define VEINSCRIPT_SCRIPTSYSTEM_H

#include <ve_eventsystem.h>
#include "globalIncludes.h"

namespace VeinScript
{
  class ScriptInstance;
  class ScriptSystemPrivate;
  class VFSCRIPT_EXPORT ScriptSystem : public VeinEvent::EventSystem
  {
    Q_OBJECT
  public:
    explicit ScriptSystem(QObject *t_parent=nullptr);
    virtual ~ScriptSystem() {}
    QStringList listScripts();
    bool loadScriptFromFile(const QString &t_fileName, const QString &t_signatureFileName = QString());

    void processEvent(QEvent *t_event) override;

  private:
    ScriptSystemPrivate *m_dPtr=nullptr;
  };
} // namespace VeinScript
#endif // VEINSCRIPT_SCRIPTSYSTEM_H
