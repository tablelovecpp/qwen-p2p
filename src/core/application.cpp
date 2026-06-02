#include "core/application.hpp"
#include "core/filemanager.hpp"
#include "network/p2pengine.hpp"
#include "network/discovery.hpp"
#include "ui/mainview.hpp"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDir>
#include <QUuid>
#include <QStandardPaths>
#include <QHostInfo>

#ifdef QT_QUICKCONTROLS_LIB
#include <QtQuickControls2/QQuickStyle>
#endif

namespace p2p {

Application::Application(QObject* parent)
    : QObject(parent)
    , m_engine(std::make_unique<QQmlApplicationEngine>())
    , m_p2pEngine(std::make_unique<P2PEngine>())
    , m_fileManager(std::make_unique<FileManager>())
    , m_discovery(std::make_unique<Discovery>())
{
}

Application::~Application() = default;

bool Application::initialize(int argc, char* argv[])
{
    // Set application attributes for high DPI support
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    
    // Register QML types
    registerQmlTypes();
    
    // Setup engine
    setupQmlEngine();
    
    // Connect signals
    connectSignals();
    
    m_isRunning = true;
    emit runningChanged();
    
    return true;
}

int Application::run()
{
    if (!m_engine->rootObjects().isEmpty()) {
        return QGuiApplication::exec();
    }
    return -1;
}

int Application::connectedPeersCount() const
{
    return m_p2pEngine ? m_p2pEngine->connectedPeerCount() : 0;
}

qint64 Application::totalUploadSpeed() const
{
    return m_p2pEngine ? m_p2pEngine->uploadSpeed() : 0;
}

qint64 Application::totalDownloadSpeed() const
{
    return m_p2pEngine ? m_p2pEngine->downloadSpeed() : 0;
}

bool Application::startListening(quint16 port)
{
    if (!m_p2pEngine) {
        return false;
    }
    
    m_listenPort = m_p2pEngine->startListening(port);
    
    // Start announcing ourselves
    if (m_discovery && m_listenPort > 0) {
        QString peerName = QHostInfo::localHostName();
        m_discovery->announce(peerName, m_listenPort);
        m_discovery->startDiscovery();
    }
    
    return m_listenPort > 0;
}

void Application::stopListening()
{
    if (m_p2pEngine) {
        m_p2pEngine->stopListening();
    }
    if (m_discovery) {
        m_discovery->stopDiscovery();
    }
    m_listenPort = 0;
}

bool Application::connectToPeer(const QString& address, quint16 port)
{
    if (!m_p2pEngine) {
        return false;
    }
    
    QHostAddress addr(address);
    if (addr.isNull()) {
        return false;
    }
    
    m_p2pEngine->connectToPeer(addr, port);
    return true;
}

void Application::disconnectFromPeer(const QString& peerId)
{
    if (m_p2pEngine) {
        m_p2pEngine->disconnectFromPeer(peerId);
    }
}

void Application::sendFile(const QString& peerId, const QString& filePath)
{
    if (!m_p2pEngine || !m_fileManager) {
        return;
    }
    
    auto fileInfo = m_fileManager->getFileInfo(filePath);
    if (fileInfo.id.isEmpty()) {
        emit transferError(QString(), tr("File not found: %1").arg(filePath));
        return;
    }
    
    m_p2pEngine->sendFile(peerId, fileInfo);
}

void Application::requestFile(const QString& peerId, const QString& fileId)
{
    if (m_p2pEngine) {
        m_p2pEngine->requestFile(peerId, fileId);
    }
}

void Application::cancelTransfer(const QString& transferId)
{
    if (m_p2pEngine) {
        m_p2pEngine->cancelTransfer(transferId);
    }
}

QVariantList Application::getDiscoveredPeers() const
{
    QVariantList result;
    
    if (m_discovery) {
        auto peers = m_discovery->getDiscoveredPeers();
        for (auto it = peers.begin(); it != peers.end(); ++it) {
            QVariantMap peerMap;
            peerMap["id"] = it->id;
            peerMap["name"] = it->name;
            peerMap["address"] = it->address.toString();
            peerMap["port"] = it->port;
            peerMap["isOnline"] = it->isOnline;
            peerMap["transferSpeed"] = it->transferSpeed;
            result.append(peerMap);
        }
    }
    
    return result;
}

QVariantList Application::getActiveTransfers() const
{
    QVariantList result;
    
    if (m_p2pEngine) {
        auto transfers = m_p2pEngine->getActiveTransfers();
        for (const auto& transfer : transfers) {
            QVariantMap transferMap;
            transferMap["transferId"] = transfer.transferId;
            transferMap["fileId"] = transfer.fileId;
            transferMap["fileName"] = transfer.fileName;
            transferMap["fileSize"] = transfer.fileSize;
            transferMap["sourcePeerId"] = transfer.sourcePeerId;
            transferMap["destPeerId"] = transfer.destPeerId;
            transferMap["isUpload"] = transfer.isUpload;
            transferMap["transferredBytes"] = transfer.transferredBytes;
            transferMap["currentSpeed"] = transfer.currentSpeed;
            
            double progress = transfer.fileSize > 0 
                ? static_cast<double>(transfer.transferredBytes) / transfer.fileSize 
                : 0.0;
            transferMap["progress"] = progress;
            
            result.append(transferMap);
        }
    }
    
    return result;
}

QString Application::getVersion()
{
    return "1.0.0";
}

void Application::setupQmlEngine()
{
    // Load QML file directly (Qt6 method)
    m_engine->load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    
    // Create context properties
    QQmlContext* context = m_engine->rootContext();
    context->setContextProperty("application", this);
    
    // Create MainView instance
    auto* mainView = new MainView(this, this);
    context->setContextProperty("mainView", mainView);
    
    if (m_engine->rootObjects().isEmpty()) {
        qCritical() << "Failed to load QML";
    }
}

void Application::registerQmlTypes()
{
    qmlRegisterUncreatableType<P2PEngine>("P2P", 1, 0, "P2PEngine", 
                                          "P2PEngine is not creatable from QML");
    qmlRegisterUncreatableType<FileManager>("P2P", 1, 0, "FileManager",
                                            "FileManager is not creatable from QML");
}

void Application::connectSignals()
{
    // P2P Engine signals - connect to lambda adapters since signatures don't match directly
    connect(m_p2pEngine.get(), &P2PEngine::peerConnected,
            this, [this](const QString& peerId, const PeerInfo& info) {
        QVariantMap peerMap;
        peerMap["id"] = peerId;
        peerMap["name"] = info.name;
        peerMap["address"] = info.address.toString();
        peerMap["port"] = info.port;
        emit peerConnected(peerMap);
    });
    connect(m_p2pEngine.get(), &P2PEngine::peerDisconnected,
            this, &Application::peerDisconnected);
    connect(m_p2pEngine.get(), &P2PEngine::transferStarted,
            this, [this](const QString& transferId, const TransferMetadata& metadata) {
        QVariantMap transferMap;
        transferMap["id"] = transferId;
        transferMap["fileName"] = metadata.fileName;
        transferMap["fileSize"] = metadata.fileSize;
        transferMap["peerId"] = metadata.sourcePeerId;
        emit transferStarted(transferMap);
    });
    connect(m_p2pEngine.get(), &P2PEngine::transferProgress,
            this, &Application::transferProgress);
    connect(m_p2pEngine.get(), &P2PEngine::transferCompleted,
            this, &Application::transferCompleted);
    connect(m_p2pEngine.get(), &P2PEngine::transferError,
            this, &Application::transferError);
    
    // Discovery signals
    connect(m_discovery.get(), &Discovery::peerFound,
            this, [this](const QString& /*peerId*/, const PeerInfo& info) {
        QVariantMap peerMap;
        peerMap["id"] = info.id;
        peerMap["name"] = info.name;
        peerMap["address"] = info.address.toString();
        peerMap["port"] = info.port;
        peerMap["isOnline"] = info.isOnline;
        emit peerDiscovered(peerMap);
    });
    
    // File Manager signals
    connect(m_fileManager.get(), &FileManager::transferProgress,
            this, [this](const QString& /*fileId*/, double /*progress*/, qint64 /*bytes*/) {
        // Forward to UI if needed
    });
}

} // namespace p2p
