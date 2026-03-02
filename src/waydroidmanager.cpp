#include "waydroidmanager.h"
#include "waydroidwindowembedder.h"
#include <QProcessEnvironment>

static void waydroidShellDetached(const QStringList &args)
{
    // Example: waydroid shell wm size 1280x720
    QProcess::startDetached(QStringLiteral("waydroid"),
                            QStringList{QStringLiteral("shell")} + args);
}

QObject *WaydroidManager::createWindowEmbedder(const QString &pkg)
{
    auto *embedder = new WaydroidWindowEmbedder(this);
    embedder->setPackageName(pkg);
    return embedder;
}

void WaydroidManager::ensureLandscape()
{
    if (!m_running) {
        qDebug() << "WaydroidManager::ensureLandscape() - Waydroid not running, skip";
        return;
    }

    // Avoid hammering `waydroid shell` on every click.
    if (m_landscapeApplied) {
        return;
    }

    const QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    const QString sizeSpec = env.value(QStringLiteral("SMART_DASHBOARD_WAYDROID_SIZE"),
                                       QStringLiteral("1280x720"));
    const QString densitySpec = env.value(QStringLiteral("SMART_DASHBOARD_WAYDROID_DENSITY"),
                                          QStringLiteral("240"));
    const QString rotationSpec = env.value(QStringLiteral("SMART_DASHBOARD_WAYDROID_ROTATION"),
                                           QStringLiteral("1"));

    qDebug() << "WaydroidManager::ensureLandscape() - applying landscape profile:"
             << "size=" << sizeSpec
             << "density=" << densitySpec
             << "rotation=" << rotationSpec;

    // 1) Lock rotation (disable accelerometer)
    // Android rotation values: 0=0°, 1=90°, 2=180°, 3=270°
    waydroidShellDetached({QStringLiteral("settings"), QStringLiteral("put"),
                           QStringLiteral("system"), QStringLiteral("accelerometer_rotation"),
                           QStringLiteral("0")});
    waydroidShellDetached({QStringLiteral("settings"), QStringLiteral("put"),
                           QStringLiteral("system"), QStringLiteral("user_rotation"),
                           rotationSpec});
    waydroidShellDetached({QStringLiteral("wm"), QStringLiteral("set-user-rotation"),
                           QStringLiteral("lock"), rotationSpec});

    // 2) Force a landscape size/density (fixes many aspect/layout issues)
    // Reset back to defaults: `waydroid shell wm size reset` and `waydroid shell wm density reset`
    if (!sizeSpec.trimmed().isEmpty()) {
        waydroidShellDetached({QStringLiteral("wm"), QStringLiteral("size"), sizeSpec});
    }
    if (!densitySpec.trimmed().isEmpty()) {
        waydroidShellDetached({QStringLiteral("wm"), QStringLiteral("density"), densitySpec});
    }

    m_landscapeApplied = true;
}
