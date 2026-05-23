import QtQuick

Item {
    id: agentDotRoot
    width: 28
    height: 28

    property string currentVoiceState: "Idle"

    readonly property color idleColorOuter: "#1f6fd6"
    readonly property color idleColorMid: "#5aa9ff"
    readonly property color idleColorHighlight: "#c8dcff"

    readonly property color listeningColorOuter: "#1f6fd6"
    readonly property color listeningColorMid: "#5aa9ff"
    readonly property color listeningColorHighlight: "#d6e8ff"

    readonly property color processingColorOuter: "#d68f1f"
    readonly property color processingColorMid: "#ffc85a"
    readonly property color processingColorHighlight: "#fff0c8"

    readonly property color respondingColorOuter: "#2fa83f"
    readonly property color respondingColorMid: "#78dc82"
    readonly property color respondingColorHighlight: "#d8f3dc"

    property color resolvedOuterColor:
        currentVoiceState === "Listening" ? listeningColorOuter :
        currentVoiceState === "Processing" ? processingColorOuter :
        currentVoiceState === "Responding" ? respondingColorOuter :
        idleColorOuter

    property color resolvedMidColor:
        currentVoiceState === "Listening" ? listeningColorMid :
        currentVoiceState === "Processing" ? processingColorMid :
        currentVoiceState === "Responding" ? respondingColorMid :
        idleColorMid

    property color resolvedHighlightColor:
        currentVoiceState === "Listening" ? listeningColorHighlight :
        currentVoiceState === "Processing" ? processingColorHighlight :
        currentVoiceState === "Responding" ? respondingColorHighlight :
        idleColorHighlight

    Rectangle {
        id: agentInnerBall
        anchors.centerIn: parent
        width: 20
        height: 20
        radius: 10
        gradient: Gradient {
            GradientStop { position: 0.0; color: agentDotRoot.resolvedHighlightColor }
            GradientStop { position: 0.55; color: agentDotRoot.resolvedMidColor }
            GradientStop { position: 1.0; color: agentDotRoot.resolvedOuterColor }
        }

        SequentialAnimation on scale {
            running: agentDotRoot.currentVoiceState === "Listening"
            loops: Animation.Infinite
            NumberAnimation { from: 1.0; to: 1.18; duration: 550; easing.type: Easing.InOutSine }
            NumberAnimation { from: 1.18; to: 1.0; duration: 550; easing.type: Easing.InOutSine }
        }
    }

    Rectangle {
        id: spinningRing
        anchors.centerIn: parent
        width: 34
        height: 34
        radius: 17
        color: "transparent"
        border.width: 2
        border.color: agentDotRoot.resolvedMidColor
        opacity: agentDotRoot.currentVoiceState === "Listening" ? 0.9 : 0.0
        Behavior on opacity { NumberAnimation { duration: 180 } }

        RotationAnimation on rotation {
            running: agentDotRoot.currentVoiceState === "Listening"
            loops: Animation.Infinite
            from: 0
            to: 360
            duration: 1100
        }
    }
}
