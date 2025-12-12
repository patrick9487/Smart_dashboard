#pragma once

#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/qwaylandquickitem.h>
#include <QQuickItem>
#include <QPointer>

/**
 * SurfaceItem
 * 
 * QML 組件，用於在 QML 場景中顯示 Wayland 表面
 * 
 * 這是真正的 compositor 模式：表面內容直接渲染到 QML 場景中
 * 
 * 注意：使用 QWaylandQuickItem 來顯示表面
 */
class SurfaceItem : public QWaylandQuickItem {
    Q_OBJECT
    Q_PROPERTY(QWaylandSurface* surface READ surface WRITE setSurface NOTIFY surfaceChanged)
    QML_ELEMENT

public:
    explicit SurfaceItem(QQuickItem *parent = nullptr)
        : QWaylandQuickItem(parent)
    {
    }

    QWaylandSurface* surface() const {
        return QWaylandQuickItem::surface();
    }
    
    void setSurface(QWaylandSurface *s) {
        if (surface() != s) {
            QWaylandQuickItem::setSurface(s);
            // QWaylandQuickItem 自動處理輸入事件轉發
            // 不需要手動設置輸入區域
            emit surfaceChanged();
        }
    }
    
    // QWaylandQuickItem 已經處理了輸入事件轉發
    // 當用戶在 Item 上點擊時，事件會自動轉發到對應的表面

signals:
    void surfaceChanged();
};

