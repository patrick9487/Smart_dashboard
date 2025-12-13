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
    // Qt 版本差異：有些版本的 QWaylandSeat 沒有 setName()/name()，用 objectName + 動態 property 兼容
    m_seat->setObjectName(QStringLiteral("seat0"));
    m_seat->setProperty("name", QStringLiteral("seat0"));
    qInfo() << "XdgShellHelper: Wayland seat created (objectName):" << m_seat->objectName();

    // 讓 compositor 使用這個 seat 作為 default seat（避免需要 WaylandMouseTracker.seat 這種版本不一致的 QML API）
    // 使用 meta-call，若該 Qt 版本沒有這個方法也不會編譯失敗/崩潰，只是返回 false。
    const bool defaultSeatOk = QMetaObject::invokeMethod(
        m_waylandCompositor,
        "setDefaultSeat",
        Q_ARG(QWaylandSeat*, m_seat)
    );
    qInfo() << "XdgShellHelper: setDefaultSeat invoke ok =" << defaultSeatOk;

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

