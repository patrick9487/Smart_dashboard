#pragma once

#include <QObject>
#include <QTimer>
#include <QProcess>
#include <QAbstractListModel>
#include <QVector>
#include <QDebug>
#include <QRegularExpression>

struct AppEntry {
    QString label;
    QString package;
};

class AppsModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
public:
    enum Roles { LabelRole = Qt::UserRole + 1, PackageRole };

    explicit AppsModel(QObject *parent = nullptr)
        : QAbstractListModel(parent) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return m_apps.size();
    }

    QVariant data(const QModelIndex &index, int role) const override {
        if (!index.isValid() || index.row() < 0 || index.row() >= m_apps.size())
            return {};
        const auto &app = m_apps.at(index.row());
        switch (role) {
        case LabelRole:
            return app.label;
        case PackageRole:
            return app.package;
        default:
            return {};
        }
    }

    QHash<int, QByteArray> roleNames() const override {
        return {{LabelRole, QByteArrayLiteral("label")},
                {PackageRole, QByteArrayLiteral("package")}};
    }

    void setApps(QVector<AppEntry> apps) {
        beginResetModel();
        m_apps = std::move(apps);
        endResetModel();
        emit countChanged();
    }

signals:
    void countChanged();

private:
    QVector<AppEntry> m_apps;
};

class WaydroidManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(AppsModel *appsModel READ appsModel CONSTANT)
public:
    explicit WaydroidManager(QObject *parent = nullptr)
        : QObject(parent)
        , m_running(false)
        , m_apps(new AppsModel(this))
        , m_refreshing(false)
        , m_refreshProcess(nullptr)
        , m_startupDelayDone(false)
        , m_refreshRetryCount(0)
    {
        connect(&m_timer, &QTimer::timeout, this, &WaydroidManager::checkStatus);
        m_timer.start(3000);  // 改為 3 秒，給 Waydroid 更多時間
        checkStatus();
    }

    bool running() const { return m_running; }
    AppsModel *appsModel() const { return m_apps; }

    Q_INVOKABLE void startSession() { QProcess::startDetached(QStringLiteral("waydroid"), {QStringLiteral("container"), QStringLiteral("start")}); }
    Q_INVOKABLE void stopSession() { QProcess::startDetached(QStringLiteral("waydroid"), {QStringLiteral("container"), QStringLiteral("stop")}); }
    Q_INVOKABLE void showFullUI() { QProcess::startDetached(QStringLiteral("waydroid"), {QStringLiteral("show-full-ui")}); }
    Q_INVOKABLE void launchApp(const QString &pkg) { QProcess::startDetached(QStringLiteral("waydroid"), {QStringLiteral("app"), QStringLiteral("launch"), pkg}); }
    
    // 創建視窗嵌入器（返回給 QML 使用）
    Q_INVOKABLE QObject* createWindowEmbedder(const QString &pkg);

signals:
    void runningChanged();

public slots:
    void checkStatus() {
        auto *process = new QProcess(this);
        connect(process, &QProcess::finished, this, [this, process](int, QProcess::ExitStatus) {
            const QString output = QString::fromUtf8(process->readAllStandardOutput());
            process->deleteLater();
            const bool newState = output.contains(QStringLiteral("RUNNING"), Qt::CaseInsensitive)
                                  || output.contains(QStringLiteral("Running: Yes"), Qt::CaseInsensitive);
            
            qDebug() << "WaydroidManager::checkStatus() - output:" << output.trimmed();
            qDebug() << "WaydroidManager::checkStatus() - newState:" << newState << "current m_running:" << m_running;
            
            if (newState != m_running) {
                m_running = newState;
                qDebug() << "WaydroidManager::checkStatus() - state changed to:" << m_running;
                emit runningChanged();
                if (m_running) {
                    // Waydroid 剛啟動，等待 8 秒讓它完全初始化
                    // （"Failed to get service waydroidplatform" 表示還沒準備好）
                    m_startupDelayDone = false;
                    m_refreshRetryCount = 0;
                    qDebug() << "WaydroidManager::checkStatus() - Waydroid just started, waiting 8s for initialization...";
                    QTimer::singleShot(8000, this, [this]() {
                        m_startupDelayDone = true;
                        qDebug() << "WaydroidManager: Startup delay done, now refreshing apps";
                        refreshApps();
                    });
                } else {
                    m_startupDelayDone = false;
                    m_apps->setApps({});
                }
            } else if (m_running && m_apps->rowCount() == 0 && m_startupDelayDone && m_refreshRetryCount < 5) {
                // 只在啟動延遲完成後且重試次數 < 5 時才刷新
                qDebug() << "WaydroidManager::checkStatus() - Waydroid running but no apps, retry" << m_refreshRetryCount;
                refreshApps();
            }
        });
        process->start(QStringLiteral("waydroid"), {QStringLiteral("status")});
    }

    void refreshApps() {
        if (!m_running) {
            qDebug() << "WaydroidManager::refreshApps() - Waydroid not running, clearing apps";
            m_apps->setApps({});
            return;
        }

        // 避免重複請求
        if (m_refreshing) {
            qDebug() << "WaydroidManager::refreshApps() - already refreshing, skipping";
            return;
        }
        m_refreshing = true;
        m_refreshRetryCount++;

        qDebug() << "WaydroidManager::refreshApps() - fetching app list (attempt" << m_refreshRetryCount << ")";
        m_refreshProcess = new QProcess(this);
        auto *process = m_refreshProcess;
        
        // 超時處理（30 秒，Waydroid 初始化可能很慢）
        m_refreshTimeout = new QTimer(this);
        m_refreshTimeout->setSingleShot(true);
        connect(m_refreshTimeout, &QTimer::timeout, this, [this]() {
            qWarning() << "WaydroidManager::refreshApps() - TIMEOUT after 30s";
            m_refreshing = false;
            if (m_refreshProcess && m_refreshProcess->state() != QProcess::NotRunning) {
                // 斷開信號連接，避免 finished 處理器被調用
                m_refreshProcess->disconnect();
                m_refreshProcess->kill();
                m_refreshProcess->waitForFinished(1000);
                m_refreshProcess->deleteLater();
                m_refreshProcess = nullptr;
            }
        });
        m_refreshTimeout->start(30000);
        
        connect(process, &QProcess::finished, this, [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
            // 停止超時計時器
            if (m_refreshTimeout) {
                m_refreshTimeout->stop();
                m_refreshTimeout->deleteLater();
                m_refreshTimeout = nullptr;
            }
            
            m_refreshing = false;
            m_refreshProcess = nullptr;
            
            // 如果是被 kill 的（超時），直接返回
            if (exitStatus == QProcess::CrashExit) {
                qWarning() << "WaydroidManager::refreshApps() - process was killed or crashed";
                process->deleteLater();
                return;
            }
            QVector<AppEntry> apps;
            const QString output = QString::fromUtf8(process->readAllStandardOutput());
            const QString error = QString::fromUtf8(process->readAllStandardError());
            process->deleteLater();
            
            if (exitCode != 0) {
                qWarning() << "WaydroidManager::refreshApps() - waydroid app list failed, exit code:" << exitCode;
                qWarning() << "Error output:" << error;
                return;
            }

            // 即使成功也輸出 stderr（有些環境會把提示/警告寫到 stderr，但仍返回 0）
            if (!error.trimmed().isEmpty()) {
                qWarning() << "WaydroidManager::refreshApps() - stderr:" << error.trimmed();
            }

            qDebug() << "WaydroidManager::refreshApps() - output:" << output;
            
            // 解析 waydroid app list 的輸出格式：
            // Name: 電話
            // packageName: com.google.android.dialer
            // categories:
            //     android.intent.category.LAUNCHER
            // (空行分隔每個應用)
            
            const auto lines = output.split('\n', Qt::KeepEmptyParts);
            QString currentName;
            QString currentPackage;
            
            for (const QString &line : lines) {
                const QString trimmed = line.trimmed();
                
                if (trimmed.startsWith(QStringLiteral("Name:"))) {
                    // 如果之前有收集到完整的應用資訊，先保存
                    if (!currentPackage.isEmpty() && !currentName.isEmpty()) {
                        AppEntry entry;
                        entry.package = currentPackage;
                        entry.label = currentName;
                        apps.push_back(std::move(entry));
                        qDebug() << "WaydroidManager::refreshApps() - found app:" << entry.package << "-" << entry.label;
                        currentName.clear();
                        currentPackage.clear();
                    }
                    // 提取應用名稱
                    currentName = trimmed.mid(5).trimmed(); // "Name: " 之後的內容
                } else if (trimmed.startsWith(QStringLiteral("packageName:"))) {
                    // 提取包名
                    currentPackage = trimmed.mid(12).trimmed(); // "packageName: " 之後的內容
                } else if (trimmed.isEmpty() && !currentPackage.isEmpty() && !currentName.isEmpty()) {
                    // 遇到空行且已有完整資訊，保存應用
                    AppEntry entry;
                    entry.package = currentPackage;
                    entry.label = currentName;
                    apps.push_back(std::move(entry));
                    qDebug() << "WaydroidManager::refreshApps() - found app:" << entry.package << "-" << entry.label;
                    currentName.clear();
                    currentPackage.clear();
                }
            }
            
            // 處理最後一個應用（如果沒有空行結尾）
            if (!currentPackage.isEmpty() && !currentName.isEmpty()) {
                AppEntry entry;
                entry.package = currentPackage;
                entry.label = currentName;
                apps.push_back(std::move(entry));
                qDebug() << "WaydroidManager::refreshApps() - found app:" << entry.package << "-" << entry.label;
            }

            // Fallback 解析：不同 Waydroid 版本可能輸出為「<package> - <label>」或「<package> <label>」或「<label> (<package>)」
            if (apps.isEmpty() && !output.trimmed().isEmpty()) {
                qDebug() << "WaydroidManager::refreshApps() - primary parse found 0 apps, trying fallback parsers";
                const auto rawLines = output.split('\n', Qt::SkipEmptyParts);
                const QRegularExpression rePkgDashLabel(QStringLiteral(R"(^([\w.\-]+)\s*-\s*(.+)$)"));
                const QRegularExpression reLabelParenPkg(QStringLiteral(R"(^(.+?)\s*\(([\w.\-]+)\)\s*$)"));
                const QRegularExpression rePkgSpaceLabel(QStringLiteral(R"(^([\w.\-]+)\s+(.+)$)"));

                for (const QString &line : rawLines) {
                    const QString trimmed = line.trimmed();
                    if (trimmed.isEmpty())
                        continue;

                    // 跳過可能的標題行
                    if (trimmed.startsWith(QStringLiteral("Apps"), Qt::CaseInsensitive) ||
                        trimmed.startsWith(QStringLiteral("List"), Qt::CaseInsensitive)) {
                        continue;
                    }

                    QRegularExpressionMatch m;

                    m = rePkgDashLabel.match(trimmed);
                    if (m.hasMatch()) {
                        const QString pkg = m.captured(1).trimmed();
                        const QString label = m.captured(2).trimmed();
                        if (pkg.contains('.') && !label.isEmpty()) {
                            apps.push_back(AppEntry{label, pkg});
                            continue;
                        }
                    }

                    m = reLabelParenPkg.match(trimmed);
                    if (m.hasMatch()) {
                        const QString label = m.captured(1).trimmed();
                        const QString pkg = m.captured(2).trimmed();
                        if (pkg.contains('.') && !label.isEmpty()) {
                            apps.push_back(AppEntry{label, pkg});
                            continue;
                        }
                    }

                    m = rePkgSpaceLabel.match(trimmed);
                    if (m.hasMatch()) {
                        const QString first = m.captured(1).trimmed();
                        const QString rest = m.captured(2).trimmed();
                        // 只有當第一段看起來像 package 時才接受
                        if (first.contains('.') && !rest.isEmpty()) {
                            apps.push_back(AppEntry{rest, first});
                            continue;
                        }
                    }
                }

                // 去重（以 package 為 key）
                if (!apps.isEmpty()) {
                    QHash<QString, QString> seen;
                    QVector<AppEntry> dedup;
                    dedup.reserve(apps.size());
                    for (const auto &a : apps) {
                        if (a.package.isEmpty() || seen.contains(a.package))
                            continue;
                        seen.insert(a.package, a.label);
                        dedup.push_back(a);
                    }
                    apps = std::move(dedup);
                }
            }
            
            qDebug() << "WaydroidManager::refreshApps() - total apps:" << apps.size();
            m_apps->setApps(std::move(apps));
        });
        process->start(QStringLiteral("waydroid"), {QStringLiteral("app"), QStringLiteral("list")});
    }

private:
    bool m_running;
    QTimer m_timer;
    AppsModel *m_apps;
    bool m_refreshing;
    QProcess *m_refreshProcess;
    QTimer *m_refreshTimeout = nullptr;
    bool m_startupDelayDone;
    int m_refreshRetryCount;
};
