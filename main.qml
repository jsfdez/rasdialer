import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import RasDialer 1.0
import Qt.labs.settings 1.0

ApplicationWindow {
    id: window
    property int secondsRemaing: mainForm.timeToChangeVpn * 60
    property int pingRetries
    property int nextVpnRow: 0;
    property string username
    property string password
    property string address

    visible: true
    width: 800
    height: 480

    maximumWidth: width
    maximumHeight: height
    minimumWidth: width
    minimumHeight: height

    property bool connected: false
    title: dialer.state === RasDialer.Connected ? "VPN PPTP Connection Manager V 1.0 - CONNECTED"
                                                : "VPN PPTP Connection Manager V 1.0 - DISCONNECTED"

    Component.onDestruction: dialer.disconnect();

    Ping {
        id: ping
        host: mainForm.pingHost

        onFinished: {
            if (ms < 0) {
                console.log("Ping: " + ms + "ms. " + pingRetries + " retries remaining");
                if (pingRetries > 0) {
                    ping.ping();
                    --pingRetries;
                } else {
                    connectToVpn();
                }
            } else {
                console.log("Ping: " + ms + "ms");
                restartTimer();
            }

        }
    }

    function restartTimer() {
        secondsRemaing = mainForm.timeToChangeVpn * 60
        timeRemainingTimer.start();
    }

    function connectToVpn() {
        dialer.disconnect();
        var row, rows = serverModel.rowCount();
        if (mainForm.randomVpn)
            row = Math.floor(Math.random() * rows);
        else
            row = nextVpnRow++;

        var index = serverModel.index(row, 0);
        address = serverModel.data(index, ServerModel.AddressRole);
        username = serverModel.data(index, ServerModel.UserNameRole);
        password = serverModel.data(index, ServerModel.PasswordRole);
        console.log("connecting to " + address + "...");
        dialer.connect(address, username, password);
    }

    Connections {
        target: dialer
        onStateChanged: {
            if (state === RasDialer.Connected) {
                if (mainForm.checkVpn){
                    pingRetries = mainForm.pingRetries
                    ping.ping();
                } else {
                    restartTimer();
                }
            }
        }
        onErrorReceived: {
            errorDialog.text = error;
            errorDialog.open();
        }
    }

    MessageDialog {
        id: errorDialog
        title: Qt.application.name
        standardButtons: StandardButton.Ok
        onAccepted: connectToVpn();
        icon: StandardIcon.Critical
    }

    Timer {
        id: timeRemainingTimer
        interval: 1000
        repeat: true
        onTriggered: {
            if (window.secondsRemaing === 0) {
                connectToVpn();
                timeRemainingTimer.stop();
            } else
                window.secondsRemaing--;

            var minutes = Math.floor(window.secondsRemaing / 60);
            var seconds = window.secondsRemaing % 60;
            var minutesString, secondsString;
            if (minutes == 1)
                minutesString = "1 minute";
            else if (minutes != 0)
                minutesString = minutes + " minutes";
            else
                minutesString = "";
            if (seconds == 1)
                secondsString = "1 second";
            else if (seconds != 0)
                secondsString = seconds + " seconds";
            else
                secondsString = "";
            mainForm.timeRemaining = minutesString + " " + secondsString;
        }
    }

    MainForm {
        id: mainForm
        enabled: dialer.state !== RasDialer.Connecting
        running: timeRemainingTimer.running
        timeToChangeVpn: settings.timeToChangeVpn;
        timeRemaining: timeToChangeVpn == 1 ? "1 minute" : timeToChangeVpn + " minutes"
        randomVpn: settings.randomVpn;
        checkVpn: settings.checkVpn;
        failedTimeouts: settings.failedTimeouts;
        pingHost: settings.pingHost;

        anchors.fill: parent
        model: serverModel

        startButton.text: timeRemainingTimer.running ? "Stop" : "Start"
        startButton.onClicked: {
            if (timeRemainingTimer.running) {
                timeRemainingTimer.stop();
                function binding() {
                    return mainForm.timeToChangeVpn * 60;
                }
                secondsRemaing = Qt.binding(binding);
                mainForm.timeRemaining = Qt.binding(function() {
                    return timeToChangeVpn == 1 ? "1 minute" : timeToChangeVpn + " minutes";
                });
            } else {
//                timeRemainingTimer.start();
                connectToVpn();
            }
        }

        saveSettingsButton.onClicked: {
            settings.timeToChangeVpn = mainForm.timeToChangeVpn;
            settings.randomVpn = mainForm.randomVpn;
            settings.checkVpn = mainForm.checkVpn;
            settings.failedTimeouts = mainForm.failedTimeouts;
            settings.pingHost = mainForm.pingHost;
        }
    }

    Settings {
        id: settings
        property int timeToChangeVpn: 30
        property bool randomVpn: true
        property bool checkVpn: true
        property int failedTimeouts: 30
        property string pingHost: "yahoo.com"
    }
}
