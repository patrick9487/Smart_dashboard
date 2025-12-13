#include "xdgshellhelper.h"

#include <QDebug>

XdgShellHelper::XdgShellHelper(QObject *parent)
    : QObject(parent)
{
}

void XdgShellHelper::setCompositor(QObject *comp)
{
    if (m_waylandCompositor == comp)
        return;

    // 先清掉舊的 xdgShell
    if (m_xdgShell) {
        m_xdgShell->deleteLater();
        m_xdgShell.clear();
    }
    // 清掉舊 seat
    if (m_seat) {
        m_seat->deleteLater();
        m_seat.clear();
    }

    m_waylandCompositor = qobject_cast<QWaylandCompositor *>(comp);
    if (!m_waylandCompositor) {
        qWarning() << "XdgShellHelper: compositor 不是 QWaylandCompositor 實例";
        emit compositorChanged();
        emit seatChanged();
        return;
    }

    // 在現有 compositor 上建立 seat（讓 client 可以收到 pointer/keyboard）
    // 注意：有些 Qt build 的 QML WaylandSeat 是 uncreatable，必須由 C++ 建立後再暴露到 QML。
    m_seat = new QWaylandSeat(m_waylandCompositor);
    m_seat->setName(QStringLiteral("seat0"));
    qInfo() << "XdgShellHelper: Wayland seat created:" << m_seat->name();

    // 在現有 compositor 上建立 QWaylandXdgShell 擴充
    m_xdgShell = new QWaylandXdgShell(m_waylandCompositor);

    // 輸出 debug 訊息幫助確認
    QObject::connect(m_xdgShell, &QWaylandXdgShell::toplevelCreated,
                     this, [](QWaylandXdgToplevel *toplevel, QWaylandXdgSurface *xdgSurface) {
        Q_UNUSED(toplevel);
        qInfo() << "XdgShellHelper: xdg toplevel created for surface" << xdgSurface;
    });

    qInfo() << "XdgShellHelper: XDG Shell 已啟用";
    emit compositorChanged();
    emit seatChanged();
}

