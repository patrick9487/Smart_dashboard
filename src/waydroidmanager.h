#pragma once

#include <QObject>
#include <QTimer>
#include <QProcess>
#include <QAbstractListModel>
#include <QVector>
#include <QDebug>

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
    {
        connect(&m_timer, &QTimer::timeout, this, &WaydroidManager::checkStatus);
        m_timer.start(2000);
        checkStatus();
    }

    bool running() const { return m_running; }
    AppsModel *appsModel() const { return m_apps; }

    Q_INVOKABLE void startSession() { QProcess::startDetached(QStringLiteral("waydroid"), {QStringLiteral("container"), QStringLiteral("start")}); }
    Q_INVOKABLE void stopSession() { QProcess::startDetached(QStringLiteral("waydroid"), {QStringLiteral("container"), QStringLiteral("stop")}); }
    Q_INVOKABLE void showFullUI() { QProcess::startDetached(QStringLiteral("waydroid"), {QStringLiteral("show-full-ui")}); }
    Q_INVOKABLE void launchApp(const QString &pkg) { QProcess::startDetached(QStringLiteral("waydroid"), {QStringLiteral("app"), QStringLiteral("launch"), pkg}); }

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
                    qDebug() << "WaydroidManager::checkStatus() - calling refreshApps()";
                    refreshApps();
                } else {
                    m_apps->setApps({});
                }
            } else if (m_running) {
                // 即使狀態沒改變，如果 Waydroid 在運行，也定期刷新應用列表
                // 這樣可以處理應用程式列表延遲載入的情況
                qDebug() << "WaydroidManager::checkStatus() - Waydroid still running, refreshing apps";
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

        qDebug() << "WaydroidManager::refreshApps() - fetching app list";
        auto *process = new QProcess(this);
        connect(process, &QProcess::finished, this, [this, process](int exitCode, QProcess::ExitStatus) {
            QVector<AppEntry> apps;
            const QString output = QString::fromUtf8(process->readAllStandardOutput());
            const QString error = QString::fromUtf8(process->readAllStandardError());
            process->deleteLater();
            
            if (exitCode != 0) {
                qWarning() << "WaydroidManager::refreshApps() - waydroid app list failed, exit code:" << exitCode;
                qWarning() << "Error output:" << error;
                return;
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
            
            qDebug() << "WaydroidManager::refreshApps() - total apps:" << apps.size();
            m_apps->setApps(std::move(apps));
        });
        process->start(QStringLiteral("waydroid"), {QStringLiteral("app"), QStringLiteral("list")});
    }

private:
    bool m_running;
    QTimer m_timer;
    AppsModel *m_apps;
};
