#pragma once

#include <QObject>
#include <QProcess>

class WaydroidLauncher : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;

    Q_INVOKABLE void launchApp(const QString &pkg) {
        QProcess::startDetached(QStringLiteral("waydroid"), {QStringLiteral("app"), QStringLiteral("launch"), pkg});
    }

    Q_INVOKABLE void showFullUI() {
        QProcess::startDetached(QStringLiteral("waydroid"), {QStringLiteral("show-full-ui")});
    }
};
