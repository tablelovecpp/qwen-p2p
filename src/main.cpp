#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <iostream>

#include "core/application.hpp"

int main(int argc, char *argv[])
{
    // Set environment variable for Qt Quick Controls 2 style
    qputenv("QT_QUICK_CONTROLS_STYLE", "Material");
    
    QGuiApplication app(argc, argv);
    app.setApplicationName("P2P Transfer");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("P2PTransfer");
    
    // Set Material style as default
    QQuickStyle::setStyle("Material");
    
    // Create and initialize application
    p2p::Application p2pApp;
    
    if (!p2pApp.initialize(argc, argv)) {
        std::cerr << "Failed to initialize application" << std::endl;
        return -1;
    }
    
    // Start listening on default port
    p2pApp.startListening(47473);
    
    return p2pApp.run();
}
