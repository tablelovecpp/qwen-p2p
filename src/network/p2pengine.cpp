#include "network/p2pengine.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QHostInfo>
#include <QUuid>
#include <algorithm>

namespace p2p {

P2PEngine::P2PEngine(QObject* parent)
    : QObject(parent)
    , m_server(std::make_unique<QTcpServer>())
    , m_fileManager(nullptr)
    , m_transferTimer(new QTimer(this))
    , m_uploadSpeed(0)
    , m_downloadSpeed(0)
    , m_bytesUploaded(0)
    , m_bytesDownloaded(0)
    , m_lastSpeedUpdate(std::chrono::steady_clock::now())
    , m_peerId(generatePeerId())
    , m_localName(QHostInfo::localHostName())
{
    // Setup server
    connect(m_server.get(), &QTcpServer::newConnection, 
            this, &P2PEngine::onNewConnection);
    
    // Setup transfer timer for speed updates and keepalive
    m_transferTimer->setInterval(1000);  // Update every second
    connect(m_transferTimer, &QTimer::timeout, 
            this, &P2PEngine::onTransferTimer);
    m_transferTimer->start();
}

P2PEngine::~P2PEngine() = default;

void P2PEngine::setFileManager(FileManager* fileManager)
{
    m_fileManager = fileManager;
}

quint16 P2PEngine::startListening(quint16 port)
{
    if (!m_server->listen(QHostAddress::Any, port)) {
        qWarning() << "Failed to start listening:" << m_server->errorString();
        return 0;
    }
    
    quint16 actualPort = m_server->serverPort();
    emit serverStarted(actualPort);
    return actualPort;
}

void P2PEngine::stopListening()
{
    m_server->close();
    
    // Disconnect all peers
    for (auto it = m_connections.begin(); it != m_connections.end(); ) {
        it.value()->socket->disconnectFromHost();
        it = m_connections.erase(it);
    }
    
    emit serverStopped();
    emit peerCountChanged();
}

bool P2PEngine::isListening() const
{
    return m_server->isListening();
}

quint16 P2PEngine::listeningPort() const
{
    return m_server->serverPort();
}

QString P2PEngine::connectToPeer(const QHostAddress& address, quint16 port)
{
    auto socket = std::make_unique<QTcpSocket>();
    
    QString peerId = generatePeerId();
    
    connect(socket.get(), &QTcpSocket::readyRead,
            this, [this, peerId]() {
                onSocketReadyRead();
            });
    connect(socket.get(), &QTcpSocket::disconnected,
            this, [this, peerId]() {
                onSocketDisconnected();
            });
    connect(socket.get(), &QTcpSocket::errorOccurred,
            this, [this, peerId](QAbstractSocket::SocketError error) {
                onSocketError(error);
            });
    
    socket->connectToHost(address, port);
    
    if (!socket->waitForConnected(5000)) {
        qWarning() << "Failed to connect to peer:" << socket->errorString();
        return QString();
    }
    
    // Create connection record
    auto conn = std::make_unique<Connection>();
    conn->socket = socket.release();
    conn->peerInfo.id = peerId;
    conn->peerInfo.address = address;
    conn->peerInfo.port = port;
    conn->isAuthenticated = false;
    conn->lastActivity = std::chrono::steady_clock::now();
    
    m_connections[peerId] = std::move(conn);
    emit peerCountChanged();
    
    // Send handshake: peerId:peerName:protocolVersion
    QString handshakeStr = m_peerId + ":" + m_localName + ":" + PROTOCOL_VERSION;
    QByteArray handshake = handshakeStr.toUtf8();
    sendMessage(peerId, MessageType::Handshake, handshake);
    
    return peerId;
}

void P2PEngine::disconnectFromPeer(const QString& peerId)
{
    auto it = m_connections.find(peerId);
    if (it != m_connections.end()) {
        it.value()->socket->disconnectFromHost();
        m_connections.erase(it);
        emit peerDisconnected(peerId);
        emit peerCountChanged();
    }
}

QMap<QString, PeerInfo> P2PEngine::getConnectedPeers() const
{
    QMap<QString, PeerInfo> result;
    for (const auto& [id, conn] : m_connections.asKeyValueRange()) {
        result[id] = conn->peerInfo;
    }
    return result;
}

int P2PEngine::connectedPeerCount() const
{
    return m_connections.size();
}

QString P2PEngine::sendFile(const QString& peerId, const FileInfo& fileInfo)
{
    auto it = m_connections.find(peerId);
    if (it == m_connections.end()) {
        return QString();
    }
    
    initiateTransfer(peerId, fileInfo, true);
    return fileInfo.id;
}

QString P2PEngine::requestFile(const QString& peerId, const QString& fileId)
{
    auto it = m_connections.find(peerId);
    if (it == m_connections.end()) {
        return QString();
    }
    
    // Send file request
    QByteArray requestData = fileId.toUtf8();
    sendMessage(peerId, MessageType::FileRequest, requestData);
    
    return fileId;
}

void P2PEngine::cancelTransfer(const QString& transferId)
{
    auto it = m_activeTransfers.find(transferId);
    if (it != m_activeTransfers.end()) {
        // Send cancel message to peer
        sendMessage(it.value().destPeerId, MessageType::Error, 
                   tr("Transfer cancelled").toUtf8());
        m_activeTransfers.erase(it);
    }
    
    if (m_fileManager) {
        m_fileManager->cancelTransfer(transferId);
    }
}

QList<TransferMetadata> P2PEngine::getActiveTransfers() const
{
    return m_activeTransfers.values();
}

QString P2PEngine::generatePeerId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void P2PEngine::onNewConnection()
{
    // SECURITY FIX: Limit maximum connections to prevent DoS attacks
    if (m_connections.size() >= MAX_CONNECTIONS) {
        qWarning() << "Maximum connection limit reached, rejecting new connection";
        QTcpSocket* socket = m_server->nextPendingConnection();
        if (socket) {
            socket->disconnectFromHost();
            socket->deleteLater();
        }
        return;
    }
    
    while (QTcpSocket* socket = m_server->nextPendingConnection()) {
        QString peerId = extractPeerId(socket);
        
        auto conn = std::make_unique<Connection>();
        conn->socket = socket;
        conn->peerInfo.id = peerId;
        conn->peerInfo.address = socket->peerAddress();
        conn->peerInfo.port = socket->peerPort();
        conn->isAuthenticated = false;
        conn->lastActivity = std::chrono::steady_clock::now();
        
        // Setup socket handlers
        connect(socket, &QTcpSocket::readyRead,
                this, &P2PEngine::onSocketReadyRead);
        connect(socket, &QTcpSocket::disconnected,
                this, &P2PEngine::onSocketDisconnected);
        connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
                this, &P2PEngine::onSocketError);
        
        m_connections[peerId] = std::move(conn);
        emit peerCountChanged();
    }
}

void P2PEngine::onSocketReadyRead()
{
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    QString peerId = extractPeerId(socket);
    if (peerId.isEmpty()) return;
    
    auto it = m_connections.find(peerId);
    if (it == m_connections.end()) return;
    
    Connection* conn = it.value().get();
    conn->buffer.append(socket->readAll());
    conn->lastActivity = std::chrono::steady_clock::now();
    
    // Process complete messages
    while (conn->buffer.size() >= 5) {  // Minimum: 1 byte type + 4 bytes length
        quint8 type = static_cast<quint8>(conn->buffer[0]);
        
        // SECURITY FIX: Validate message length to prevent integer overflow attacks
        if (conn->buffer.size() < 5) break;
        
        quint32 length = qFromBigEndian<quint32>(conn->buffer.constData() + 1);
        
        // Sanity check on message length (max 10MB for any single message)
        constexpr quint32 MAX_MESSAGE_SIZE = 10 * 1024 * 1024;
        if (length > MAX_MESSAGE_SIZE) {
            qWarning() << "Oversized message from" << peerId << ":" << length << "bytes";
            socket->disconnectFromHost();
            return;
        }
        
        if (conn->buffer.size() < static_cast<qsizetype>(length + 5)) {
            break;  // Wait for more data
        }
        
        QByteArray message = conn->buffer.mid(5, length);
        conn->buffer.remove(0, length + 5);
        
        m_bytesDownloaded += message.size();
        
        processMessage(peerId, message);
    }
}

void P2PEngine::onSocketDisconnected()
{
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    QString peerId = extractPeerId(socket);
    if (!peerId.isEmpty()) {
        m_connections.remove(peerId);
        emit peerDisconnected(peerId);
        emit peerCountChanged();
    }
}

void P2PEngine::onSocketError(QAbstractSocket::SocketError error)
{
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    qWarning() << "Socket error:" << socket->errorString();
    
    QString peerId = extractPeerId(socket);
    if (!peerId.isEmpty()) {
        emit transferError(peerId, socket->errorString());
    }
}

void P2PEngine::onTransferTimer()
{
    updateTransferSpeeds();
    
    // Check for stale connections
    auto now = std::chrono::steady_clock::now();
    for (auto it = m_connections.begin(); it != m_connections.end(); ) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - it.value()->lastActivity).count();
        
        if (elapsed > KEEPALIVE_INTERVAL / 1000 * 2) {
            // Connection timed out
            it.value()->socket->disconnectFromHost();
            QString peerId = it.key();
            it = m_connections.erase(it);
            emit peerDisconnected(peerId);
            emit peerCountChanged();
        } else {
            ++it;
        }
    }
    
    // Send keepalive to all connections
    for (const auto& [peerId, conn] : m_connections.asKeyValueRange()) {
        sendMessage(peerId, MessageType::KeepAlive, QByteArray());
    }
}

void P2PEngine::processMessage(const QString& peerId, const QByteArray& data)
{
    if (data.isEmpty()) return;
    
    MessageType type = static_cast<MessageType>(data[0]);
    QByteArray payload = data.mid(1);
    
    switch (type) {
        case MessageType::Handshake:
            handleHandshake(peerId, payload);
            break;
        case MessageType::FileInfo:
            handleFileInfo(peerId, payload);
            break;
        case MessageType::FileRequest:
            handleFileRequest(peerId, payload);
            break;
        case MessageType::FileData:
            handleFileData(peerId, payload);
            break;
        case MessageType::FileAck:
            handleFileAck(peerId, payload);
            break;
        case MessageType::TransferComplete:
            handleTransferComplete(peerId, payload);
            break;
        case MessageType::Error:
            handleError(peerId, payload);
            break;
        default:
            qWarning() << "Unknown message type:" << static_cast<int>(type);
    }
}

void P2PEngine::handleHandshake(const QString& peerId, const QByteArray& data)
{
    auto it = m_connections.find(peerId);
    if (it != m_connections.end()) {
        // SECURITY FIX: Implement proper handshake validation
        // Expected format: peerId:peerName:protocolVersion
        QList<QByteArray> parts = data.split(':');
        
        if (parts.size() < 3) {
            qWarning() << "Invalid handshake format from" << peerId;
            // Disconnect invalid peers
            it.value()->socket->disconnectFromHost();
            return;
        }
        
        QString receivedPeerId = QString::fromUtf8(parts[0]);
        QString peerName = QString::fromUtf8(parts[1]);
        QString protocolVersion = QString::fromUtf8(parts[2]);
        
        // Validate that the peer ID matches what we expect
        if (receivedPeerId != it.key()) {
            qWarning() << "Peer ID mismatch! Expected:" << it.key() << "Got:" << receivedPeerId;
            it.value()->socket->disconnectFromHost();
            return;
        }
        
        // Validate protocol version
        if (protocolVersion != PROTOCOL_VERSION) {
            qWarning() << "Protocol version mismatch from" << peerId << ":" << protocolVersion;
            it.value()->socket->disconnectFromHost();
            return;
        }
        
        it.value()->peerInfo.name = peerName;
        it.value()->isAuthenticated = true;
        
        // Send back our own handshake with version
        QByteArray response = (m_peerId + ":" + m_localName + ":" + PROTOCOL_VERSION).toUtf8();
        sendMessage(peerId, MessageType::Handshake, response);
        
        emit peerConnected(peerId, it.value()->peerInfo);
    }
}

void P2PEngine::handleFileInfo(const QString& peerId, const QByteArray& data)
{
    // Parse file info and notify UI
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return;
    
    QJsonObject obj = doc.object();
    FileInfo info;
    info.id = obj["id"].toString();
    info.name = obj["name"].toString();
    info.size = obj["size"].toInteger();
    info.hash = obj["hash"].toString();
    
    // Auto-request or notify user based on settings
}

void P2PEngine::handleFileRequest(const QString& peerId, const QByteArray& data)
{
    QString fileId = QString::fromUtf8(data);
    emit incomingFileRequest(peerId, fileId);
    
    // If we have the file, start sending
    if (m_fileManager) {
        // Implementation would retrieve file and start transfer
    }
}

void P2PEngine::handleFileData(const QString& peerId, const QByteArray& data)
{
    // Parse file data chunk
    if (data.size() < 8) return;  // Need at least fileId (variable) + offset (8 bytes)
    
    // Extract file ID and offset from beginning
    int nullPos = data.indexOf('\0');
    if (nullPos < 0) return;
    
    QString fileId = QString::fromUtf8(data.left(nullPos));
    qint64 offset = qFromBigEndian<qint64>(data.constData() + nullPos + 1);
    QByteArray chunk = data.mid(nullPos + 9);
    
    if (m_fileManager) {
        FileInfo info;
        info.id = fileId;
        m_fileManager->saveFileChunk(info, chunk, offset);
    }
}

void P2PEngine::handleFileAck(const QString& peerId, const QByteArray& data)
{
    // Acknowledge received chunk, continue transfer
}

void P2PEngine::handleTransferComplete(const QString& peerId, const QByteArray& data)
{
    QString transferId = QString::fromUtf8(data);
    emit transferCompleted(transferId, QString());
    m_activeTransfers.remove(transferId);
}

void P2PEngine::handleError(const QString& peerId, const QByteArray& data)
{
    QString error = QString::fromUtf8(data);
    emit transferError(peerId, error);
}

void P2PEngine::sendMessage(const QString& peerId, MessageType type, const QByteArray& payload)
{
    auto it = m_connections.find(peerId);
    if (it == m_connections.end()) return;
    
    QTcpSocket* socket = it.value()->socket;
    QByteArray message = createMessage(type, payload);
    
    socket->write(message);
    m_bytesUploaded += message.size();
}

void P2PEngine::broadcastMessage(MessageType type, const QByteArray& payload)
{
    for (const auto& [peerId, conn] : m_connections.asKeyValueRange()) {
        sendMessage(peerId, type, payload);
    }
}

void P2PEngine::initiateTransfer(const QString& peerId, const FileInfo& fileInfo, bool isUpload)
{
    TransferMetadata metadata;
    metadata.transferId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    metadata.fileId = fileInfo.id;
    metadata.fileName = fileInfo.name;
    metadata.fileSize = fileInfo.size;
    metadata.sourcePeerId = isUpload ? m_peerId : peerId;
    metadata.destPeerId = isUpload ? peerId : m_peerId;
    metadata.isUpload = isUpload;
    metadata.startTime = std::chrono::steady_clock::now();
    metadata.transferredBytes = 0;
    metadata.currentSpeed = 0;
    
    m_activeTransfers[metadata.transferId] = metadata;
    
    // Send file info to peer
    QJsonObject obj;
    obj["id"] = fileInfo.id;
    obj["name"] = fileInfo.name;
    obj["size"] = fileInfo.size;
    obj["hash"] = fileInfo.hash;
    
    sendMessage(peerId, MessageType::FileInfo, QJsonDocument(obj).toJson());
    
    emit transferStarted(metadata.transferId, metadata);
}

void P2PEngine::updateTransferSpeeds()
{
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        now - m_lastSpeedUpdate).count();
    
    if (elapsed > 0) {
        m_uploadSpeed = m_bytesUploaded / elapsed;
        m_downloadSpeed = m_bytesDownloaded / elapsed;
        
        m_bytesUploaded = 0;
        m_bytesDownloaded = 0;
        m_lastSpeedUpdate = now;
        
        emit speedUpdated();
        
        // Update transfer speeds
        for (auto it = m_activeTransfers.begin(); it != m_activeTransfers.end(); ++it) {
            it.value().currentSpeed = it.value().isUpload ? m_uploadSpeed.load() : m_downloadSpeed.load();
        }
    }
}

QByteArray P2PEngine::createMessage(MessageType type, const QByteArray& payload)
{
    QByteArray message;
    message.reserve(5 + payload.size());
    
    message.append(static_cast<char>(type));
    message.append(qToBigEndian<quint32>(static_cast<quint32>(payload.size())));
    message.append(payload);
    
    return message;
}

QString P2PEngine::extractPeerId(QTcpSocket* socket) const
{
    for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
        if (it.value()->socket == socket) {
            return it.key();
        }
    }
    return QString();
}

} // namespace p2p
