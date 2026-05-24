#pragma once

#include <QColor>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>

// One task that orbits the cursor while it is alive, exposes a status +
// progress, and renders in the focus-mode task list. Optionally backed by
// a QProcess subprocess; lines on the process's stdout matching
// `PROGRESS: <fraction>` drive `progressFraction`, and the last
// non-progress stdout line is exposed as `stdoutTail` for the row UI.
//
// Tasks are owned by CompanionState; QML accesses them via a
// QAbstractListModel and never instantiates a TaskItem directly.
class TaskItem : public QObject {
    Q_OBJECT
    Q_PROPERTY(QColor color READ color CONSTANT)
    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
    Q_PROPERTY(qreal elapsedSeconds READ elapsedSeconds NOTIFY elapsedSecondsChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(qreal progressFraction READ progressFraction NOTIFY progressFractionChanged)
    Q_PROPERTY(QString stdoutTail READ stdoutTail NOTIFY stdoutTailChanged)
    Q_PROPERTY(bool hasAttachedProcess READ hasAttachedProcess NOTIFY hasAttachedProcessChanged)
    Q_PROPERTY(bool isTerminal READ isTerminal NOTIFY statusChanged)

public:
    enum Status {
        Pending,
        Running,
        Done,
        Failed,
        Cancelled,
    };
    Q_ENUM(Status)

    explicit TaskItem(QColor color,
                      QString title = {},
                      QString description = {},
                      QObject* parent = nullptr);
    ~TaskItem() override;

    QColor color() const { return colorValue; }
    QString title() const { return titleValue; }
    QString description() const { return descriptionValue; }
    qreal elapsedSeconds() const { return elapsedSecondsValue; }
    Status status() const { return statusValue; }
    qreal progressFraction() const { return progressFractionValue; }
    QString stdoutTail() const { return stdoutTailValue; }
    bool hasAttachedProcess() const { return processInstance != nullptr; }

    bool isTerminal() const {
        return statusValue == Done || statusValue == Failed || statusValue == Cancelled;
    }

    void advance(qreal deltaSeconds);
    void setStatus(Status newStatus);
    void setProgressFraction(qreal newProgressFraction);
    void setDescription(const QString& newDescription);

    // Attach + start a subprocess. The task transitions to Running on
    // successful start, Done on clean exit, Failed on non-zero exit or
    // launch error. Force-close (requestKill) flips to Cancelled.
    void attachProcess(const QString& program, const QStringList& arguments);
    void requestKill();

signals:
    void elapsedSecondsChanged();
    void statusChanged();
    void progressFractionChanged();
    void descriptionChanged();
    void stdoutTailChanged();
    void hasAttachedProcessChanged();

private slots:
    void handleProcessStdoutReadyRead();
    void handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void handleProcessErrorOccurred(QProcess::ProcessError error);

private:
    QColor colorValue;
    QString titleValue;
    QString descriptionValue;
    qreal elapsedSecondsValue = 0.0;
    Status statusValue = Running;
    qreal progressFractionValue = 0.0;
    QString stdoutTailValue;

    QProcess* processInstance = nullptr;
    QString stdoutLineBuffer;  // accumulates partial lines

    void setStdoutTail(const QString& newStdoutTail);
    void consumeStdoutLine(const QString& line);
};
