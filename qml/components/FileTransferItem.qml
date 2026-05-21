import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    
    property string transferId: model.transferId || ""
    property string fileName: model.fileName || qsTr("Unknown File")
    property int fileSize: model.fileSize || 0
    property int transferredBytes: model.transferredBytes || 0
    property real progress: model.progress || 0.0
    property int currentSpeed: model.currentSpeed || 0
    property bool isUpload: model.isUpload || false
    property string peerName: model.sourcePeerId || model.destPeerId || ""
    
    height: 80
    
    Rectangle {
        anchors.fill: parent
        radius: 5
        color: palette.alternateBase
        border.color: palette.mid
        border.width: 1
        
        RowLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 10
            
            // File icon
            Rectangle {
                width: 50
                height: 50
                radius: 8
                color: isUpload ? "#2196F3" : "#4CAF50"
                
                Label {
                    anchors.centerIn: parent
                    text: isUpload ? "📤" : "📥"
                    font.pixelSize: 24
                }
            }
            
            // File info and progress
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 5
                
                // File name
                Label {
                    text: fileName
                    font.pixelSize: 13
                    font.bold: true
                    elide: Text.ElideMiddle
                    Layout.fillWidth: true
                }
                
                // Progress bar
                ProgressBar {
                    id: progressBar
                    value: progress
                    from: 0
                    to: 1
                    Layout.fillWidth: true
                    height: 8
                    
                    background: Rectangle {
                        implicitWidth: 200
                        implicitHeight: 8
                        color: palette.mid
                        radius: 4
                    }
                    
                    contentItem: Item {
                        implicitWidth: 200
                        implicitHeight: progressBar.height
                        
                        Rectangle {
                            width: progressBar.visualPosition * parent.width
                            height: parent.height
                            radius: 4
                            color: progressBar.value >= 1.0 ? "#4CAF50" : "#2196F3"
                            
                            Behavior on width {
                                NumberAnimation { duration: 200 }
                            }
                        }
                    }
                }
                
                // Status info
                RowLayout {
                    Layout.fillWidth: true
                    
                    Label {
                        text: mainView.formatFileSize(transferredBytes) + " / " + 
                              mainView.formatFileSize(fileSize)
                        font.pixelSize: 11
                        color: palette.midText
                    }
                    
                    Item { Layout.fillWidth: true }
                    
                    Label {
                        text: (progress * 100).toFixed(1) + "%"
                        font.pixelSize: 11
                        font.bold: true
                        color: palette.highlight
                    }
                }
            }
            
            // Speed and actions
            ColumnLayout {
                spacing: 5
                horizontalAlignment: Qt.AlignHCenter
                
                // Speed
                Label {
                    text: mainView.formatSpeed(currentSpeed)
                    font.pixelSize: 11
                    color: palette.midText
                }
                
                // ETA (if we had it)
                Label {
                    text: "--:--"
                    font.pixelSize: 10
                    color: palette.midText
                }
                
                // Cancel button
                Button {
                    text: "❌"
                    flat: true
                    padding: 5
                    ToolTip.text: qsTr("Cancel Transfer")
                    ToolTip.visible: hovered
                    onClicked: {
                        if (transferId) {
                            application.cancelTransfer(transferId)
                        }
                    }
                }
            }
        }
        
        // Completion overlay
        Rectangle {
            anchors.fill: parent
            visible: progress >= 1.0
            color: "#80000000"
            radius: 5
            
            Label {
                anchors.centerIn: parent
                text: "✅ " + qsTr("Complete")
                font.pixelSize: 16
                font.bold: true
                color: "#FFFFFF"
            }
        }
    }
}
