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

static void dumpQmlResources()
{
    // 列出實際打包進去的 QML 資源，便於確認路徑是否正確
    QDirIterator it(":/qt/qml/SmartDashboard", QDirIterator::Subdirectories);
    qDebug() << "---- QML resources under :/qt/qml/SmartDashboard ----";
    while (it.hasNext())
        qDebug() << it.next();
    qDebug() << "------------------------------------------------------";
}

int main(int argc, char *argv[])
{
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

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [](QObject *obj, const QUrl &objUrl) {
                         if (!obj) {
                             qCritical() << "QML root object not created for URL:" << objUrl;
                             QCoreApplication::exit(-1);
                         }
                     }, Qt::QueuedConnection);

    // 3)（可選）列印已打包的 QML 資源，協助確認路徑
    dumpQmlResources();

    // 4) 統一用 qrc 路徑載入（對應 qt_add_qml_module 的 URI=SmartDashboard）
    // qt_add_qml_module 會生成路徑：qrc:/qt/qml/{URI}/{QML_FILES的路徑}
    // 因為 URI=SmartDashboard，QML_FILES 包含 qml/DashboardShell.qml
    // 所以實際路徑是：qrc:/qt/qml/SmartDashboard/qml/DashboardShell.qml
    const QUrl kHomeUrl(u"qrc:/qt/qml/SmartDashboard/qml/DashboardShell.qml"_qs);
    engine.load(kHomeUrl);

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
