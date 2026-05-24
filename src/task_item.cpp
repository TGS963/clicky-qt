#include "task_item.h"

#include <QRegularExpression>

TaskItem::TaskItem(QColor color, QString title, QString description, QObject* parent)
    : QObject(parent),
      colorValue(std::move(color)),
      titleValue(std::move(title)),
      descriptionValue(std::move(description)) {}

TaskItem::~TaskItem() {
    // Send SIGKILL asynchronously and let QProcess's QObject-parent cleanup
    // reap the child. Synchronously waiting up to 500 ms here meant app
    // shutdown stalled N × 500 ms with N still-running tasks.
    if (processInstance && processInstance->state() != QProcess::NotRunning) {
        processInstance->kill();
    }
}

void TaskItem::advance(qreal deltaSeconds) {
    elapsedSecondsValue += deltaSeconds;
    emit tickedAhead();
}

void TaskItem::setStatus(Status newStatus) {
    if (statusValue == newStatus) {
        return;
    }
    statusValue = newStatus;
    emit statusChanged();
}

void TaskItem::setProgressFraction(qreal newProgressFraction) {
    const qreal clamped = qBound(0.0, newProgressFraction, 1.0);
    if (qFuzzyIsNull(progressFractionValue - clamped)) {
        return;
    }
    progressFractionValue = clamped;
    emit progressFractionChanged();
}

void TaskItem::setDescription(const QString& newDescription) {
    if (descriptionValue == newDescription) {
        return;
    }
    descriptionValue = newDescription;
    emit descriptionChanged();
}

void TaskItem::setStdoutTail(const QString& newStdoutTail) {
    if (stdoutTailValue == newStdoutTail) {
        return;
    }
    stdoutTailValue = newStdoutTail;
    emit stdoutTailChanged();
}

void TaskItem::attachProcess(const QString& program, const QStringList& arguments) {
    if (processInstance) {
        return;
    }
    processInstance = new QProcess(this);
    processInstance->setProcessChannelMode(QProcess::MergedChannels);

    connect(processInstance, &QProcess::readyReadStandardOutput,
            this, &TaskItem::handleProcessStdoutReadyRead);
    connect(processInstance,
            static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &TaskItem::handleProcessFinished);
    connect(processInstance, &QProcess::errorOccurred,
            this, &TaskItem::handleProcessErrorOccurred);

    setStatus(Running);
    emit hasAttachedProcessChanged();
    processInstance->start(program, arguments);
}

void TaskItem::requestKill() {
    if (isTerminal()) {
        return;
    }
    if (processInstance && processInstance->state() != QProcess::NotRunning) {
        processInstance->kill();
        // handleProcessFinished will normally flip status; preempt with
        // Cancelled here so the row immediately reflects the user action.
    }
    setStatus(Cancelled);
}

void TaskItem::handleProcessStdoutReadyRead() {
    if (!processInstance) {
        return;
    }
    stdoutLineBuffer += QString::fromUtf8(processInstance->readAllStandardOutput());

    int newlineIndex = stdoutLineBuffer.indexOf('\n');
    while (newlineIndex != -1) {
        const QString line = stdoutLineBuffer.left(newlineIndex);
        stdoutLineBuffer.remove(0, newlineIndex + 1);
        consumeStdoutLine(line);
        newlineIndex = stdoutLineBuffer.indexOf('\n');
    }
}

void TaskItem::consumeStdoutLine(const QString& line) {
    // Function-local static avoids constructing the regex before main().
    static const QRegularExpression progressLinePattern(
        QStringLiteral(R"(^PROGRESS:\s*([01]?\.\d+|0|1)\s*$)"));
    const QRegularExpressionMatch progressMatch = progressLinePattern.match(line);
    if (progressMatch.hasMatch()) {
        setProgressFraction(progressMatch.captured(1).toDouble());
        return;
    }
    if (!line.trimmed().isEmpty()) {
        setStdoutTail(line.trimmed());
        setDescription(line.trimmed());
    }
}

void TaskItem::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    // Drain any final stdout that didn't have a trailing newline.
    if (!stdoutLineBuffer.isEmpty()) {
        consumeStdoutLine(stdoutLineBuffer);
        stdoutLineBuffer.clear();
    }

    if (isTerminal()) {
        // Already Cancelled by requestKill, or otherwise resolved.
        return;
    }
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        setProgressFraction(1.0);
        setStatus(Done);
    } else {
        setStatus(Failed);
    }
}

void TaskItem::handleProcessErrorOccurred(QProcess::ProcessError error) {
    if (isTerminal()) {
        return;
    }
    // Crashed/Timedout/UnknownError typically also surface via
    // handleProcessFinished, but explicit handling here guarantees the task
    // doesn't get stuck in Running if QProcess::finished is somehow missed.
    switch (error) {
    case QProcess::FailedToStart:
        setDescription(QStringLiteral("Failed to start process"));
        setStatus(Failed);
        break;
    case QProcess::Crashed:
        setDescription(QStringLiteral("Process crashed"));
        setStatus(Failed);
        break;
    case QProcess::Timedout:
    case QProcess::ReadError:
    case QProcess::WriteError:
    case QProcess::UnknownError:
        setStatus(Failed);
        break;
    }
}
