import QtQuick
import Clicky

// Header + separator + scrollable list of active tasks. Chrome-less:
// the surrounding background, border, shadow, and rounded corners are owned
// by the parent (MorphingCompanion.qml). This item exposes an
// `implicitContentHeight` that the parent reads to size its morph target.
Item {
    id: taskListContentRoot
    implicitWidth: 320
    implicitHeight: contentColumn.implicitHeight + 28
    readonly property real implicitContentHeight: implicitHeight
    readonly property int activeTaskCount: taskRepeater.count

    Column {
        id: contentColumn
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        anchors.topMargin: 14
        spacing: 10

        // ---- Header ----
        Item {
            id: headerRow
            width: parent.width
            height: 18

            Text {
                id: headerTitle
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                text: qsTr("Active processes")
                color: "#e6e8ec"
                font.pixelSize: 13
                font.weight: Font.DemiBold
                font.letterSpacing: 0.4
            }

            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                visible: taskListContentRoot.activeTaskCount > 0
                width: countLabel.implicitWidth + 12
                height: 18
                radius: 9
                color: "#2c3038"
                border.color: "#3a3f47"
                border.width: 1

                Text {
                    id: countLabel
                    anchors.centerIn: parent
                    text: taskListContentRoot.activeTaskCount
                    color: "#cbd0d8"
                    font.pixelSize: 11
                    font.weight: Font.Medium
                }
            }
        }

        // ---- Separator ----
        Rectangle {
            width: parent.width
            height: 1
            color: "#2a2d33"
        }

        // ---- Empty state ----
        Text {
            visible: taskRepeater.count === 0
            text: qsTr("No active tasks")
            color: "#6a6e76"
            font.pixelSize: 12
            font.italic: true
            topPadding: 8
            bottomPadding: 8
        }

        // ---- Task rows ----
        Column {
            width: parent.width
            spacing: 2

            Repeater {
                id: taskRepeater
                model: companionState.activeTasksModel

                delegate: Item {
                    id: rowRoot
                    required property var task
                    width: parent.width
                    height: 44

                    readonly property bool rowIsHovered:
                        rowHoverArea.containsMouse || closeButtonHoverArea.containsMouse
                    readonly property real workProgressFraction: task.progressFraction

                    // Rows fade out when the task hits a terminal status,
                    // mirroring the satellite fade. CompanionState removes
                    // the task from the model after the matching delay.
                    opacity: task.isTerminal ? 0 : 1
                    Behavior on opacity { NumberAnimation { duration: 360; easing.type: Easing.OutCubic } }


                    Rectangle {
                        anchors.fill: parent
                        radius: 8
                        color: rowRoot.rowIsHovered ? "#2a2e36" : "transparent"
                        Behavior on color { ColorAnimation { duration: 110 } }
                    }

                    Rectangle {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        width: 3
                        radius: 1.5
                        color: rowRoot.task.color
                        opacity: rowRoot.rowIsHovered ? 0.95 : 0.0
                        Behavior on opacity { NumberAnimation { duration: 140 } }
                    }

                    MouseArea {
                        id: rowHoverArea
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.NoButton
                    }

                    Row {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 8
                        spacing: 11

                        Item {
                            anchors.verticalCenter: parent.verticalCenter
                            width: 12
                            height: 12

                            Rectangle {
                                anchors.fill: parent
                                radius: width / 2
                                gradient: Gradient {
                                    GradientStop { position: 0.0; color: Qt.lighter(rowRoot.task.color, 1.5) }
                                    GradientStop { position: 0.6; color: rowRoot.task.color }
                                    GradientStop { position: 1.0; color: Qt.darker(rowRoot.task.color, 1.4) }
                                }
                            }
                        }

                        Column {
                            anchors.verticalCenter: parent.verticalCenter
                            width: parent.width - 12 - parent.spacing
                                   - closeButton.width - parent.spacing
                            spacing: 2

                            Text {
                                text: rowRoot.task.title
                                color: "#eaecf0"
                                font.pixelSize: 13
                                font.weight: Font.Medium
                                elide: Text.ElideRight
                                width: parent.width
                            }
                            Text {
                                visible: rowRoot.task.description.length > 0
                                text: rowRoot.task.description
                                color: "#8b8f97"
                                font.pixelSize: 11
                                elide: Text.ElideRight
                                width: parent.width
                            }
                        }

                        Item {
                            id: closeButton
                            anchors.verticalCenter: parent.verticalCenter
                            width: 24
                            height: 24

                            opacity: rowRoot.rowIsHovered ? 1.0 : 0.0
                            Behavior on opacity { NumberAnimation { duration: 130 } }

                            Rectangle {
                                anchors.fill: parent
                                radius: 6
                                color: closeButtonHoverArea.containsMouse ? "#48323a" : "#34373f"
                                border.color: closeButtonHoverArea.containsMouse ? "#a83a4d" : "#3d414a"
                                border.width: 1
                                Behavior on color  { ColorAnimation { duration: 90 } }
                                Behavior on border.color { ColorAnimation { duration: 90 } }

                                Text {
                                    anchors.centerIn: parent
                                    text: "✕"
                                    color: closeButtonHoverArea.containsMouse ? "#ffd6dd" : "#cbd0d8"
                                    font.pixelSize: 11
                                    font.weight: Font.Bold
                                    Behavior on color { ColorAnimation { duration: 90 } }
                                }
                            }

                            MouseArea {
                                id: closeButtonHoverArea
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: companionState.forceCloseTask(rowRoot.task)
                            }
                        }
                    }

                    Rectangle {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        anchors.bottomMargin: 4
                        height: 2
                        radius: 1
                        color: "#2a2d33"

                        Rectangle {
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            width: parent.width * rowRoot.workProgressFraction
                            radius: 1
                            color: rowRoot.task.color
                            opacity: 0.65
                        }
                    }
                }
            }
        }
    }
}
