#pragma once

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandOutput>
#include <QtWaylandCompositor/QWaylandXdgShell>
#include <QtWaylandCompositor/QWaylandXdgToplevel>
#include <QtWaylandCompositor/QWaylandXdgSurface>
#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/QWaylandWlShell>
#include <QtWaylandCompositor/QWaylandWlShellSurface>
#include <QObject>
#include <QHash>
#include <QList>
#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QSize>
#include <QTimer>
#include <QDebug>

/**
 * DashboardWaylandCompositor
 * 
 * 自定義的 Wayland Compositor，用於將 Waydroid 應用視窗嵌入到 Dashboard 中
 * 
 * 工作原理：
 * 1. 創建 Wayland compositor 實例
 * 2. 監聽新視窗的創建
 * 3. 將視窗表面映射到 QML 場景中
 */
class DashboardWaylandCompositor : public QWaylandCompositor {
    Q_OBJECT
    Q_PROPERTY(QWaylandXdgShell* xdgShell READ xdgShell CONSTANT)
    Q_PROPERTY(QWaylandWlShell* wlShell READ wlShell CONSTANT)

public:
    explicit DashboardWaylandCompositor(QObject *parent = nullptr)
        : QWaylandCompositor(parent)
    {
        // 創建 XDG Shell（現代 Wayland 應用使用）
        m_xdgShell = new QWaylandXdgShell(this);
        connect(m_xdgShell, &QWaylandXdgShell::toplevelCreated, this,
                &DashboardWaylandCompositor::onXdgToplevelCreated);
        
        // 創建 WL Shell（舊版 Wayland 應用使用）
        m_wlShell = new QWaylandWlShell(this);
        // 注意：QWaylandWlShell 可能沒有 shellSurfaceCreated 信號
        // 我們通過 surfaceCreated 來處理所有表面
        
        // 監聽所有表面的創建
        connect(this, &QWaylandCompositor::surfaceCreated, this,
                &DashboardWaylandCompositor::onSurfaceCreated);
        
        qDebug() << "DashboardWaylandCompositor: Initialized";
    }

    QWaylandXdgShell* xdgShell() const { return m_xdgShell; }
    QWaylandWlShell* wlShell() const { return m_wlShell; }
    
    // 註冊包名與表面的映射關係（當應用啟動時調用）
    Q_INVOKABLE void registerPackageSurface(const QString &packageName, QWaylandSurface *surface) {
        if (surface) {
            m_packageToSurface[packageName] = surface;
            qDebug() << "DashboardWaylandCompositor: Registered surface for package" << packageName;
        }
    }
    
    // 根據包名查找對應的表面（如果還沒找到，會等待表面創建）
    Q_INVOKABLE QWaylandSurface* findSurfaceByPackage(const QString &packageName) {
        // 首先檢查已註冊的映射
        if (m_packageToSurface.contains(packageName)) {
            QWaylandSurface *surface = m_packageToSurface[packageName];
            if (surface && hasSurfaceContent(surface)) {
                return surface;
            }
        }
        
        // 如果沒有找到，添加到待匹配列表
        if (!m_pendingPackages.contains(packageName)) {
            m_pendingPackages.append(packageName);
            qDebug() << "DashboardWaylandCompositor: Added package to pending list:" << packageName;
        }
        
        // 嘗試立即匹配（如果表面已經存在）
        QString searchTerm = packageName;
        if (packageName.contains('.')) {
            QStringList parts = packageName.split('.');
            if (!parts.isEmpty()) {
                searchTerm = parts.last();
            }
        }
        
        for (auto *surface : m_surfaces) {
            if (!hasSurfaceContent(surface)) continue;
            
            // 檢查 XDG Surface
            for (auto *xdgSurface : m_xdgSurfaces) {
                if (xdgSurface->surface() == surface && xdgSurface->toplevel()) {
                    QString title = xdgSurface->toplevel()->title();
                    if (title.contains(searchTerm, Qt::CaseInsensitive) || 
                        title.contains(packageName, Qt::CaseInsensitive)) {
                        m_packageToSurface[packageName] = surface;
                        m_pendingPackages.removeAll(packageName);
                        return surface;
                    }
                }
            }
            
            // 檢查 WL Shell Surface（通過查找關聯的表面）
            // 注意：由於無法直接獲取 WL Shell Surface，我們跳過這個檢查
            // 主要依賴 XDG Shell 來匹配表面
        }
        
        return nullptr; // 還沒找到，等待表面創建
    }
    
    // 獲取所有已映射的表面
    Q_INVOKABLE QVariantList getAllMappedSurfaces() {
        QVariantList result;
        for (auto *surface : m_surfaces) {
            if (hasSurfaceContent(surface)) {
                result.append(QVariant::fromValue(surface));
            }
        }
        return result;
    }

signals:
    void surfaceCreated(QWaylandSurface *surface);
    void surfaceMapped(QWaylandSurface *surface);
    void surfaceUnmapped(QWaylandSurface *surface);
    void surfaceMatchedToPackage(const QString &packageName, QWaylandSurface *surface);

private slots:
    void onSurfaceCreated(QWaylandSurface *surface) {
        qDebug() << "DashboardWaylandCompositor: Surface created";
        m_surfaces.append(surface);
        
        // 監聽表面提交事件來判斷是否已映射
        // QWaylandSurface 有 committed 信號，當表面提交新內容時觸發
        connect(surface, &QWaylandSurface::committed, this, [this, surface]() {
            if (hasSurfaceContent(surface)) {
                qDebug() << "DashboardWaylandCompositor: Surface mapped (committed)";
                emit surfaceMapped(surface);
            }
        });
        
        // 監聽表面銷毀
        connect(surface, &QObject::destroyed, this, [this, surface]() {
            qDebug() << "DashboardWaylandCompositor: Surface destroyed";
            emit surfaceUnmapped(surface);
            m_surfaces.removeAll(surface);
        });
        
        // 如果表面已經有內容，立即觸發 mapped 信號
        if (hasSurfaceContent(surface)) {
            QTimer::singleShot(0, this, [this, surface]() {
                emit surfaceMapped(surface);
            });
        }
        
        emit surfaceCreated(surface);
    }
    
    void onXdgToplevelCreated(QWaylandXdgToplevel *toplevel, QWaylandXdgSurface *xdgSurface) {
        qDebug() << "DashboardWaylandCompositor: XDG Toplevel created";
        // 可以從 toplevel 獲取應用信息
        if (toplevel && xdgSurface) {
            m_xdgSurfaces.append(xdgSurface);
            connect(toplevel, &QWaylandXdgToplevel::titleChanged, this, [this, toplevel, xdgSurface]() {
                QString title = toplevel->title();
                qDebug() << "DashboardWaylandCompositor: XDG Toplevel title:" << title;
                // 嘗試根據標題匹配包名
                matchSurfaceToPackage(xdgSurface->surface(), title);
            });
            // 立即檢查標題
            if (!toplevel->title().isEmpty()) {
                matchSurfaceToPackage(xdgSurface->surface(), toplevel->title());
            }
        }
    }
    
    // 注意：由於 QWaylandWlShell 可能沒有 shellSurfaceCreated 信號
    // 我們通過其他方式處理 WL Shell 表面
    // 如果需要，可以在 onSurfaceCreated 中檢查表面類型
    
    // 嘗試將表面匹配到包名
    void matchSurfaceToPackage(QWaylandSurface *surface, const QString &title) {
        // 遍歷所有等待匹配的包名
        for (auto it = m_pendingPackages.begin(); it != m_pendingPackages.end();) {
            const QString &packageName = *it;
            QString searchTerm = packageName;
            if (packageName.contains('.')) {
                QStringList parts = packageName.split('.');
                if (!parts.isEmpty()) {
                    searchTerm = parts.last();
                }
            }
            
            // 檢查標題是否匹配
            if (title.contains(searchTerm, Qt::CaseInsensitive) || 
                title.contains(packageName, Qt::CaseInsensitive)) {
                m_packageToSurface[packageName] = surface;
                qDebug() << "DashboardWaylandCompositor: Matched surface to package" << packageName;
                it = m_pendingPackages.erase(it);
                emit surfaceMatchedToPackage(packageName, surface);
            } else {
                ++it;
            }
        }
    }

    // 檢查表面是否有內容（替代 isMapped）
    bool hasSurfaceContent(QWaylandSurface *surface) const {
        if (!surface) return false;
        // 檢查表面是否有緩衝區（buffer）
        // 如果有緩衝區，說明表面已經有內容
        return surface->hasContent();
    }

private:
    QWaylandXdgShell *m_xdgShell = nullptr;
    QWaylandWlShell *m_wlShell = nullptr;
    QList<QWaylandSurface*> m_surfaces;
    QList<QWaylandXdgSurface*> m_xdgSurfaces;
    QHash<QString, QWaylandSurface*> m_packageToSurface; // 包名到表面的映射
    QStringList m_pendingPackages; // 等待匹配的包名列表
};

