#ifndef P2PTRANSFER_NETWORK_DISCOVERY_HPP
#define P2PTRANSFER_NETWORK_DISCOVERY_HPP

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QMap>
#include <memory>

#include "core/peerinfo.hpp"

namespace p2p {

/**
 * @brief Network discovery using UDP broadcast/multicast
 * 
 * Implements peer discovery using:
 * - UDP broadcast for local network discovery
 * - Multicast for larger networks
 * - mDNS/DNS-SD compatibility layer (optional)
 */
class Discovery : public QObject {
    Q_OBJECT
    
    Q_PROPERTY(int discoveredPeerCount READ discoveredPeerCount NOTIFY peerCountChanged)
    
public:
    explicit Discovery(QObject* parent = nullptr);
    ~Discovery() override;
    
    /**
     * @brief Start peer discovery
     * @param port Port for discovery protocol (default: 47474)
     * @param useMulticast Use multicast instead of broadcast
     * @return True if started successfully
     */
    bool startDiscovery(quint16 port = 47474, bool useMulticast = false);
    
    /**
     * @brief Stop peer discovery
     */
    void stopDiscovery();
    
    /**
     * @brief Check if discovery is running
     * @return True if running
     */
    bool isRunning() const { return m_isRunning; }
    
    /**
     * @brief Announce this peer to the network
     * @param name Peer display name
     * @param port Service port number
     * @param metadata Additional metadata (JSON format)
     */
    void announce(const QString& name, quint16 port, const QString& metadata = {});
    
    /**
     * @brief Get list of discovered peers
     * @return Map of peer IDs to peer info
     */
    QMap<QString, PeerInfo> getDiscoveredPeers() const;
    
    /**
     * @brief Get number of discovered peers
     * @return Peer count
     */
    int discoveredPeerCount() const;
    
    /**
     * @brief Clear the list of discovered peers
     */
    void clearPeers();
    
    /**
     * @brief Remove a peer from the discovered list
     * @param peerId Peer ID to remove
     */
    void removePeer(const QString& peerId);
    
signals:
    void peerCountChanged();
    void peerFound(const QString& peerId, const PeerInfo& info);
    void peerLost(const QString& peerId);
    void discoveryError(const QString& error);
    
private slots:
    void processPendingDatagrams();
    void onAnnouncementTimer();
    void onCleanupTimer();
    
private:
    struct DiscoveryPacket {
        QString magic;          // Protocol magic string
        QString version;        // Protocol version
        QString peerId;         // Unique peer identifier
        QString peerName;       // Display name
        QHostAddress address;   // Sender address
        quint16 servicePort;    // Service port
        QString metadata;       // Additional JSON metadata
        qint64 timestamp;       // Unix timestamp
    };
    
    // Packet handling
    QByteArray createAnnouncementPacket();
    std::optional<DiscoveryPacket> parseAnnouncementPacket(const QByteArray& data, 
                                                           const QHostAddress& sender);
    void sendAnnouncement();
    void sendDiscoveryRequest();
    
    // Peer management
    void addOrUpdatePeer(const DiscoveryPacket& packet);
    void checkPeerTimeout();
    
    std::unique_ptr<QUdpSocket> m_socket;
    QMap<QString, PeerInfo> m_discoveredPeers;
    
    QTimer* m_announcementTimer;
    QTimer* m_cleanupTimer;
    
    QString m_peerId;
    QString m_peerName;
    quint16 m_servicePort;
    quint16 m_discoveryPort;
    bool m_isRunning;
    bool m_useMulticast;
    
    QHostAddress m_multicastAddress;
    
    static constexpr const char* PROTOCOL_MAGIC = "P2PDISC";
    static constexpr const char* PROTOCOL_VERSION = "1.0";
    static constexpr int ANNOUNCEMENT_INTERVAL = 5000;   // 5 seconds
    static constexpr int PEER_TIMEOUT = 15000;            // 15 seconds
    static constexpr int CLEANUP_INTERVAL = 10000;        // 10 seconds
};

} // namespace p2p

#endif // P2PTRANSFER_NETWORK_DISCOVERY_HPP
