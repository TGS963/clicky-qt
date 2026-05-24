import QtQuick
import QtQuick.Effects
import Clicky

// Two-stage morph:
//   Passive       — root is fully transparent; user sees the AgentDot dot.
//   MenuOpen      — root takes over: fades in (still tiny + blue), then
//                   expands + recolors into the menu card. AgentDot fades
//                   out during the crossfade phase so the two never visibly
//                   stack at full strength.
//
// The crossfade-first / geometry-second staging is achieved by using a
// SequentialAnimation { PauseAnimation; NumberAnimation } inside the
// geometry Behaviors. The pause holds the geometry at its rest size while
// opacities crossfade; only after the crossfade is mostly done does the
// rectangle start growing into the menu.
Rectangle {
    id: morphRoot

    // ---- Wired in by parent ----
    // CompanionState.VoiceState enum value (int).
    property int currentVoiceState: CompanionState.Idle
    // Cursor + follow offset are passed in so OverlayContent owns the math
    // and MorphingCompanion doesn't recompute it independently.
    property real cursorPosX: 0
    property real cursorPosY: 0
    property real followOffsetPx: 22

    // ---- Tunables ----
    readonly property real passiveSizePx: 16
    readonly property real menuWidth: 320
    readonly property real menuMinHeight: 80
    readonly property real menuRadius: 14
    readonly property real menuMarginPx: 16
    readonly property int crossfadeDurationMs: 160
    readonly property int geometryDelayMs: 130     // ~80% of crossfade
    readonly property int geometryDurationMs: 220
    readonly property int followDurationMs: 80
    readonly property int contentFadeDurationMs: 200

    // ---- Derived state ----
    readonly property bool isMenuOpen: companionState.interactionMode === CompanionState.MenuOpen
    readonly property real anchorPosX: companionState.taskMenuAnchorPosition.x
    readonly property real anchorPosY: companionState.taskMenuAnchorPosition.y

    // Companion palette mirrors AgentDot primary mode.
    readonly property color idleColor: "#5aa9ff"
    readonly property color listeningColor: "#5aa9ff"
    readonly property color processingColor: "#ffc85a"
    readonly property color respondingColor: "#78dc82"
    readonly property color voiceStateColor:
        currentVoiceState === CompanionState.Listening  ? listeningColor :
        currentVoiceState === CompanionState.Processing ? processingColor :
        currentVoiceState === CompanionState.Responding ? respondingColor :
                                                          idleColor
    readonly property color companionDotColor:
        companionState.primaryFlashActive
            ? companionState.primaryFlashColor
            : voiceStateColor
    readonly property color menuBackgroundColor: "#16181c"

    // Passive target: top-left of the dot-sized footprint at (cursor + offset).
    // Use `passiveSizePx` rather than the live `width` so during the close
    // morph (while width is still 320) the target doesn't fly far to the
    // left as if the still-big rectangle were being re-centered.
    readonly property real cursorFollowX: cursorPosX + followOffsetPx - passiveSizePx / 2
    readonly property real cursorFollowY: cursorPosY + followOffsetPx - passiveSizePx / 2

    // Menu target: top-left fixed at the dot's pre-press top-left so the
    // card grows out to the right + down rather than re-centering on a new
    // point.
    readonly property real overlayWidth: parent ? parent.width : 0
    readonly property real overlayHeight: parent ? parent.height : 0
    readonly property real dotTopLeftX: anchorPosX + followOffsetPx - passiveSizePx / 2
    readonly property real dotTopLeftY: anchorPosY + followOffsetPx - passiveSizePx / 2
    readonly property real clampedAnchorX: Math.max(
        menuMarginPx,
        Math.min(dotTopLeftX, overlayWidth - menuWidth - menuMarginPx))
    readonly property real clampedAnchorY: Math.max(
        menuMarginPx,
        Math.min(dotTopLeftY, overlayHeight - height - menuMarginPx))

    // ---- Morph-in-progress flag (for position duration switching) ----
    property bool morphInProgress: false
    onIsMenuOpenChanged: {
        morphInProgress = true;
        morphCompletionTimer.restart();
    }
    Timer {
        id: morphCompletionTimer
        interval: morphRoot.crossfadeDurationMs
                + morphRoot.geometryDelayMs
                + morphRoot.geometryDurationMs
                + 40
        repeat: false
        onTriggered: morphRoot.morphInProgress = false
    }

    // ---- Visibility crossfade ----
    // Root is invisible in Passive (AgentDot below is what the user sees);
    // fades to fully opaque in MenuOpen.
    opacity: isMenuOpen ? 1.0 : 0.0
    Behavior on opacity {
        NumberAnimation {
            duration: morphRoot.crossfadeDurationMs
            easing.type: Easing.InOutQuad
        }
    }
    visible: opacity > 0.01

    // ---- Geometry (delayed start so the crossfade reads first) ----
    width:  isMenuOpen ? menuWidth  : passiveSizePx
    height: isMenuOpen ? Math.max(menuMinHeight, taskListContent.implicitContentHeight)
                       : passiveSizePx
    radius: isMenuOpen ? menuRadius : passiveSizePx / 2
    Behavior on width {
        SequentialAnimation {
            PauseAnimation { duration: morphRoot.geometryDelayMs }
            NumberAnimation { duration: morphRoot.geometryDurationMs; easing.type: Easing.OutCubic }
        }
    }
    Behavior on height {
        SequentialAnimation {
            PauseAnimation { duration: morphRoot.geometryDelayMs }
            NumberAnimation { duration: morphRoot.geometryDurationMs; easing.type: Easing.OutCubic }
        }
    }
    Behavior on radius {
        SequentialAnimation {
            PauseAnimation { duration: morphRoot.geometryDelayMs }
            NumberAnimation { duration: morphRoot.geometryDurationMs; easing.type: Easing.OutCubic }
        }
    }

    // ---- Color (delayed start so it stays blue during the crossfade) ----
    color: isMenuOpen ? menuBackgroundColor : companionDotColor
    Behavior on color {
        SequentialAnimation {
            PauseAnimation { duration: morphRoot.geometryDelayMs }
            ColorAnimation { duration: morphRoot.geometryDurationMs }
        }
    }

    border.color: "#33373f"
    border.width: isMenuOpen ? 1 : 0
    Behavior on border.width {
        SequentialAnimation {
            PauseAnimation { duration: morphRoot.geometryDelayMs }
            NumberAnimation { duration: morphRoot.geometryDurationMs }
        }
    }

    // ---- Position ----
    x: isMenuOpen ? clampedAnchorX : cursorFollowX
    y: isMenuOpen ? clampedAnchorY : cursorFollowY
    Behavior on x {
        NumberAnimation {
            duration: morphRoot.morphInProgress
                ? morphRoot.geometryDurationMs
                : morphRoot.followDurationMs
            easing.type: morphRoot.morphInProgress ? Easing.OutCubic : Easing.OutQuad
        }
    }
    Behavior on y {
        NumberAnimation {
            duration: morphRoot.morphInProgress
                ? morphRoot.geometryDurationMs
                : morphRoot.followDurationMs
            easing.type: morphRoot.morphInProgress ? Easing.OutCubic : Easing.OutQuad
        }
    }

    // ---- Drop shadow (menu shape only) ----
    // No layer effect / drop shadow. Qt's MultiEffect shadow at the menu's
    // size clipped at the layer FBO edge, showing a visible boundary on
    // lighter desktops. Border + dark gradient provides enough definition;
    // we can revisit a sibling-based soft shadow if needed.
    layer.enabled: false

    // ---- Task list view (MenuOpen only) ----
    TaskListContent {
        id: taskListContent
        anchors.fill: parent
        opacity: morphRoot.isMenuOpen ? 1.0 : 0.0
        visible: opacity > 0.01
        Behavior on opacity {
            NumberAnimation {
                duration: morphRoot.contentFadeDurationMs
                easing.type: Easing.InOutQuad
            }
        }
    }
}
