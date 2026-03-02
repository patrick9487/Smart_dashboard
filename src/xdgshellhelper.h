// SPDX-FileCopyrightText: 2025
// SPDX-License-Identifier: MIT

#pragma once

#include <QObject>
#include <QPointer>

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandSeat>
#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/QWaylandXdgShell>

// 簡單的 C++ 幫手：在現有的 QML WaylandCompositor 上啟用 xdg-shell
// 用法（在 QML 中）：
//
// WaylandCompositor {
//     id: comp
//     XdgShellHelper {
//         compositor: comp
//     }
//     ...
// }
//
// 這樣就會在底層註冊 QWaylandXdgShell，讓 Waydroid 等 xdg-shell client 可以連線。

class XdgShellHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QObject *compositor READ compositor WRITE setCompositor NOTIFY compositorChanged)
    // 由 C++ 建立的 seat（QML 端不可 creatable 的 WaylandSeat，會在某些 Qt build 直接報錯）
    Q_PROPERTY(QObject *seat READ seat NOTIFY seatChanged)

public:
    explicit XdgShellHelper(QObject *parent = nullptr);

    QObject *compositor() const { return m_waylandCompositor; }
    void setCompositor(QObject *comp);
    QObject *seat() const { return m_seat; }

    // 關閉所有 toplevel（從「返回」按鈕呼叫）
    Q_INVOKABLE void closeAllToplevels();

signals:
    void compositorChanged();
    void seatChanged();
    // 只有 XDG toplevel surface（真正的 app 視窗）才會觸發這個信號。
    void toplevelSurfaceCreated(QWaylandSurface *surface);

private:
    QPointer<QWaylandCompositor> m_waylandCompositor;
    QPointer<QWaylandXdgShell> m_xdgShell;
    QPointer<QWaylandSeat> m_seat;
    QList<QPointer<QWaylandXdgToplevel>> m_toplevels;
};

