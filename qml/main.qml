import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

ApplicationWindow {
    id: window
    visible: true
    width: 1024
    height: 720
    minimumWidth: 800
    minimumHeight: 600
    title: qsTr("P2P File Transfer") + " - " + application.version
    
    color: palette.window
    
    header: ToolBar {
        Material.background: palette.base
        Material.foreground: palette.text
        
        RowLayout {
            anchors.fill: parent
            spacing: 10
            
            Label {
                text: "📤 P2P Transfer"
                font.pixelSize: 20
                font.bold: true
                Material.foreground: palette.highlight
            }
            
            Item { Layout.fillWidth: true }
            
            // Status indicators
            RowLayout {
                spacing: 15
                
                RowLayout {
                    Label {
                        text: "●"
                        color: application.isRunning ? "#4CAF50" : "#F44336"
                        font.pixelSize: 16
                    }
                    Label {
                        text: application.isRunning ? qsTr("Online") : qsTr("Offline")
                        font.pixelSize: 12
                    }
                }
                
                RowLayout {
                    Label {
                        text: "👥"
                        font.pixelSize: 14
                    }
                    Label {
                        text: application.connectedPeers
                        font.pixelSize: 12
                    }
                }
                
                RowLayout {
                    Label {
                        text: "⬆"
                        font.pixelSize: 12
                    }
                    Label {
                        text: mainView.formatSpeed(application.totalUploadSpeed)
                        font.pixelSize: 12
                    }
                }
                
                RowLayout {
                    Label {
                        text: "⬇"
                        font.pixelSize: 12
                    }
                    Label {
                        text: mainView.formatSpeed(application.totalDownloadSpeed)
                        font.pixelSize: 12
                    }
                }
            }
            
            ToolButton {
                icon.source: "qrc:/icons/settings.svg"
                icon.color: palette.text
                onClicked: settingsPopup.open()
            }
        }
    }
    
    footer: StatusBar {}
    
    StackLayout {
        anchors.fill: parent
        currentIndex: tabView.currentIndex
        
        SwipeView {
            id: tabView
            anchors.fill: parent
            
            // Peers Page
            Item {
                StackLayout.title: qsTr("Peers")
                StackLayout.icon: "qrc:/icons/peers.svg"
                
                PeerList {
                    anchors.fill: parent
                    anchors.margins: 10
                }
            }
            
            // Transfers Page
            Item {
                StackLayout.title: qsTr("Transfers")
                StackLayout.icon: "qrc:/icons/transfers.svg"
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10
                    
                    // Active transfers
                    GroupBox {
                        title: qsTr("Active Transfers")
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        
                        ListView {
                            id: transferListView
                            anchors.fill: parent
                            clip: true
                            model: application.activeTransfers
                            
                            delegate: FileTransferItem {
                                width: transferListView.width
                            }
                            
                            SectionScrollIndicator.visible: true
                            
                            Label {
                                anchors.centerIn: parent
                                text: qsTr("No active transfers")
                                visible: transferListView.count === 0
                                color: palette.midText
                            }
                        }
                    }
                    
                    // Quick actions
                    RowLayout {
                        Layout.fillWidth: true
                        
                        Button {
                            text: "📁 " + qsTr("Send File")
                            Layout.fillWidth: true
                            highlighted: true
                            onClicked: {
                                var files = mainView.openFileDialog()
                                for (var i = 0; i < files.length; i++) {
                                    // Send to selected peer
                                    console.log("Sending file:", files[i])
                                }
                            }
                        }
                        
                        Button {
                            text: "📂 " + qsTr("Share Folder")
                            Layout.fillWidth: true
                            onClicked: {
                                var folder = mainView.openFolderDialog()
                                if (folder) {
                                    console.log("Sharing folder:", folder)
                                }
                            }
                        }
                    }
                }
            }
            
            // Settings Page
            Item {
                StackLayout.title: qsTr("Settings")
                StackLayout.icon: "qrc:/icons/settings.svg"
                
                ScrollView {
                    anchors.fill: parent
                    anchors.margins: 10
                    
                    ColumnLayout {
                        width: parent.width
                        spacing: 15
                        
                        GroupBox {
                            title: qsTr("Network Settings")
                            Layout.fillWidth: true
                            
                            ColumnLayout {
                                anchors.fill: parent
                                spacing: 10
                                
                                RowLayout {
                                    Label {
                                        text: qsTr("Listen Port:")
                                        Layout.preferredWidth: 120
                                    }
                                    TextField {
                                        text: "47473"
                                        Layout.fillWidth: true
                                        validator: IntValidator { bottom: 1024; top: 65535 }
                                    }
                                }
                                
                                RowLayout {
                                    Label {
                                        text: qsTr("Discovery:")
                                        Layout.preferredWidth: 120
                                    }
                                    CheckBox {
                                        text: qsTr("Enable UDP Discovery")
                                        checked: true
                                    }
                                }
                                
                                RowLayout {
                                    Label {
                                        text: qsTr("Multicast:")
                                        Layout.preferredWidth: 120
                                    }
                                    CheckBox {
                                        text: qsTr("Use Multicast")
                                        enabled: false
                                    }
                                }
                            }
                        }
                        
                        GroupBox {
                            title: qsTr("Transfer Settings")
                            Layout.fillWidth: true
                            
                            ColumnLayout {
                                anchors.fill: parent
                                spacing: 10
                                
                                RowLayout {
                                    Label {
                                        text: qsTr("Download Location:")
                                        Layout.preferredWidth: 120
                                    }
                                    TextField {
                                        text: "~/Downloads/P2PTransfer"
                                        Layout.fillWidth: true
                                        readOnly: true
                                    }
                                    Button {
                                        text: "..."
                                        onClicked: mainView.openFolderDialog()
                                    }
                                }
                                
                                RowLayout {
                                    Label {
                                        text: qsTr("Max Connections:")
                                        Layout.preferredWidth: 120
                                    }
                                    SpinBox {
                                        from: 1
                                        to: 50
                                        value: 10
                                    }
                                }
                                
                                RowLayout {
                                    Label {
                                        text: qsTr("Transfer Limit:")
                                        Layout.preferredWidth: 120
                                    }
                                    ComboBox {
                                        model: ["Unlimited", "1 MB/s", "5 MB/s", "10 MB/s"]
                                        currentIndex: 0
                                    }
                                }
                            }
                        }
                        
                        GroupBox {
                            title: qsTr("Appearance")
                            Layout.fillWidth: true
                            
                            ColumnLayout {
                                anchors.fill: parent
                                spacing: 10
                                
                                RowLayout {
                                    Label {
                                        text: qsTr("Theme:")
                                        Layout.preferredWidth: 120
                                    }
                                    ComboBox {
                                        model: ["System", "Light", "Dark"]
                                        currentIndex: 0
                                        onCurrentTextChanged: {
                                            // Theme change logic
                                        }
                                    }
                                }
                            }
                        }
                        
                        Label {
                            text: qsTr("P2P Transfer v") + application.version
                            horizontalAlignment: Text.AlignHCenter
                            Layout.fillWidth: true
                            color: palette.midText
                            font.pixelSize: 11
                        }
                    }
                }
            }
        }
    }
    
    // Settings popup
    Popup {
        id: settingsPopup
        modal: true
        focus: true
        x: (window.width - width) / 2
        y: (window.height - height) / 2
        width: Math.min(400, window.width - 40)
        
        contentItem: ColumnLayout {
            spacing: 15
            
            Label {
                text: qsTr("Settings")
                font.pixelSize: 18
                font.bold: true
            }
            
            // Add settings content here
            
            DialogButtonBox {
                Layout.fillWidth: true
                
                Button {
                    text: qsTr("Close")
                    DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
                    onClicked: settingsPopup.close()
                }
            }
        }
    }
}
