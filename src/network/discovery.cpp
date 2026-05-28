#include "network/discovery.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QUuid>
#include <QNetworkDatagram>

namespace p2p {

Discovery::Discovery(QObject* parent)
    : QObject(parent)
    , m_socket(std::make_unique<QUdpSocket>())
    , m_announcementTimer(new QTimer(this))
    , m_cleanupTimer(new QTimer(this))
    , m_peerId(QUuid::createUuid().toString(QUuid::WithoutBraces))
    , m_servicePort(0)
    , m_discoveryPort(47474)
    , m_isRunning(false)
    , m_useMulticast(false)
    , m_multicastAddress("239.255.43.21")  // Multicast address for P2P discovery
{
    connect(m_announcementTimer, &QTimer::timeout, 
            this, &Discovery::onAnnouncementTimer);
    connect(m_cleanupTimer, &QTimer::timeout,
            this, &Discovery::onCleanupTimer);
    
    connect(m_socket.get(), &QUdpSocket::readyRead,
            this, &Discovery::processPendingDatagrams);
}

Discovery::~Discovery() = default;

bool Discovery::startDiscovery(quint16 port, bool useMulticast)
{
    m_discoveryPort = port;
    m_useMulticast = useMulticast;
    
    QHostAddress bindAddress;
    
    if (useMulticast) {
        bindAddress = QHostAddress::Any;
    } else {
        bindAddress = QHostAddress::Any;
    }
    
    if (!m_socket->bind(bindAddress, m_discoveryPort, 
                        QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        emit discoveryError(m_socket->errorString());
        return false;
    }
    
    if (useMulticast) {
        if (!m_socket->joinMulticastGroup(m_multicastAddress)) {
            emit discoveryError("Failed to join multicast group");
            return false;
        }
    }
    
    m_isRunning = true;
    
    // Start timers
    m_announcementTimer->start(ANNOUNCEMENT_INTERVAL);
    m_cleanupTimer->start(CLEANUP_INTERVAL);
    
    // Send initial announcement
    sendAnnouncement();
    
    return true;
}

void Discovery::stopDiscovery()
{
    m_announcementTimer->stop();
    m_cleanupTimer->stop();
    
    if (m_useMulticast) {
        m_socket->leaveMulticastGroup(m_multicastAddress);
    }
    
    m_socket->close();
    m_isRunning = false;
}

void Discovery::announce(const QString& name, quint16 port, const QString& metadata)
{
    m_peerName = name;
    m_servicePort = port;
    
    if (m_isRunning) {
        sendAnnouncement();
    }
}

QMap<QString, PeerInfo> Discovery::getDiscoveredPeers() const
{
    return m_discoveredPeers;
}

int Discovery::discoveredPeerCount() const
{
    return m_discoveredPeers.size();
}

void Discovery::clearPeers()
{
    m_discoveredPeers.clear();
    emit peerCountChanged();
}

void Discovery::removePeer(const QString& peerId)
{
    auto it = m_discoveredPeers.find(peerId);
    if (it != m_discoveredPeers.end()) {
        emit peerLost(peerId);
        m_discoveredPeers.erase(it);
        emit peerCountChanged();
    }
}

void Discovery::processPendingDatagrams()
{
    while (m_socket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_socket->receiveDatagram();
        
        if (datagram.isValid()) {
            // SECURITY FIX: Validate sender address to prevent UDP spoofing
            // Only accept datagrams from private IP ranges or localhost
            QHostAddress senderAddr = datagram.senderAddress();
            if (!isTrustedAddress(senderAddr)) {
                qWarning() << "Ignoring datagram from untrusted address:" << senderAddr.toString();
                continue;
            }
            
            auto packet = parseAnnouncementPacket(datagram.data(), datagram.senderAddress());
            if (packet.has_value()) {
                addOrUpdatePeer(packet.value());
            }
        }
    }
}

bool Discovery::isTrustedAddress(const QHostAddress& address) const
{
    // Allow localhost
    if (address == QHostAddress::LocalHost || 
        address == QHostAddress::LocalHostIPv6 ||
        address.isLoopback()) {
        return true;
    }
    
    // Allow private IPv4 ranges (RFC 1918)
    if (address.protocol() == QAbstractSocket::IPv4Protocol) {
        quint32 ip = address.toIPv4Address();
        
        // 10.0.0.0/8
        if ((ip & 0xFF000000) == 0x0A000000) {
            return true;
        }
        
        // 172.16.0.0/12
        if ((ip & 0xFFF00000) == 0xAC100000) {
            return true;
        }
        
        // 192.168.0.0/16
        if ((ip & 0xFFFF0000) == 0xC0A80000) {
            return true;
        }
    }
    
    // Allow Unique Local Addresses (ULA) for IPv6 (fc00::/7)
    if (address.protocol() == QAbstractSocket::IPv6Protocol) {
        Q_IPV6ADDR ipv6 = address.toIPv6Address();
        if ((ipv6[0] & 0xFE) == 0xFC) {
            return true;
        }
    }
    
    // Allow link-local addresses
    if (address.isMulticast() || address.isLinkLocal()) {
        return true;
    }
    
    return false;
}

void Discovery::onAnnouncementTimer()
{
    sendAnnouncement();
}

void Discovery::onCleanupTimer()
{
    checkPeerTimeout();
}

QByteArray Discovery::createAnnouncementPacket()
{
    QJsonObject obj;
    obj["magic"] = PROTOCOL_MAGIC;
    obj["version"] = PROTOCOL_VERSION;
    obj["peerId"] = m_peerId;
    obj["peerName"] = m_peerName;
    obj["servicePort"] = static_cast<int>(m_servicePort);
    obj["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

std::optional<Discovery::DiscoveryPacket> Discovery::parseAnnouncementPacket(
    const QByteArray& data, const QHostAddress& sender)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        return std::nullopt;
    }
    
    QJsonObject obj = doc.object();
    
    // Validate magic string
    if (obj["magic"].toString() != PROTOCOL_MAGIC) {
        return std::nullopt;
    }
    
    DiscoveryPacket packet;
    packet.magic = obj["magic"].toString();
    packet.version = obj["version"].toString();
    packet.peerId = obj["peerId"].toString();
    packet.peerName = obj["peerName"].toString();
    packet.address = sender;
    packet.servicePort = static_cast<quint16>(obj["servicePort"].toInt());
    packet.metadata = obj["metadata"].toString();
    packet.timestamp = obj["timestamp"].toVariant().toLongLong();
    
    // Don't process our own announcements
    if (packet.peerId == m_peerId) {
        return std::nullopt;
    }
    
    return packet;
}

void Discovery::sendAnnouncement()
{
    QByteArray packet = createAnnouncementPacket();
    
    if (m_useMulticast) {
        m_socket->writeDatagram(packet, m_multicastAddress, m_discoveryPort);
    } else {
        // Broadcast to local network
        m_socket->writeDatagram(packet, QHostAddress::Broadcast, m_discoveryPort);
    }
}

void Discovery::sendDiscoveryRequest()
{
    // Send a request packet to discover peers
    QJsonObject obj;
    obj["type"] = "discovery_request";
    obj["peerId"] = m_peerId;
    
    QByteArray packet = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    
    if (m_useMulticast) {
        m_socket->writeDatagram(packet, m_multicastAddress, m_discoveryPort);
    } else {
        m_socket->writeDatagram(packet, QHostAddress::Broadcast, m_discoveryPort);
    }
}

void Discovery::addOrUpdatePeer(const DiscoveryPacket& packet)
{
    bool isNew = !m_discoveredPeers.contains(packet.peerId);
    
    PeerInfo info;
    info.id = packet.peerId;
    info.name = packet.peerName;
    info.address = packet.address;
    info.port = packet.servicePort;
    info.isOnline = true;
    info.lastSeen = std::chrono::steady_clock::now();
    
    m_discoveredPeers[packet.peerId] = info;
    
    if (isNew) {
        emit peerFound(packet.peerId, info);
        emit peerCountChanged();
    }
}

void Discovery::checkPeerTimeout()
{
    auto now = std::chrono::steady_clock::now();
    QList<QString> expiredPeers;
    
    for (auto it = m_discoveredPeers.begin(); it != m_discoveredPeers.end(); ++it) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - it->lastSeen).count();
        
        if (elapsed > PEER_TIMEOUT) {
            expiredPeers.append(it.key());
        }
    }
    
    for (const QString& peerId : expiredPeers) {
        removePeer(peerId);
    }
}

} // namespace p2p
