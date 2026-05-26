import QtQuick
import QtQuick.Effects
import Clicky

// Companion dot rendered in two modes:
//   - Primary dot (default): tracks the cursor, drives the voice-state
//     palette (idle/listening/processing/responding). While listening, the
//     dot grows and renders a small bar waveform inside, fed by
//     CompanionState.listeningAmplitudes.
//   - Satellite star (isSatelliteStar=true): smaller, color comes entirely
//     from overrideColor. Skips listening visuals entirely.
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

    readonly property bool isPrimaryListening:
        !isSatelliteStar && currentVoiceState === CompanionState.Listening

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
    property color blendedVoiceColor: Qt.rgba(
        voiceStateColor.r * (1.0 - overrideColor.a) + overrideColor.r * overrideColor.a,
        voiceStateColor.g * (1.0 - overrideColor.a) + overrideColor.g * overrideColor.a,
        voiceStateColor.b * (1.0 - overrideColor.a) + overrideColor.b * overrideColor.a,
        1.0)

    // While the primary dot is listening, swap to a neutral charcoal/silver
    // backdrop. The flash/voice color does not tint the dot itself — it
    // becomes the *bar* color, leaving the dot as a contrasting surface for
    // the waveform to sit on.
    readonly property color listeningBackdropColor: "#2c2f37"
    property color resolvedBaseColor:
        isPrimaryListening ? listeningBackdropColor : blendedVoiceColor
    Behavior on resolvedBaseColor {
        ColorAnimation {
            duration: agentDotRoot.colorFlashDurationMs
            easing.type: Easing.InOutCubic
        }
    }

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
            blurMax:        agentDotRoot.isSatelliteStar ? 96 : 32
            blur: 1.0
            blurMultiplier: agentDotRoot.isSatelliteStar ? 2.1 : 0.9
            brightness:     agentDotRoot.isSatelliteStar ? 1.35 : 0.12
            saturation:     agentDotRoot.isSatelliteStar ? 2.1 : 0.0
            // paddingRect must comfortably exceed blurMax*blurMultiplier or
            // the halo is clipped (satellite) / brightness-boosted-uniformly
            // into a rectangle (primary). Keep primary tight, satellite wide.
            paddingRect: agentDotRoot.isSatelliteStar
                ? Qt.rect(-160, -160, 320, 320)
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
            GradientStop { position: 0.30; color: Qt.lighter(agentDotRoot.resolvedBaseColor, 1.7) }
            GradientStop { position: 0.60; color: Qt.lighter(agentDotRoot.resolvedBaseColor, 1.25) }
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

    }

    // ---- Inline listening waveform (primary only, when listening) ----
    // Bars are sized + positioned to fit inside the grown dot. Each delegate
    // pulls its height from `companionState.listeningAmplitudes[index]`.
    Row {
        id: listeningWaveform
        anchors.centerIn: parent
        spacing: 1.2

        // 5 bars at 1.2 px wide + 1.2 px spacing = 10.8 px row inside a
        // 16 px primary dot. Max height capped to a chord short of the
        // full diameter so bars don't poke past the circular silhouette.
        readonly property real barWidth: 1.2
        readonly property real baseBarHeight: 2
        readonly property real maxBarHeight:
            Math.max(baseBarHeight, agentDotRoot.width * 0.7)
        // Bars carry the upcoming task color (the random hue chosen at
        // PTT-start), so the dot itself is just the neutral backdrop.
        readonly property color barColor:
            companionState.primaryFlashColor.a > 0
                ? companionState.primaryFlashColor
                : "#fff4d6"

        opacity: agentDotRoot.isPrimaryListening ? 1.0 : 0.0
        visible: opacity > 0.01
        Behavior on opacity { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }

        Repeater {
            model: companionState.listeningBarCount
            delegate: Rectangle {
                required property int index
                readonly property real amplitude: {
                    const amps = companionState.listeningAmplitudes;
                    return index < amps.length ? amps[index] : 0.0;
                }
                width: listeningWaveform.barWidth
                height: listeningWaveform.baseBarHeight
                        + amplitude * (listeningWaveform.maxBarHeight
                                       - listeningWaveform.baseBarHeight)
                radius: width / 2
                color: listeningWaveform.barColor
                anchors.verticalCenter: parent.verticalCenter
                Behavior on height {
                    NumberAnimation { duration: 80; easing.type: Easing.OutCubic }
                }
            }
        }
    }
}
