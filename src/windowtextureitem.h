#pragma once

#include <QQuickItem>
#include <QQuickWindow>
#include <QOpenGLTexture>
#include <QOpenGLFramebufferObject>
#include <QWindow>
#include <QPointer>
#include <QTimer>

/**
 * WindowTextureItem
 * 
 * 真正的 compositor 模式：將外部視窗的內容作為紋理渲染到 QML 場景中
 * 
 * 工作原理：
 * 1. 使用 EGL 獲取外部視窗的渲染表面
 * 2. 將表面內容轉換為 OpenGL 紋理
 * 3. 在 QML 場景中渲染這個紋理
 * 
 * 注意：這需要 Wayland 或 X11 的特定擴展支持
 */
class WindowTextureItem : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY(QWindow* sourceWindow READ sourceWindow WRITE setSourceWindow NOTIFY sourceWindowChanged)
    QML_ELEMENT

public:
    explicit WindowTextureItem(QQuickItem *parent = nullptr)
        : QQuickItem(parent)
        , m_sourceWindow(nullptr)
    {
        setFlag(ItemHasContents, true);
        // 定期更新紋理
        connect(&m_updateTimer, &QTimer::timeout, this, &WindowTextureItem::update);
        m_updateTimer.start(16); // ~60 FPS
    }

    QWindow* sourceWindow() const { return m_sourceWindow; }
    void setSourceWindow(QWindow *w) {
        if (m_sourceWindow != w) {
            m_sourceWindow = w;
            emit sourceWindowChanged();
            update(); // 觸發重繪
        }
    }

signals:
    void sourceWindowChanged();

protected:
    QSGNode* updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override {
        // 這裡需要實現真正的紋理合成
        // 由於涉及平台特定的 EGL/Wayland API，這是一個簡化的框架
        
        if (!m_sourceWindow) {
            return oldNode;
        }

        // TODO: 實現真正的紋理合成
        // 1. 使用 EGL 獲取視窗的渲染表面
        // 2. 將表面內容讀取為 OpenGL 紋理
        // 3. 創建 QSGSimpleTextureNode 來顯示紋理
        
        // 目前返回空節點，需要平台特定的實現
        return oldNode;
    }

private:
    QPointer<QWindow> m_sourceWindow;
    QTimer m_updateTimer;
};

