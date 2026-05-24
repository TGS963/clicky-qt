import QtQuick
import QtQuick.Effects
import Clicky

// Companion dot rendered in two modes:
//   - Primary dot (default): tracks the cursor, drives the voice-state
//     palette (idle/listening/processing/responding), and shows a pulse +
//     spinning ring while listening.
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

    // CompanionState.VoiceState enum value (int). Parents pass the enum
    // straight through rather than serialising via a string.
    property int currentVoiceState: CompanionState.Idle

    // ---- Cross-mode timings + sizing ----
    readonly property real primaryCoreSizePx: 16
    readonly property real satelliteCoreSizePx: 6
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
        currentVoiceState === CompanionState.Listening  ? listeningColor :
        currentVoiceState === CompanionState.Processing ? processingColor :
        currentVoiceState === CompanionState.Responding ? respondingColor :
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

    // ---- Glow halo ----
    // Rendered as a blurred copy of `glowSource` via MultiEffect. Padding
    // extends the rendered bounds so the blur isn't clipped. Per-mode tuning
    // is intentionally inline below: values are tightly coupled to whether
    // the dot is a small satellite star or the primary cursor companion.
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
            blurMax:        agentDotRoot.isSatelliteStar ? 64 : 32
            blur: 1.0
            blurMultiplier: agentDotRoot.isSatelliteStar ? 1.6 : 0.9
            brightness:     agentDotRoot.isSatelliteStar ? 0.95 : 0.12
            saturation:     agentDotRoot.isSatelliteStar ? 1.6 : 0.0
            // paddingRect must comfortably exceed blurMax*blurMultiplier or
            // the halo is clipped (satellite) / brightness-boosted-uniformly
            // into a rectangle (primary). Keep primary tight, satellite wide.
            paddingRect: agentDotRoot.isSatelliteStar
                ? Qt.rect(-120, -120, 240, 240)
                : Qt.rect(-30, -30, 60, 60)
        }
    }

    // ---- Sharp core ----
    Rectangle {
        id: sharpCore
        anchors.centerIn: parent
        width: parent.width * (agentDotRoot.isSatelliteStar ? 0.65 : 0.75)
        height: width
        radius: width / 2
        gradient: agentDotRoot.isSatelliteStar ? satelliteCoreGradient : primaryCoreGradient

        // Gradient outer stops use a fully-transparent extension so the
        // sharp core has a soft alpha falloff at the rim. Combined with the
        // MultiEffect glow halo, the edge blends into the background.
        Gradient {
            id: satelliteCoreGradient
            GradientStop { position: 0.0;  color: "#ffffff" }
            GradientStop { position: 0.25; color: Qt.lighter(agentDotRoot.resolvedBaseColor, 1.4) }
            GradientStop { position: 0.55; color: agentDotRoot.resolvedBaseColor }
            GradientStop { position: 0.85; color: Qt.rgba(agentDotRoot.resolvedOuterColor.r,
                                                          agentDotRoot.resolvedOuterColor.g,
                                                          agentDotRoot.resolvedOuterColor.b,
                                                          0.45) }
            GradientStop { position: 1.0;  color: Qt.rgba(agentDotRoot.resolvedOuterColor.r,
                                                          agentDotRoot.resolvedOuterColor.g,
                                                          agentDotRoot.resolvedOuterColor.b,
                                                          0.0) }
        }
        Gradient {
            id: primaryCoreGradient
            GradientStop { position: 0.0;  color: agentDotRoot.resolvedHighlightColor }
            GradientStop { position: 0.5;  color: agentDotRoot.resolvedBaseColor }
            GradientStop { position: 0.85; color: Qt.rgba(agentDotRoot.resolvedOuterColor.r,
                                                          agentDotRoot.resolvedOuterColor.g,
                                                          agentDotRoot.resolvedOuterColor.b,
                                                          0.5) }
            GradientStop { position: 1.0;  color: Qt.rgba(agentDotRoot.resolvedOuterColor.r,
                                                          agentDotRoot.resolvedOuterColor.g,
                                                          agentDotRoot.resolvedOuterColor.b,
                                                          0.0) }
        }

        // Listening pulse — primary only.
        SequentialAnimation on scale {
            running: agentDotRoot.currentVoiceState === CompanionState.Listening
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

        opacity: agentDotRoot.currentVoiceState === CompanionState.Listening ? 0.9 : 0.0
        Behavior on opacity { NumberAnimation { duration: 180 } }

        RotationAnimation on rotation {
            running: agentDotRoot.currentVoiceState === CompanionState.Listening
                     && !agentDotRoot.isSatelliteStar
            loops: Animation.Infinite
            from: 0
            to: 360
            duration: agentDotRoot.ringRotationDurationMs
        }
    }
}
