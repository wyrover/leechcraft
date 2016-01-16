import QtQuick 2.3
import org.LC.common 1.0

Rectangle {
    id: rootRect

    property real length: battView.count * parent.quarkBaseSize
    implicitWidth: viewOrient == "vertical" ? parent.quarkBaseSize : length
    implicitHeight: viewOrient == "vertical" ? length : parent.quarkBaseSize
    width: parent.quarkBaseSize
    height: parent.quarkBaseSize

    color: "transparent"

    Common { id: commonJS }

    ListView {
        id: battView

        width: parent.length
        height: rootRect.parent.quarkBaseSize
        interactive: false

        orientation: ListView.Horizontal

        model: Liznoo_proxy.batteryModel

        delegate: Item {
            height: battView.height
            width: battView.height

            ActionButton {
                id: battButton

                anchors.fill: parent

                marginsManaged: true
                anchors.margins: margins

                onTriggered: Liznoo_proxy.batteryHistoryDialogRequested(batteryId)

                Rectangle {
                    width: parent.width - anchors.rightMargin
                    height: width / 1.62

                    anchors.centerIn: parent
                    anchors.rightMargin: 2

                    radius: 3
                    border.width: 1
                    border.color: colorProxy.color_ToolButton_SelectedBorderColor

                    gradient: Gradient {
                        GradientStop { position: 0.0; color: colorProxy.color_ToolButton_TopColor }
                        GradientStop { position: 1.0; color: colorProxy.color_ToolButton_BottomColor }
                    }

                    Rectangle {
                        id: battLevelFillRect
                        anchors.fill: parent
                        property real baseMargin: 1
                        anchors.leftMargin: baseMargin
                        anchors.bottomMargin: baseMargin
                        anchors.topMargin: baseMargin
                        anchors.rightMargin: baseMargin + (parent.width - 2 * baseMargin) * (1 - percentage / 100)

                        radius: parent.radius

                        function adjustColor(c, base) {
                            function lightness(c) {
                                // https://en.wikipedia.org/wiki/HSL_and_HSV#Lightness
                                return (c.r + c.g + c.b) / 3;
                            }

                            return lightness(c) >= lightness(base) ?
                                        Qt.lighter(c) :
                                        Qt.darker(c);
                        }

                        gradient: Gradient {
                            GradientStop {
                                position: 0.0
                                color: battLevelFillRect.adjustColor(colorProxy.color_ToolButton_HoveredBottomColor, colorProxy.color_ToolButton_BottomColor)
                            }
                            GradientStop {
                                position: 1.0
                                color: battLevelFillRect.adjustColor(colorProxy.color_ToolButton_HoveredTopColor, colorProxy.color_ToolButton_TopColor)
                            }
                        }
                    }

                    Rectangle {
                        anchors.left: parent.right
                        width: parent.anchors.rightMargin
                        height: parent.height / 3
                        anchors.verticalCenter: parent.verticalCenter

                        radius: parent.radius

                        color: colorProxy.color_ToolButton_SelectedBorderColor
                    }

                    Text {
                        id: battLevelText
                        color: colorProxy.color_ToolButton_TextColor
                        text: percentage + '%'
                        font.pointSize: 8

                        anchors.centerIn: parent
                    }
                }
            }
        }
    }
}
