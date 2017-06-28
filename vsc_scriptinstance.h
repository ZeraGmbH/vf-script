#ifndef VEINSCRIPT_SCRIPTINSTANCE_H
#define VEINSCRIPT_SCRIPTINSTANCE_H

#include <QObject>
#include <QJsonDocument>

QT_BEGIN_NAMESPACE
class QQuickItem;
QT_END_NAMESPACE

namespace VeinScript
{
  class ScriptInstance : public QObject
  {
    Q_OBJECT
    Q_PROPERTY(bool scriptActive READ scriptActive WRITE setScriptActive NOTIFY scriptActiveChanged)
  public:
    explicit ScriptInstance(QQuickItem *t_scriptObject, const QJsonDocument &t_scriptData, QObject *t_parent = 0);
    ~ScriptInstance();
    QString getScriptName() const;
    bool scriptActive() const;
    bool isValid() const;
    QJsonDocument getScriptData() const;

  signals:
    void scriptActiveChanged(ScriptInstance *t_instance, bool t_scriptActive);
    /**
     * @brief scriptMessageReceived transfers messages to the script
     * @param t_message JSON in QString format
     */
    void scriptMessageReceived(const QString &t_message);

  public slots:
    void setScriptActive(bool t_active);

  private:
    QQuickItem *m_scriptObject=0;
    const QJsonDocument m_scriptData;
    bool m_scriptActive=false;
    const bool m_scriptValid;
  };
} // namespace VeinScript

#endif // VEINSCRIPT_SCRIPTINSTANCE_H
