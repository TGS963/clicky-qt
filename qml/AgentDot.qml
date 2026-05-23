import QtQuick
import QtQuick.Effects

// Companion dot used in two modes:
//   - Primary dot (isSatelliteStar=false): tracks the cursor, drives the
//     voice-state palette (idle/listening/processing/responding), and shows
//     a pulse + spinning ring while listening.
//   - Satellite star (isSatelliteStar=true): smaller, color comes entirely
//     from overrideColor. Skips the listening pulse / ring entirely.
//
// `overrideColor` is animated as an alpha-weighted blend over the
// voice-state color so transitions between blue and a task color are smooth.
Item {
    id: agentDotRoot

    property bool isSatelliteStar: false

    // Transparent (alpha == 0) means "no override — use voiceStateColor".
    // Anything non-transparent blends through that color. ColorAnimation
    // makes the transition smooth in both directions.
    property color overrideColor: "transparent"
    Behavior on overrideColor {
        ColorAnimation {
            duration: agentDotRoot.colorFlashDurationMs
            easing.type: Easing.InOutCubic
        }
    }

    property string currentVoiceState: "Idle"

    // ---- Cross-mode timings + sizing ----
    readonly property real primaryCoreSizePx: 16
    readonly property real satelliteCoreSizePx: 8
    readonly property int colorFlashDurationMs: 380
    readonly property int pulseAnimationDurationMs: 550
    readonly property int ringRotationDurationMs: 1100

    width: isSatelliteStar ? satelliteCoreSizePx : primaryCoreSizePx
    height: width

    // ---- Voice-state palette (primary mode) ----
    readonly property color idleColor: "#5aa9ff"
    readonly property color listeningColor: "#5aa9ff"
    readonly property color processingColor: "#ffc85a"
    readonly property color respondingColor: "#78dc82"

    property color voiceStateColor:
        currentVoiceState === "Listening"  ? listeningColor :
        currentVoiceState === "Processing" ? processingColor :
        currentVoiceState === "Responding" ? respondingColor :
                                             idleColor

    // Alpha-weighted blend between voiceStateColor and overrideColor. As the
    // Behavior on overrideColor animates the alpha, this glides cleanly
    // between blue and the flash color with no threshold jump.
    property color resolvedBaseColor: Qt.rgba(
        voiceStateColor.r * (1.0 - overrideColor.a) + overrideColor.r * overrideColor.a,
        voiceStateColor.g * (1.0 - overrideColor.a) + overrideColor.g * overrideColor.a,
        voiceStateColor.b * (1.0 - overrideColor.a) + overrideColor.b * overrideColor.a,
        1.0)

    property color resolvedHighlightColor: Qt.lighter(resolvedBaseColor, 1.9)
    property color resolvedOuterColor: Qt.darker(resolvedBaseColor, 1.4)

    // ---- Glow halo (both modes) ----
    // Rendered as a blurred copy of `glowSource` via MultiEffect. Padding
    // extends the rendered bounds so the blur isn't clipped. Per-mode blur /
    // brightness tuning is intentionally inline below: these values are
    // tightly coupled to whether the dot is a small star or the larger
    // companion, and moving them to top-of-file properties would lose that
    // mode coupling.
    Rectangle {
        id: glowSource
        anchors.centerIn: parent
        width: parent.width
        height: parent.height
        radius: width / 2
        color: agentDotRoot.resolvedBaseColor
        layer.enabled: true
        layer.effect: MultiEffect {
            blurEnabled: true
            blurMax: 32
            blur: 1.0
            blurMultiplier: agentDotRoot.isSatelliteStar ? 1.1 : 0.9
            brightness:     agentDotRoot.isSatelliteStar ? 0.15 : 0.12
            saturation: 0.0
            paddingRect: Qt.rect(-30, -30, 60, 60)
        }
    }

    // ---- Sharp core (both modes) ----
    Rectangle {
        id: sharpCore
        anchors.centerIn: parent
        width: parent.width * 0.75
        height: width
        radius: width / 2
        gradient: Gradient {
            GradientStop { position: 0.0;  color: agentDotRoot.resolvedHighlightColor }
            GradientStop { position: 0.55; color: agentDotRoot.resolvedBaseColor }
            GradientStop { position: 1.0;  color: agentDotRoot.resolvedOuterColor }
        }

        // Listening pulse — primary only.
        SequentialAnimation on scale {
            running: agentDotRoot.currentVoiceState === "Listening"
                     && !agentDotRoot.isSatelliteStar
            loops: Animation.Infinite
            NumberAnimation {
                from: 1.0; to: 1.18
                duration: agentDotRoot.pulseAnimationDurationMs
                easing.type: Easing.InOutSine
            }
            NumberAnimation {
                from: 1.18; to: 1.0
                duration: agentDotRoot.pulseAnimationDurationMs
                easing.type: Easing.InOutSine
            }
        }
    }

    // ---- Spinning ring (primary only, when listening) ----
    Rectangle {
        id: spinningRing
        visible: !agentDotRoot.isSatelliteStar
        anchors.centerIn: parent
        width: 26
        height: 26
        radius: 13
        color: "transparent"
        border.width: 2
        border.color: agentDotRoot.resolvedBaseColor

        opacity: agentDotRoot.currentVoiceState === "Listening" ? 0.9 : 0.0
        Behavior on opacity { NumberAnimation { duration: 180 } }

        RotationAnimation on rotation {
            running: agentDotRoot.currentVoiceState === "Listening"
                     && !agentDotRoot.isSatelliteStar
            loops: Animation.Infinite
            from: 0
            to: 360
            duration: agentDotRoot.ringRotationDurationMs
        }
    }
}
