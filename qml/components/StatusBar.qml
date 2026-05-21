import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    
    implicitHeight: 30
    
    Rectangle {
        anchors.fill: parent
        color: palette.base
        
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 15
            anchors.rightMargin: 15
            spacing: 20
            
            // Connection status
            Label {
                text: "🌐 " + qsTr("Network Status:")
                font.pixelSize: 12
            }
            
            Label {
                text: application.isRunning ? qsTr("Connected") : qsTr("Disconnected")
                font.pixelSize: 12
                font.bold: true
                color: application.isRunning ? "#4CAF50" : "#F44336"
            }
            
            Separator {
                Layout.preferredWidth: 1
                Layout.preferredHeight: 16
            }
            
            // Listening port
            Label {
                text: "🔌 " + qsTr("Port:") + " " + (application.isRunning ? "47473" : "-")
                font.pixelSize: 12
            }
            
            Separator {
                Layout.preferredWidth: 1
                Layout.preferredHeight: 16
            }
            
            // Peers count
            Label {
                text: "👥 " + qsTr("Peers:") + " " + application.connectedPeers
                font.pixelSize: 12
            }
            
            Item { Layout.fillWidth: true }
            
            // Transfer speeds
            RowLayout {
                spacing: 15
                
                Label {
                    text: "⬆ " + mainView.formatSpeed(application.totalUploadSpeed)
                    font.pixelSize: 11
                    color: "#2196F3"
                }
                
                Label {
                    text: "⬇ " + mainView.formatSpeed(application.totalDownloadSpeed)
                    font.pixelSize: 11
                    color: "#4CAF50"
                }
            }
        }
    }
}
