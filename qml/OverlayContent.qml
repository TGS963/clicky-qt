import QtQuick
import Clicky

// Root of the overlay scene. Renders:
//   - one primary AgentDot that smoothly follows the cursor (or docks to
//     the top-right) and flashes a task color during listening
//   - a Repeater of satellite stars, one per active TaskItem, orbiting the
//     primary dot with a brief birth animation and a fade-out near end of life
Item {
    id: overlayRoot
    anchors.fill: parent

    // ---- Layout tunables ----
    readonly property real followOffset: 22       // primary dot offset from cursor (x and y)
    readonly property real dockOffset: 60         // dock distance from screen corner
    readonly property real orbitRadiusPx: 22      // satellite radius around primary
    readonly property real orbitFullRotationSeconds: 3.6
    readonly property real satelliteBirthDurationSeconds: 0.32

    // ---- Transition timings ----
    readonly property int dockTransitionDurationMs: 360
    readonly property int followTransitionDurationMs: 80
    readonly property real satelliteBirthFadeInSeconds: 0.15
    readonly property real satelliteFadeOutLeadSeconds: 0.6

    // ---- Derived state ----
    readonly property string currentVoiceState: voiceStateNameFor(companionState.voiceState)
    readonly property bool isDocked: companionState.overlayMode === CompanionState.Docked

    readonly property real cursorPosX: companionState.cursorScreenPosition.x
    readonly property real cursorPosY: companionState.cursorScreenPosition.y

    readonly property real primaryTargetX: isDocked
        ? overlayRoot.width - dockOffset
        : cursorPosX + followOffset
    readonly property real primaryTargetY: isDocked
        ? dockOffset
        : cursorPosY + followOffset

    // Satellites orbit the primary dot's *rendered* position rather than the
    // raw cursor — primary smooths cursor jitter via its own Behaviors, so
    // binding here keeps satellites in lockstep with the primary's visible
    // motion instead of fighting with cursor-poll jitter.
    readonly property real smoothedOrbitCenterX: primaryAgentDot.x + primaryAgentDot.width / 2
    readonly property real smoothedOrbitCenterY: primaryAgentDot.y + primaryAgentDot.height / 2

    function voiceStateNameFor(stateEnumValue) {
        if (stateEnumValue === CompanionState.Listening) return "Listening";
        if (stateEnumValue === CompanionState.Processing) return "Processing";
        if (stateEnumValue === CompanionState.Responding) return "Responding";
        return "Idle";
    }

    AgentDot {
        id: primaryAgentDot
        isSatelliteStar: false
        currentVoiceState: overlayRoot.currentVoiceState

        x: overlayRoot.primaryTargetX - width / 2
        y: overlayRoot.primaryTargetY - height / 2

        // Color flash on task spawn — animated smoothly via Behavior in AgentDot.
        overrideColor: companionState.primaryFlashActive
            ? companionState.primaryFlashColor
            : "transparent"

        Behavior on x {
            NumberAnimation {
                duration: overlayRoot.isDocked
                    ? overlayRoot.dockTransitionDurationMs
                    : overlayRoot.followTransitionDurationMs
                easing.type: overlayRoot.isDocked ? Easing.OutCubic : Easing.OutQuad
            }
        }
        Behavior on y {
            NumberAnimation {
                duration: overlayRoot.isDocked
                    ? overlayRoot.dockTransitionDurationMs
                    : overlayRoot.followTransitionDurationMs
                easing.type: overlayRoot.isDocked ? Easing.OutCubic : Easing.OutQuad
            }
        }
    }

    Repeater {
        model: companionState.activeTasks
        AgentDot {
            id: satelliteDot
            required property var modelData

            // Birth animation: radius grows from 0 to orbitRadiusPx within
            // satelliteBirthDurationSeconds of life, so the satellite looks
            // like it shot out of the primary dot rather than popped in.
            readonly property real birthProgress: Math.min(
                1.0,
                modelData.elapsedSeconds / overlayRoot.satelliteBirthDurationSeconds)
            readonly property real easedBirthProgress: 1 - Math.pow(1 - birthProgress, 3)
            readonly property real currentRadiusPx: easedBirthProgress * overlayRoot.orbitRadiusPx

            readonly property real currentAngleRadians:
                (modelData.orbitAngleBaseDegrees * Math.PI / 180.0)
                + (modelData.elapsedSeconds * 2.0 * Math.PI / overlayRoot.orbitFullRotationSeconds)

            // Fade in over the first satelliteBirthFadeInSeconds and fade out
            // over the last satelliteFadeOutLeadSeconds of the task's life.
            readonly property real fadeOpacity: {
                var remaining = modelData.totalLifetimeSeconds - modelData.elapsedSeconds;
                if (remaining < overlayRoot.satelliteFadeOutLeadSeconds) {
                    return Math.max(0, remaining / overlayRoot.satelliteFadeOutLeadSeconds);
                }
                if (modelData.elapsedSeconds < overlayRoot.satelliteBirthFadeInSeconds) {
                    return modelData.elapsedSeconds / overlayRoot.satelliteBirthFadeInSeconds;
                }
                return 1.0;
            }

            isSatelliteStar: true
            overrideColor: modelData.color
            currentVoiceState: "Idle"
            opacity: fadeOpacity

            x: overlayRoot.smoothedOrbitCenterX
               + Math.cos(currentAngleRadians) * currentRadiusPx
               - width / 2
            y: overlayRoot.smoothedOrbitCenterY
               + Math.sin(currentAngleRadians) * currentRadiusPx
               - height / 2
        }
    }
}
