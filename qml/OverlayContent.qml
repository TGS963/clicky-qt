import QtQuick
import Clicky

// Root of the overlay scene. Renders:
//   - one AgentDot that follows the cursor; visible only in Passive
//   - one MorphingCompanion that morphs from a small blue surface (right
//     under the AgentDot) into the task menu card; invisible in Passive
//   - a Repeater of satellite stars orbiting the dot position; hidden while
//     the menu is open
Item {
    id: overlayRoot
    anchors.fill: parent

    // ---- Layout tunables ----
    readonly property real followOffset: 22
    readonly property real orbitRadiusPx: 11
    readonly property real orbitFullRotationSeconds: 3.6
    readonly property real satelliteBirthDurationSeconds: 0.32
    readonly property real satelliteBirthFadeInSeconds: 0.15
    readonly property real satelliteFadeOutLeadSeconds: 0.6
    readonly property int  followTransitionDurationMs: 80

    // Shared continuous rotation phase. Every satellite reads this value, so
    // their orbit angles stay locked relative to each other and the
    // index-based stationary spread remains equispaced regardless of when
    // each task spawned.
    property real globalOrbitPhaseRadians: 0
    NumberAnimation on globalOrbitPhaseRadians {
        running: true
        from: 0
        to: 2 * Math.PI
        duration: orbitFullRotationSeconds * 1000
        loops: Animation.Infinite
    }

    // ---- Derived state ----
    // Pass the VoiceState enum through as an int. Both AgentDot and
    // MorphingCompanion compare against CompanionState.Listening etc
    // directly — no enum→string→ternary round-trip.
    readonly property int currentVoiceState: companionState.voiceState
    readonly property bool isMenuOpen: companionState.interactionMode === CompanionState.MenuOpen

    readonly property real cursorPosX: companionState.cursorScreenPosition.x
    readonly property real cursorPosY: companionState.cursorScreenPosition.y
    readonly property real dotTargetX: cursorPosX + followOffset
    readonly property real dotTargetY: cursorPosY + followOffset

    // ---- Click-outside-to-dismiss backdrop (menu open only) ----
    MouseArea {
        id: menuDismissBackdrop
        anchors.fill: parent
        enabled: overlayRoot.isMenuOpen
        visible: overlayRoot.isMenuOpen
        hoverEnabled: false
        z: 0
        onClicked: companionState.closeTaskMenu()
    }

    // ---- Primary companion dot (Passive only) ----
    AgentDot {
        id: primaryAgentDot
        z: 2

        isSatelliteStar: false
        currentVoiceState: overlayRoot.currentVoiceState
        overrideColor: companionState.primaryFlashActive
            ? companionState.primaryFlashColor
            : "transparent"

        x: overlayRoot.dotTargetX - width / 2
        y: overlayRoot.dotTargetY - height / 2

        opacity: overlayRoot.isMenuOpen ? 0.0 : 1.0
        Behavior on opacity {
            NumberAnimation {
                duration: 160
                easing.type: Easing.InOutQuad
            }
        }
        visible: opacity > 0.01

        Behavior on x {
            NumberAnimation {
                duration: overlayRoot.followTransitionDurationMs
                easing.type: Easing.OutQuad
            }
        }
        Behavior on y {
            NumberAnimation {
                duration: overlayRoot.followTransitionDurationMs
                easing.type: Easing.OutQuad
            }
        }
    }

    // ---- Morphing companion (MenuOpen only — invisible in Passive) ----
    MorphingCompanion {
        id: morphingCompanion
        z: 3
        currentVoiceState: overlayRoot.currentVoiceState
        cursorPosX: overlayRoot.cursorPosX
        cursorPosY: overlayRoot.cursorPosY
        followOffsetPx: overlayRoot.followOffset
    }

    // ---- Satellite stars ----
    Repeater {
        id: satelliteRepeater
        model: companionState.activeTasksModel
        AgentDot {
            id: satelliteDot
            required property var task
            required property int index

            readonly property real birthProgress: Math.min(
                1.0,
                task.elapsedSeconds / overlayRoot.satelliteBirthDurationSeconds)
            readonly property real easedBirthProgress: 1 - Math.pow(1 - birthProgress, 3)
            readonly property real currentRadiusPx: easedBirthProgress * overlayRoot.orbitRadiusPx

            // Stationary angular slot from index/count keeps satellites
            // equispaced; the shared globalOrbitPhase makes them all rotate
            // together so the spacing is preserved over time. Not readonly
            // so the Behavior below can smooth re-bindings when count
            // changes (task spawn / expire).
            property real stationaryAngleRadians:
                2.0 * Math.PI * index / Math.max(1, satelliteRepeater.count)
            Behavior on stationaryAngleRadians {
                NumberAnimation {
                    duration: 320
                    easing.type: Easing.OutCubic
                }
            }
            readonly property real currentAngleRadians:
                stationaryAngleRadians + overlayRoot.globalOrbitPhaseRadians

            // Fade in over the birth window; once terminal, fade to 0
            // smoothly (model removal is scheduled with a matching delay in
            // CompanionState).
            readonly property real fadeOpacity: {
                if (task.isTerminal) return 0.0;
                if (task.elapsedSeconds < overlayRoot.satelliteBirthFadeInSeconds) {
                    return task.elapsedSeconds / overlayRoot.satelliteBirthFadeInSeconds;
                }
                return 1.0;
            }

            isSatelliteStar: true
            overrideColor: task.color
            currentVoiceState: CompanionState.Idle
            opacity: fadeOpacity * (overlayRoot.isMenuOpen ? 0.0 : 1.0)
            Behavior on opacity { NumberAnimation { duration: 360; easing.type: Easing.OutCubic } }
            z: 1

            // Orbit around the AgentDot's rendered center.
            readonly property real orbitCenterX: primaryAgentDot.x + primaryAgentDot.width / 2
            readonly property real orbitCenterY: primaryAgentDot.y + primaryAgentDot.height / 2

            x: orbitCenterX + Math.cos(currentAngleRadians) * currentRadiusPx - width / 2
            y: orbitCenterY + Math.sin(currentAngleRadians) * currentRadiusPx - height / 2
        }
    }
}
