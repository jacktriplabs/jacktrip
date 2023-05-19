import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.12
import VS 1.0

Item {
    width: parent.width; height: parent.height
    clip: true

    state: auth.authenticationStage
    states: [
        State {
            name: "unauthenticated"
        },
        State {
            name: "polling"
        },
        State {
            name: "success"
        },
        State {
            name: "failed"
        }
    ]

    Rectangle {
        width: parent.width; height: parent.height
        color: backgroundColour
    }

    property bool showBackButton: true
    property bool codeCopied: false

    property string backgroundColour: virtualstudio.darkMode ? "#272525" : "#FAFBFB"
    property string textColour: virtualstudio.darkMode ? "#FAFBFB" : "#0F0D0D"
    property string buttonColour: virtualstudio.darkMode ? "#FAFBFB" : "#F0F1F1"
    property string buttonHoverColour: virtualstudio.darkMode ? "#E9E9E9" : "#E4E5E5"
    property string buttonPressedColour: virtualstudio.darkMode ? "#FAFBFB" : "#E4E5E5"
    property string buttonStroke: virtualstudio.darkMode ? "#80827D7D" : "#34979797"
    property string buttonHoverStroke: virtualstudio.darkMode ? "#6F6C6C" : "#B0B5B5"
    property string buttonPressedStroke: virtualstudio.darkMode ? "#6F6C6C" : "#B0B5B5"
    property string buttonTextColour: virtualstudio.darkMode ? "#272525" : "#DB0A0A"
    property string buttonTextHover: virtualstudio.darkMode ? "#242222" : "#D00A0A"
    property string buttonTextPressed: virtualstudio.darkMode ? "#323030" : "#D00A0A"
    property string shadowColour: virtualstudio.darkMode ? "40000000" : "#80A1A1A1"
    property string linkTextColour: virtualstudio.darkMode ? "#8B8D8D" : "#272525"
    property string toolTipTextColour: codeCopied ? "#FAFBFB" : textColour
    property string toolTipBackgroundColour: codeCopied ? "#57B147" : (virtualstudio.darkMode ? "#323232" : "#F3F3F3")
    property string tooltipStroke: virtualstudio.darkMode ? "#80827D7D" : "#34979797"
    property string disabledButtonText: "#D3D4D4"

    Clipboard {
        id: clipboard
    }

    Image {
        id: loginLogo
        source: "logo.svg"
        x: parent.width / 2 - (150 * virtualstudio.uiScale); y: 110 * virtualstudio.uiScale
        width: 42 * virtualstudio.uiScale; height: 76 * virtualstudio.uiScale
        sourceSize: Qt.size(loginLogo.width,loginLogo.height)
        fillMode: Image.PreserveAspectFit
        smooth: true
        visible: loginScreen.state === "unauthenticated"
    }

    Image {
        source: virtualstudio.darkMode ? "jacktrip white.png" : "jacktrip.png"
        anchors.bottom: loginLogo.bottom
        x: parent.width / 2 - (88 * virtualstudio.uiScale)
        width: 238 * virtualstudio.uiScale; height: 56 * virtualstudio.uiScale
        visible: loginScreen.state === "unauthenticated" || loginScreen.state === "failed"
    }

    Text {
        text: "Virtual Studio"
        font.family: "Poppins"
        font.pixelSize: 28 * virtualstudio.fontScale * virtualstudio.uiScale
        anchors.horizontalCenter: parent.horizontalCenter
        y: 208 * virtualstudio.uiScale
        color: textColour
        visible: loginScreen.state === "unauthenticated" || loginScreen.state === "failed"
    }

    Text {
        id: loggingInText
        text: "Logging in..."
        font.family: "Poppins"
        font.pixelSize: 18 * virtualstudio.fontScale * virtualstudio.uiScale
        anchors.horizontalCenter: parent.horizontalCenter
        y: 282 * virtualstudio.uiScale
        visible: loginScreen.state === "unauthenticated" && virtualstudio.hasRefreshToken
        color: textColour
    }

    Text {
        id: authSucceededText
        text: "Success!"
        font.family: "Poppins"
        font.pixelSize: 18 * virtualstudio.fontScale * virtualstudio.uiScale
        anchors.horizontalCenter: parent.horizontalCenter
        y: 348 * virtualstudio.uiScale
        visible: loginScreen.state === "success"
        color: textColour
    }

    Text {
        id: authFailedText
        text: "Log in failed. Please try again."
        font.family: "Poppins"
        font.pixelSize: 18 * virtualstudio.fontScale * virtualstudio.uiScale
        anchors.horizontalCenter: parent.horizontalCenter
        y: 282 * virtualstudio.uiScale
        visible: loginScreen.state === "failed"
        color: textColour
    }

    Image {
        id: successIcon
        source: "check.svg"
        y: 240 * virtualstudio.uiScale
        anchors.horizontalCenter: parent.horizontalCenter
        visible: loginScreen.state === "success"
        sourceSize: Qt.size(96 * virtualstudio.uiScale, 96 * virtualstudio.uiScale)
        fillMode: Image.PreserveAspectFit
        smooth: true
    }

    Colorize {
        anchors.fill: successIcon
        source: successIcon
        hue: .44
        saturation: .55
        lightness: .49
        visible: loginScreen.state === "success"
    }

    Text {
        id: deviceVerificationHeader
        text: "Authorize Application"
        font.family: "Poppins"
        font.pixelSize: 20 * virtualstudio.fontScale * virtualstudio.uiScale
        anchors.horizontalCenter: parent.horizontalCenter
        y: 88 * virtualstudio.uiScale
        visible: loginScreen.state === "polling"
        color: textColour
        horizontalAlignment: Text.AlignHCenter
    }

    Text {
        id: deviceVerificationExplanation
        text: `To log in to your Virtual Studio account, please enter the following one-time code at `
            + `<a style="color: ${linkTextColour}" href='${auth.verificationUrl}'><u><b>${
                    ((completeUrl) => {
                        if (completeUrl.indexOf("?") === -1) {
                            return completeUrl;
                        }
                        return completeUrl.substring(0, auth.verificationUrl.indexOf("?"))
                    })(auth.verificationUrl)
                }</b></u></a>`
            + ` and sign in through your browser.`
        font.family: "Poppins"
        font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
        anchors.horizontalCenter: parent.horizontalCenter
        y: 160 * virtualstudio.uiScale
        width: 500 * virtualstudio.uiScale;
        visible: loginScreen.state === "polling"
        color: textColour
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
        textFormat: Text.RichText
        onLinkActivated: link => {
            if (!Boolean(auth.verificationCode)) {
                return;
            }
            virtualstudio.openLink(link)
        }
    }

    Text {
        id: deviceVerificationCode
        text: auth.verificationCode || "Loading...";
        font.family: "Poppins"
        font.pixelSize: 20 * virtualstudio.fontScale * virtualstudio.uiScale
        font.letterSpacing: Boolean(auth.verificationCode) ? 8 : 1
        anchors.horizontalCenter: parent.horizontalCenter
        y: 280 * virtualstudio.uiScale
        width: 360 * virtualstudio.uiScale;
        visible: loginScreen.state === "polling"
        color: Boolean(auth.verificationCode) ? textColour : disabledButtonText
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter

        Timer {
            id: copiedResetTimer
            interval: 2000; running: false; repeat: false
            onTriggered: codeCopied = false;
        }

        MouseArea {
            id: deviceVerificationCodeMouseArea
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            hoverEnabled: true
            onClicked: () => {
                codeCopied = true;
                clipboard.setText(auth.verificationCode);
                copiedResetTimer.restart()
            }
        }

        ToolTip {
            parent: deviceVerificationCode
            visible: loginScreen.state === "polling" && deviceVerificationCodeMouseArea.containsMouse
            delay: 100
            contentItem: Rectangle {
                color: toolTipBackgroundColour
                radius: 3
                anchors.fill: parent
                layer.enabled: true
                border.width: 1
                border.color: tooltipStroke

                Text {
                    anchors.centerIn: parent
                    font { family: "Poppins"; pixelSize: 8 * virtualstudio.fontScale * virtualstudio.uiScale}
                    text: codeCopied ? qsTr("📋 Copied code to clipboard") : qsTr("📋 Copy code to Clipboard")
                    color: toolTipTextColour
                }
            }
            background: Rectangle {
                color: "transparent"
            }
        }
    }

    Text {
        id: deviceVerificationFollowUp
        text: "Once you've authorized this application, you'll automatically be moved to the next screen."
        font.family: "Poppins"
        font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
        anchors.horizontalCenter: parent.horizontalCenter
        y: 475 * virtualstudio.uiScale
        width: 500 * virtualstudio.uiScale;
        visible: loginScreen.state === "polling"
        color: textColour
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
    }

    Text {
        id: cancelDeviceVerification
        text: "Cancel"
        font.family: "Poppins"
        font.pixelSize: 11 * virtualstudio.fontScale * virtualstudio.uiScale
        font.underline: true;
        anchors.horizontalCenter: parent.horizontalCenter
        y: 535 * virtualstudio.uiScale
        visible: loginScreen.state === "polling"
        color: textColour
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
        
        MouseArea {
            anchors.fill: parent
            onClicked: () => {
                auth.cancelAuthenticationFlow();
            }
            cursorShape: Qt.PointingHandCursor
        }
    }

    Button {
        id: loginButton
        background: Rectangle {
            radius: 6 * virtualstudio.uiScale
            color: loginButton.down ? buttonPressedColour : (loginButton.hovered ? buttonHoverColour : buttonColour)
            border.width: 1
            border.color: loginButton.down ? buttonPressedStroke : (loginButton.hovered ? buttonHoverStroke : buttonStroke)
            layer.enabled: !loginButton.down
        }
        onClicked: {
            loginScreen.state = "polling"; // Hack - transition to 'polling' before auth state actually changes for improved UX
            virtualstudio.showFirstRun = false;
            virtualstudio.login()
        }
        anchors.horizontalCenter: parent.horizontalCenter
        y: showBackButton ? 321 * virtualstudio.uiScale : 371 * virtualstudio.uiScale
        width: 263 * virtualstudio.uiScale; height: 64 * virtualstudio.uiScale
        Text {
            text: "Sign In"
            font.family: "Poppins"
            font.pixelSize: 18 * virtualstudio.fontScale * virtualstudio.uiScale
            font.weight: Font.Bold
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            color: loginButton.down ? buttonTextPressed : (loginButton.hovered ? buttonTextHover : buttonTextColour)
        }
        visible: (!virtualstudio.hasRefreshToken && loginScreen.state === "unauthenticated") || loginScreen.state === "failed"
    }

    Button {
        id: backButton
        visible: (!virtualstudio.hasRefreshToken && loginScreen.state === "unauthenticated") || loginScreen.state === "failed"
        background: Rectangle {
            radius: 6 * virtualstudio.uiScale
            color: backButton.down ? buttonPressedColour : (backButton.hovered ? buttonHoverColour : buttonColour)
            border.width: 1
            border.color: backButton.down ? buttonPressedStroke : (backButton.hovered ? buttonHoverStroke : buttonStroke)
            layer.enabled: !backButton.down
        }
        onClicked: { virtualstudio.windowState = "start" }
        anchors.horizontalCenter: parent.horizontalCenter
        y: 401 * virtualstudio.uiScale
        width: 263 * virtualstudio.uiScale; height: 64 * virtualstudio.uiScale
        Text {
            text: "Back"
            font.family: "Poppins"
            font.pixelSize: 18 * virtualstudio.fontScale * virtualstudio.uiScale
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            color: backButton.down ? buttonTextPressed : (backButton.hovered ? buttonTextHover : buttonTextColour)
        }
    }

    Button {
        id: openVerificationUrlButton
        visible: loginScreen.state === "polling"
        enabled: Boolean(auth.verificationCode)
        background: Rectangle {
            radius: 6 * virtualstudio.uiScale
            color: openVerificationUrlButton.down ? buttonPressedColour : (openVerificationUrlButton.hovered ? buttonHoverColour : buttonColour)
            border.width: 1
            border.color: openVerificationUrlButton.down ? buttonPressedStroke : (openVerificationUrlButton.hovered ? buttonHoverStroke : buttonStroke)
            layer.enabled: !openVerificationUrlButton.down
        }
        onClicked: { virtualstudio.openLink(auth.verificationUrl); }
        anchors.horizontalCenter: parent.horizontalCenter
        y: 370 * virtualstudio.uiScale
        width: 216 * virtualstudio.uiScale; height: 48 * virtualstudio.uiScale
        Text {
            text: "Open Browser"
            font.family: "Poppins"
            font.pixelSize: 12 * virtualstudio.fontScale * virtualstudio.uiScale
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            color: !Boolean(auth.verificationCode)
                ? disabledButtonText
                : openVerificationUrlButton.down ? buttonTextPressed : (openVerificationUrlButton.hovered ? buttonTextHover : buttonTextColour)
        }
    }

    Button {
        id: classicModeButton
        visible: !showBackButton && virtualstudio.showFirstRun && virtualstudio.vsFtux
        background: Rectangle {
            radius: 6 * virtualstudio.uiScale
            color: classicModeButton.down ? buttonPressedColour : (classicModeButton.hovered ? buttonHoverColour : backgroundColour)
            border.width: 0
            layer.enabled: !classicModeButton.down
        }
        onClicked: { virtualstudio.windowState = "login"; virtualstudio.toStandard(); }
        anchors.horizontalCenter: parent.horizontalCenter
        y: 600 * virtualstudio.uiScale
        width: 160 * virtualstudio.uiScale; height: 32 * virtualstudio.uiScale
        Text {
            text: "Use Classic Mode"
            font.family: "Poppins"
            font.pixelSize: 9 * virtualstudio.fontScale * virtualstudio.uiScale
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            color: classicModeButton.down ? buttonTextPressed : (classicModeButton.hovered ? buttonTextHover : textColour)
        }
    }

    Connections {
        target: auth
        function onUpdatedAuthenticationStage (stage) {
            loginScreen.state = stage;
        }
    }
}
