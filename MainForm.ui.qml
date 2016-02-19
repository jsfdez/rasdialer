import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2

Item {
    id: item1
    width: 800
    height: 400

    property alias model: tableView.model
    property string timeRemaining: "30 minutes"
    property alias timeToChangeVpn: timeSpinBox.value
    property alias randomVpn: randomVpnCheckBox.checked
    property alias checkVpn: checkVpnCheckBox.checked
    property alias failedTimeouts: failedTimeoutsSpinBox.value
    property alias tableView: tableView
    property alias startButton: startButton
    property alias saveSettingsButton: saveSettingsButton
    property alias pingHost: pingHostField.text
    property alias pingRetries: failedTimeoutsSpinBox.value
    property bool running: false

    Label {
        id: tableViewLabel
        x: 8
        y: 8
        width: 292
        height: 13
        text: "PPTP server list (username:password@domain)"
        font.underline: true
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
    }

    TableView {
        id: tableView
        anchors.top: tableViewLabel.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        width: 300
        selectionMode: 0
        anchors.topMargin: 6
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff

        model: ListModel {
            id: model
            ListElement {
                address: "67.212.175.123"
                username: "freevpnaccess.com"
                password: "1509"
                ping: 50
            }
            ListElement {
                address: "83.170.84.216"
                username: "freevpnaccess.com"
                password: "1091"
                ping: 50
            }
            ListElement {
                address: "184.107.164.107"
                username: "freevpnaccess.com"
                password: "9727"
                ping: 50
            }
        }

        TableViewColumn {
            role: "combined"
            title: "Server"
            width: 225
            resizable: false
        }

        TableViewColumn {
            role: "address"
            title: "Address"
            width: 100
            resizable: false
            visible: false
        }

        TableViewColumn {
            role: "username"
            title: "User name"
            width: 100
            resizable: false
            visible: false
        }

        TableViewColumn {
            role: "password"
            title: "Password"
            width: 100
            resizable: false
            visible: false
        }

        TableViewColumn {
            role: "ping"
            title: "Ping"
            width: 75
            resizable: false
        }
    }

    Label {
        id: label1
        x: 327
        y: 8
        text: qsTr("Options")
        font.underline: true
        font.bold: true
    }

    CheckBox {
        id: checkVpnCheckBox
        x: 352
        y: 189
        width: 215
        height: 17
        text: qsTr("Check if VPN connection works by  PING")
        enabled: !running
        anchors.right: parent.right
        anchors.rightMargin: 233
    }

    CheckBox {
        id: randomVpnCheckBox
        x: 352
        y: 164
        width: 440
        height: 17
        text: qsTr("Select next VPN randomly")
        enabled: !running
        anchors.right: parent.right
        anchors.rightMargin: 8
    }

    SpinBox {
        id: timeSpinBox
        x: 593
        y: 37
        width: 199
        height: 20
        enabled: !running
        value: 30
        anchors.right: parent.right
        anchors.rightMargin: 8
        suffix: " minutes"
    }

    Label {
        id: label2
        x: 308
        y: 41
        width: 279
        height: 13
        text: qsTr("Time to switch between servers:")
        enabled: !running
        anchors.right: parent.right
        anchors.rightMargin: 213
        horizontalAlignment: Text.AlignRight
    }

    TextField {
        id: pingHostField
        x: 573
        y: 187
        width: 219
        height: 20
        text: "yahoo.com"
        enabled: !running && checkVpnCheckBox.checked
        anchors.right: parent.right
        anchors.rightMargin: 8
        placeholderText: qsTr("Text Field")
    }

    Label {
        id: label3
        x: 411
        y: 212
        width: 381
        height: 13
        text: "if no connection (time out) move to the next random server."
        enabled: !running
        anchors.right: parent.right
        anchors.rightMargin: 8
    }

    Label {
        id: label4
        x: 365
        y: 235
        width: 362
        height: 13
        text: qsTr("Number of continues Ping time outs before move to the next random VPN")
        enabled: !running
        anchors.right: parent.right
        anchors.rightMargin: 73
    }

    SpinBox {
        id: failedTimeoutsSpinBox
        x: 731
        y: 231
        width: 61
        height: 20
        enabled: !running && checkVpnCheckBox.checked
        anchors.right: parent.right
        anchors.rightMargin: 8
        minimumValue: 1
        value: 30
        maximumValue: 300
    }

    Button {
        id: saveSettingsButton
        x: 308
        y: 449
        width: 112
        height: 23
        text: qsTr("SAVE options")
        enabled: !running
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 8
    }

    Button {
        id: startButton
        x: 426
        y: 449
        text: qsTr("Start")
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 8
    }

    Label {
        id: label5
        y: 325
        height: 67
        text: qsTr("Time remains before move to the next random VPN:") + "<br/><b>" + timeRemaining + "</b>"
        textFormat: Text.RichText
        wrapMode: Text.WordWrap
        verticalAlignment: Text.AlignBottom
        anchors.left: parent.left
        anchors.leftMargin: 514
        anchors.right: parent.right
        anchors.rightMargin: 8
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 8
    }
}
