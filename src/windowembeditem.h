#pragma once

#include <QQuickItem>
#include <QQuickWindow>
#include <QWindow>
#include <QPointer>
#include <QRectF>
#include <QPoint>
#include <QPointF>

/**
 * WindowEmbedItem
 * 
 * QQuickItem 子類，用於在 QML 中顯示嵌入的外部視窗（QWindow）
 * 
 * 使用方法：
 * - 設置 window 屬性為要嵌入的 QWindow
 * - 該 Item 會自動調整大小並顯示視窗內容
 */
class WindowEmbedItem : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY(QWindow* window READ window WRITE setWindow NOTIFY windowChanged)
    QML_ELEMENT

public:
    explicit WindowEmbedItem(QQuickItem *parent = nullptr)
        : QQuickItem(parent)
        , m_window(nullptr)
    {
        setFlag(ItemHasContents, true);
    }

    QWindow* window() const { return m_window; }
    void setWindow(QWindow *w) {
        if (m_window != w) {
            if (m_window) {
                disconnect(m_window, nullptr, this, nullptr);
            }
            m_window = w;
            if (m_window) {
                // 當視窗大小改變時，更新 Item 大小
                connect(m_window, &QWindow::widthChanged, this, &WindowEmbedItem::updateSize);
                connect(m_window, &QWindow::heightChanged, this, &WindowEmbedItem::updateSize);
                // 當視窗可見性改變時，更新 Item 可見性
                connect(m_window, &QWindow::visibleChanged, this, [this]() {
                    setVisible(m_window->isVisible());
                });
                updateSize();
                setVisible(m_window->isVisible());
            }
            emit windowChanged();
            update(); // 觸發重繪
        }
    }

signals:
    void windowChanged();

protected:
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override {
        QQuickItem::geometryChange(newGeometry, oldGeometry);
        updateWindowGeometry();
    }
    
    QSGNode* updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override {
        // 在 QML 中顯示外部視窗需要使用特殊的渲染方式
        // 這裡我們返回一個簡單的節點，實際渲染由 Qt 的視窗系統處理
        // 注意：這是一個簡化實現，可能需要根據平台調整
        
        if (!m_window) {
            return oldNode;
        }

        updateWindowGeometry();
        return oldNode;
    }
    
    void updateWindowGeometry() {
        if (!m_window) {
            return;
        }
        
        // 獲取 Item 在場景中的全局位置
        QPointF globalPos = mapToScene(QPointF(0, 0));
        
        // 獲取主視窗
        QQuickWindow *quickWindow = window();
        if (!quickWindow) {
            return;
        }
        
        // 轉換為屏幕坐標
        QPoint screenPos = quickWindow->mapToGlobal(QPoint(
            static_cast<int>(globalPos.x()),
            static_cast<int>(globalPos.y())
        ));
        
        // 設置視窗位置和大小
        QRectF itemRect = boundingRect();
        m_window->setGeometry(
            screenPos.x(),
            screenPos.y(),
            static_cast<int>(itemRect.width()),
            static_cast<int>(itemRect.height())
        );
        
        if (!m_window->isVisible()) {
            m_window->show();
        }
    }

private slots:
    void updateSize() {
        if (m_window) {
            // 當視窗大小改變時，可以選擇是否同步 Item 大小
            // 這裡我們保持 Item 的大小由 QML 控制
        }
    }

private:
    QPointer<QWindow> m_window;
};

