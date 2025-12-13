// SPDX-FileCopyrightText: 2025
// SPDX-License-Identifier: MIT

#pragma once

#include <QObject>
#include <QPointer>

#include <QtWaylandCompositor/QWaylandCompositor>
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

public:
    explicit XdgShellHelper(QObject *parent = nullptr);

    QObject *compositor() const { return m_waylandCompositor; }
    void setCompositor(QObject *comp);

signals:
    void compositorChanged();

private:
    QPointer<QWaylandCompositor> m_waylandCompositor;
    QPointer<QWaylandXdgShell> m_xdgShell;
};

