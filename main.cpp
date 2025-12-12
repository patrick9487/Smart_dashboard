#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QCoreApplication>
#include <QUrl>
#include <QDir>
#include <QDirIterator>
#include <QDebug>

#include "AppConfig.h"
#include "src/waydroidmanager.h"
#include "src/windowembeditem.h"
#include "src/waylandcompositor.h"
#include "src/surfaceitem.h"

static void dumpQmlResources()
{
    // 列出所有可能的 QML 資源路徑，協助除錯
    qDebug() << "---- 檢查 QML 資源路徑 ----";
    
    QStringList pathsToCheck = {
        ":/qt/qml/SmartDashboard",
        ":/qt/qml/SmartDashboard/qml",
        ":/qml",
        ":/"
    };
    
    for (const QString &basePath : pathsToCheck) {
        QDirIterator it(basePath, QDirIterator::Subdirectories);
        bool found = false;
        while (it.hasNext()) {
            QString path = it.next();
            if (path.contains("DashboardShell", Qt::CaseInsensitive)) {
                qDebug() << "找到 DashboardShell:" << path;
                found = true;
            }
        }
        if (found) {
            qDebug() << "在" << basePath << "找到資源";
        }
    }
    
    // 列出所有 :/ 下的資源
    qDebug() << "---- 所有 :/ 資源 ----";
    QDirIterator allIt(":/", QDirIterator::Subdirectories);
    int count = 0;
    while (allIt.hasNext() && count < 50) {
        QString path = allIt.next();
        if (path.contains("qml", Qt::CaseInsensitive) || path.contains("Dashboard", Qt::CaseInsensitive)) {
            qDebug() << path;
            count++;
        }
    }
    qDebug() << "------------------------------------------------------";
}

int main(int argc, char *argv[])
{
    // 檢查是否啟用 compositor 模式
    bool useCompositorMode = qEnvironmentVariableIsSet("SMART_DASHBOARD_COMPOSITOR");
    
    // 如果啟用 compositor 模式，確保 Qt 應用可以連接到顯示服務器
    // 注意：Qt 應用本身需要連接到系統的顯示服務器（Wayland 或 X11）
    // 然後創建嵌套的 compositor
    if (useCompositorMode) {
        // 確保 XDG_RUNTIME_DIR 已設置
        QString runtimeDir = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
        if (runtimeDir.isEmpty()) {
            runtimeDir = QDir::tempPath();
            qputenv("XDG_RUNTIME_DIR", runtimeDir.toUtf8());
            qDebug() << "Set XDG_RUNTIME_DIR to:" << runtimeDir;
        }
        
        // 如果沒有設置 WAYLAND_DISPLAY，嘗試使用系統的 Wayland
        // 但這不會影響我們創建的嵌套 compositor
        if (!qEnvironmentVariableIsSet("WAYLAND_DISPLAY")) {
            // 不設置，讓 Qt 使用默認的顯示服務器
            qDebug() << "WAYLAND_DISPLAY not set, Qt will use default display server";
        }
    }
    
    QGuiApplication app(argc, argv);

    // 1) 載入設定（先從 qrc，再依序嘗試 bundle/當前目錄）
    AppConfig config;
    QString configPath = QStringLiteral(":/assets/config.json");

    auto tryLoad = [&](const QString& p)->bool {
        bool ok = config.loadFromFile(p);
        if (ok) qDebug() << "Config loaded successfully from:" << p;
        return ok;
    };

    if (!tryLoad(configPath)) {
#ifdef Q_OS_MACOS
        QDir bundleDir(QCoreApplication::applicationDirPath());
        if (bundleDir.cdUp() && bundleDir.cd("Resources")) {
            const QString p = bundleDir.absoluteFilePath("assets/config.json");
            if (tryLoad(p)) goto CONFIG_DONE;
        }
#endif
        {
            QDir appDir(QCoreApplication::applicationDirPath());
            if (tryLoad(appDir.absoluteFilePath("assets/config.json"))) goto CONFIG_DONE;
            if (tryLoad(QDir::currentPath() + "/assets/config.json")) goto CONFIG_DONE;
            if (tryLoad(QDir::currentPath() + "/../assets/config.json")) goto CONFIG_DONE;
            qCritical() << "Cannot load config file from any location!";
            qCritical() << "Application dir:" << QCoreApplication::applicationDirPath();
            qCritical() << "Current dir:" << QDir::currentPath();
            return -1;
        }
    }
CONFIG_DONE:;

    // 2) 建立 QML engine 與 Context
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("AppConfig", &config);

    WaydroidManager waydroid;
    engine.rootContext()->setContextProperty("Waydroid", &waydroid);
    
    // 註冊 QML 類型
    qmlRegisterType<WindowEmbedItem>("SmartDashboard", 1, 0, "WindowEmbedItem");
    
    // 註冊 Wayland Compositor 類型（真正的 compositor 模式）
    qmlRegisterType<DashboardWaylandCompositor>("SmartDashboard", 1, 0, "WaylandCompositor");
    qmlRegisterType<SurfaceItem>("SmartDashboard", 1, 0, "SurfaceItem");
    
    // 創建 Wayland Compositor 實例（可選，需要環境變量啟用）
    // 注意：啟用 compositor 模式需要設置 WAYLAND_DISPLAY 環境變量
    bool useCompositorMode = qEnvironmentVariableIsSet("SMART_DASHBOARD_COMPOSITOR");
    DashboardWaylandCompositor *compositor = nullptr;
    
    if (useCompositorMode) {
        qDebug() << "啟用 Wayland Compositor 模式";
        compositor = new DashboardWaylandCompositor(&app);
        engine.rootContext()->setContextProperty("Compositor", compositor);
    } else {
        qDebug() << "使用標準模式（視窗疊加）";
        qDebug() << "提示：設置環境變量 SMART_DASHBOARD_COMPOSITOR=1 來啟用 compositor 模式";
    }

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [](QObject *obj, const QUrl &objUrl) {
                         if (!obj) {
                             qCritical() << "QML root object not created for URL:" << objUrl;
                             QCoreApplication::exit(-1);
                         }
                     }, Qt::QueuedConnection);

    // 3) 列印已打包的 QML 資源，協助確認路徑
    dumpQmlResources();

    // 4) 嘗試多個可能的 QML 載入路徑
    // 根據實際資源路徑 :/SmartDashboard/qml/DashboardShell.qml
    QStringList possiblePaths = {
        "qrc:/SmartDashboard/qml/DashboardShell.qml",         // 實際資源路徑（Linux）
        "qrc:/qt/qml/SmartDashboard/qml/DashboardShell.qml",  // qt_add_qml_module 標準路徑（macOS）
        "qrc:/qt/qml/SmartDashboard/DashboardShell.qml",      // 可能的路徑變體
        "qrc:/qml/DashboardShell.qml",                        // qml.qrc 路徑
        "qrc:/DashboardShell.qml"                             // 最簡單的路徑
    };

    bool loaded = false;
    for (const QString &path : possiblePaths) {
        const QUrl url(path);
        qDebug() << "嘗試載入:" << path;
        engine.load(url);
        if (!engine.rootObjects().isEmpty()) {
            qDebug() << "成功載入:" << path;
            loaded = true;
            break;
        } else {
            qWarning() << "載入失敗:" << path;
        }
    }

    if (!loaded) {
        qCritical() << "無法載入 DashboardShell.qml，已嘗試所有路徑";
        return -1;
    }

    return app.exec();
}
