#pragma once

#include <QObject>
#include <QWindow>
#include <QQuickItem>
#include <QProcess>
#include <QTimer>
#include <QDebug>
#include <QString>

/**
 * WaydroidWindowEmbedder
 * 
 * 負責將 Waydroid 應用的視窗嵌入到 Qt/QML 應用中
 * 
 * 工作原理：
 * 1. 啟動 Waydroid 應用後，等待視窗出現
 * 2. 通過 X11/Wayland 協議查找對應的視窗 ID
 * 3. 使用 QWindow::fromWinId() 創建 QWindow
 * 4. 將 QWindow 嵌入到 QML 中
 */
class WaydroidWindowEmbedder : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString packageName READ packageName WRITE setPackageName NOTIFY packageNameChanged)
    Q_PROPERTY(bool embedded READ embedded NOTIFY embeddedChanged)
    Q_PROPERTY(QWindow* embeddedWindow READ embeddedWindow NOTIFY embeddedWindowChanged)

public:
    explicit WaydroidWindowEmbedder(QObject *parent = nullptr)
        : QObject(parent)
        , m_embedded(false)
        , m_window(nullptr)
    {
        // 定期檢查視窗是否出現
        connect(&m_checkTimer, &QTimer::timeout, this, &WaydroidWindowEmbedder::tryFindAndEmbed);
        m_checkTimer.setSingleShot(false);
    }

    ~WaydroidWindowEmbedder() {
        if (m_window) {
            m_window->deleteLater();
        }
    }

    QString packageName() const { return m_packageName; }
    void setPackageName(const QString &pkg) {
        if (m_packageName != pkg) {
            m_packageName = pkg;
            emit packageNameChanged();
            // 當包名改變時，重置嵌入狀態
            if (m_window) {
                m_window->deleteLater();
                m_window = nullptr;
            }
            m_embedded = false;
            emit embeddedChanged();
        }
    }

    bool embedded() const { return m_embedded; }
    QWindow* embeddedWindow() const { return m_window; }

    Q_INVOKABLE void startEmbedding() {
        if (m_packageName.isEmpty()) {
            qWarning() << "WaydroidWindowEmbedder: packageName is empty";
            return;
        }

        qDebug() << "WaydroidWindowEmbedder: Starting embedding for" << m_packageName;
        
        // 啟動應用
        QProcess::startDetached("waydroid", {"app", "launch", m_packageName});
        
        // 開始定期檢查視窗
        m_checkTimer.start(500); // 每 500ms 檢查一次
    }

    Q_INVOKABLE void stopEmbedding() {
        m_checkTimer.stop();
        if (m_window) {
            m_window->deleteLater();
            m_window = nullptr;
        }
        m_embedded = false;
        emit embeddedChanged();
        emit embeddedWindowChanged();
    }

signals:
    void packageNameChanged();
    void embeddedChanged();
    void embeddedWindowChanged();

private slots:
    void tryFindAndEmbed() {
#ifdef Q_OS_LINUX
        // 在 Linux 上，嘗試通過 xdotool 或 wmctrl 查找視窗
        // 注意：這需要系統安裝 xdotool 或 wmctrl
        
        // 方法 1: 使用 xdotool 查找視窗（推薦，更精確）
        QProcess *proc = new QProcess(this);
        connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [this, proc](int exitCode, QProcess::ExitStatus) {
            if (exitCode == 0) {
                QString output = QString::fromUtf8(proc->readAllStandardOutput()).trimmed();
                QStringList winIds = output.split('\n', Qt::SkipEmptyParts);
                for (const QString &winIdStr : winIds) {
                    bool ok;
                    WId winId = winIdStr.toULongLong(&ok, 16); // 十六進制
                    if (ok && winId != 0) {
                        embedWindow(winId);
                        break; // 找到第一個就嵌入
                    }
                }
            }
            proc->deleteLater();
        });
        
        // 嘗試使用 xdotool 查找視窗
        // 查找標題或類名包含包名的視窗
        proc->start("xdotool", {"search", "--class", m_packageName});
        
        // 如果 5 秒後還沒找到，停止檢查
        static int attempts = 0;
        if (!m_embedded) {
            attempts++;
            if (attempts > 10) { // 5 秒 = 10 * 500ms
                m_checkTimer.stop();
                attempts = 0;
                qWarning() << "WaydroidWindowEmbedder: Failed to find window after 5 seconds for" << m_packageName;
            }
        } else {
            m_checkTimer.stop();
            attempts = 0;
        }
#else
        // 非 Linux 平台暫時不支持
        qWarning() << "WaydroidWindowEmbedder: Window embedding is only supported on Linux";
        m_checkTimer.stop();
#endif
    }

private:
    void embedWindow(WId winId) {
        if (m_embedded && m_window) {
            return; // 已經嵌入
        }

        qDebug() << "WaydroidWindowEmbedder: Attempting to embed window" << QString::number(winId, 16);

#ifdef Q_OS_LINUX
        // 使用 QWindow::fromWinId() 創建 QWindow
        // 注意：這需要視窗系統支持（X11 或 Wayland）
        m_window = QWindow::fromWinId(winId);
        if (m_window) {
            // 不設置 parent，讓視窗獨立管理
            // 設置視窗標誌，使其適合嵌入
            m_window->setFlags(Qt::FramelessWindowHint);
            m_embedded = true;
            emit embeddedChanged();
            emit embeddedWindowChanged();
            qDebug() << "WaydroidWindowEmbedder: Successfully embedded window" << QString::number(winId, 16);
        } else {
            qWarning() << "WaydroidWindowEmbedder: Failed to create QWindow from WinId" << QString::number(winId, 16);
        }
#else
        Q_UNUSED(winId);
        qWarning() << "WaydroidWindowEmbedder: Window embedding not supported on this platform";
#endif
    }

    QString m_packageName;
    bool m_embedded;
    QWindow *m_window;
    QTimer m_checkTimer;
};

