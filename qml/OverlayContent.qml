import QtQuick
import Clicky

// Root of the overlay scene. One instance runs per physical screen.
// `screenRect` is injected as a context property by OverlayWindow (C++);
// it holds the screen's global geometry so this file can:
//   - convert global cursor coords → window-local coords
//   - show the dot / menu only when content belongs to this screen
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
    readonly property int currentVoiceState: companionState.voiceState
    readonly property bool isMenuOpen: companionState.interactionMode === CompanionState.MenuOpen

    // Global cursor coordinates from CompanionState.
    readonly property real cursorPosX: companionState.cursorScreenPosition.x
    readonly property real cursorPosY: companionState.cursorScreenPosition.y

    // Window-local cursor coordinates (global minus this screen's top-left).
    // screenRect is injected per-window by OverlayWindow::OverlayWindow().
    readonly property real localCursorX: cursorPosX - screenRect.x
    readonly property real localCursorY: cursorPosY - screenRect.y

    readonly property real dotTargetX: localCursorX + followOffset
    readonly property real dotTargetY: localCursorY + followOffset

    // Cursor is on this screen when its global position falls inside screenRect.
    readonly property bool isCursorOnThisScreen:
        cursorPosX >= screenRect.x && cursorPosX < screenRect.x + screenRect.width &&
        cursorPosY >= screenRect.y && cursorPosY < screenRect.y + screenRect.height

    // Menu belongs to the screen where the cursor was when the menu opened.
    readonly property bool isMenuOnThisScreen: isMenuOpen && (
        companionState.taskMenuAnchorPosition.x >= screenRect.x &&
        companionState.taskMenuAnchorPosition.x < screenRect.x + screenRect.width &&
        companionState.taskMenuAnchorPosition.y >= screenRect.y &&
        companionState.taskMenuAnchorPosition.y < screenRect.y + screenRect.height)

    // ---- Click-outside-to-dismiss backdrop (menu open on this screen only) ----
    MouseArea {
        id: menuDismissBackdrop
        anchors.fill: parent
        enabled: overlayRoot.isMenuOnThisScreen
        visible: overlayRoot.isMenuOnThisScreen
        hoverEnabled: false
        z: 0
        onClicked: companionState.closeTaskMenu()
    }

    // ---- Screenshot thumbnail preview (flashed ~1s after a grab) ----
    // Shows what was just captured, near the cursor, on the screen the cursor
    // is on. Source + visibility are driven by CompanionState; the fade is
    // local. Width is a tunable; height follows the image aspect ratio.
    Image {
        id: screenshotPreviewThumbnail
        readonly property real thumbnailWidthPx: 160

        z: 4
        width: thumbnailWidthPx
        fillMode: Image.PreserveAspectFit
        cache: false  // each capture is a distinct file; don't cache stale ones
        source: companionState.screenshotPreviewSource

        x: overlayRoot.localCursorX + overlayRoot.followOffset
        y: overlayRoot.localCursorY - height - overlayRoot.followOffset

        opacity: (companionState.screenshotPreviewActive
                  && overlayRoot.isCursorOnThisScreen) ? 1.0 : 0.0
        visible: opacity > 0.01
        Behavior on opacity {
            NumberAnimation { duration: 180; easing.type: Easing.OutQuad }
        }
    }

    // ---- Primary companion dot (Passive only, this screen only) ----
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
        visible: opacity > 0.01 && overlayRoot.isCursorOnThisScreen

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

    // ---- Morphing companion (MenuOpen on this screen only) ----
    MorphingCompanion {
        id: morphingCompanion
        z: 3
        currentVoiceState: overlayRoot.currentVoiceState
        cursorPosX: overlayRoot.localCursorX
        cursorPosY: overlayRoot.localCursorY
        followOffsetPx: overlayRoot.followOffset
        screenOriginX: screenRect.x
        screenOriginY: screenRect.y
        isMenuOpen: overlayRoot.isMenuOnThisScreen
    }

    // ---- Satellite stars (follow primary dot, this screen only) ----
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
            opacity: fadeOpacity
                     * (overlayRoot.isMenuOpen ? 0.0 : 1.0)
                     * (overlayRoot.isCursorOnThisScreen ? 1.0 : 0.0)
            Behavior on opacity { NumberAnimation { duration: 360; easing.type: Easing.OutCubic } }
            z: 1

            readonly property real orbitCenterX: primaryAgentDot.x + primaryAgentDot.width / 2
            readonly property real orbitCenterY: primaryAgentDot.y + primaryAgentDot.height / 2

            x: orbitCenterX + Math.cos(currentAngleRadians) * currentRadiusPx - width / 2
            y: orbitCenterY + Math.sin(currentAngleRadians) * currentRadiusPx - height / 2
        }
    }
}
