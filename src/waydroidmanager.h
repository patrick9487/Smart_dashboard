#pragma once

#include <QObject>
#include <QTimer>
#include <QProcess>
#include <QAbstractListModel>
#include <QVector>

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
            if (newState != m_running) {
                m_running = newState;
                emit runningChanged();
                if (m_running)
                    refreshApps();
                else
                    m_apps->setApps({});
            }
        });
        process->start(QStringLiteral("waydroid"), {QStringLiteral("status")});
    }

    void refreshApps() {
        if (!m_running) {
            m_apps->setApps({});
            return;
        }

        auto *process = new QProcess(this);
        connect(process, &QProcess::finished, this, [this, process](int, QProcess::ExitStatus) {
            QVector<AppEntry> apps;
            const QString output = QString::fromUtf8(process->readAllStandardOutput());
            process->deleteLater();
            const auto lines = output.split('\n', Qt::SkipEmptyParts);
            for (const QString &line : lines) {
                const int sep = line.indexOf(QStringLiteral(" - "));
                if (sep > 0) {
                    AppEntry entry;
                    entry.package = line.left(sep).trimmed();
                    entry.label = line.mid(sep + 3).trimmed();
                    if (!entry.package.isEmpty())
                        apps.push_back(std::move(entry));
                }
            }
            m_apps->setApps(std::move(apps));
        });
        process->start(QStringLiteral("waydroid"), {QStringLiteral("app"), QStringLiteral("list")});
    }

private:
    bool m_running;
    QTimer m_timer;
    AppsModel *m_apps;
};
