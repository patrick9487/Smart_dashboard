import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: dock
    width: parent ? parent.width : 800
    height: 130

    readonly property bool waydroidAvailable: (typeof Waydroid !== "undefined") && Waydroid !== null
    readonly property bool waydroidRunning: waydroidAvailable && Waydroid.running

    Rectangle {
        anchors.fill: parent
        radius: 24
        color: "#151820"
        border.color: "#2a2d36"
        opacity: 0.92
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 8

        RowLayout {
            Layout.fillWidth: true

            Text {
                Layout.fillWidth: true
                text: !waydroidAvailable ? qsTr("Waydroid unavailable")
                     : waydroidRunning
                         ? (Waydroid.appsModel.count > 0
                            ? qsTr("Android Apps: %1").arg(Waydroid.appsModel.count)
                            : qsTr("Loading apps from Waydroid…"))
                         : qsTr("Starting Waydroid…")
                color: !waydroidAvailable ? "#f07f7f"
                       : waydroidRunning ? "#8fe6c2" : "#f7b35a"
                font.pixelSize: 16
            }

            Button {
                visible: waydroidAvailable
                text: waydroidRunning ? qsTr("Reload") : qsTr("Start")
                onClicked: {
                    if (!waydroidAvailable)
                        return;
                    if (waydroidRunning)
                        Waydroid.refreshApps();
                    else
                        Waydroid.startSession();
                }
            }
        }

        Flickable {
            id: flick
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentWidth: row.width
            contentHeight: row.height
            clip: true
            interactive: contentWidth > width

            Row {
                id: row
                spacing: 24
                anchors.verticalCenter: parent.verticalCenter

                Repeater {
                    model: waydroidAvailable ? Waydroid.appsModel : 0
                    delegate: AppIcon {
                        appTitle: model.label
                        packageId: model.package
                        enabled: waydroidRunning
                    }
                }
            }
        }
    }
}
