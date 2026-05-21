import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    
    property var peers: application.discoveredPeers
    
    GroupBox {
        anchors.fill: parent
        title: qsTr("Discovered Peers")
        
        ListView {
            id: peerListView
            anchors.fill: parent
            clip: true
            model: root.peers
            
            delegate: Item {
                width: peerListView.width
                height: 60
                
                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    spacing: 10
                    
                    // Status indicator
                    Rectangle {
                        width: 12
                        height: 12
                        radius: 6
                        color: model.isOnline ? "#4CAF50" : "#9E9E9E"
                        
                        Label {
                            anchors.centerIn: parent
                            text: "●"
                            font.pixelSize: 16
                            color: parent.color
                            opacity: 0
                        }
                    }
                    
                    // Peer info
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        
                        Label {
                            text: model.name || model.address
                            font.pixelSize: 14
                            font.bold: true
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                        
                        Label {
                            text: model.address + ":" + model.port
                            font.pixelSize: 11
                            color: palette.midText
                        }
                    }
                    
                    // Connection status
                    Label {
                        text: model.isOnline ? qsTr("Online") : qsTr("Offline")
                        font.pixelSize: 11
                        color: model.isOnline ? "#4CAF50" : "#9E9E9E"
                    }
                    
                    // Actions
                    RowLayout {
                        spacing: 5
                        
                        Button {
                            text: "📤"
                            flat: true
                            ToolTip.text: qsTr("Send File")
                            ToolTip.visible: hovered
                            onClicked: {
                                // Send file to this peer
                                console.log("Send file to:", model.id)
                            }
                        }
                        
                        Button {
                            text: "📥"
                            flat: true
                            ToolTip.text: qsTr("Request File")
                            ToolTip.visible: hovered
                            onClicked: {
                                // Request file from this peer
                                console.log("Request file from:", model.id)
                            }
                        }
                        
                        Button {
                            text: "❌"
                            flat: true
                            ToolTip.text: qsTr("Disconnect")
                            ToolTip.visible: hovered
                            visible: model.isOnline
                            onClicked: {
                                application.disconnectFromPeer(model.id)
                            }
                        }
                    }
                }
                
                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 1
                    color: palette.mid
                }
            }
            
            Label {
                anchors.centerIn: parent
                text: qsTr("No peers discovered\nMake sure you're on the same network")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                visible: peerListView.count === 0
                color: palette.midText
                font.pixelSize: 13
            }
        }
    }
}
