#ifndef P2PTRANSFER_NETWORK_P2PENGINE_HPP
#define P2PTRANSFER_NETWORK_P2PENGINE_HPP

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QQueue>
#include <memory>
#include <mutex>
#include <atomic>

#include "core/peerinfo.hpp"
#include "core/filemanager.hpp"

namespace p2p {

/**
 * @brief Protocol message types for P2P communication
 */
enum class MessageType : quint8 {
    Handshake = 0x01,
    PeerList = 0x02,
    FileInfo = 0x03,
    FileRequest = 0x04,
    FileData = 0x05,
    FileAck = 0x06,
    TransferComplete = 0x07,
    KeepAlive = 0x08,
    Error = 0xFF
};

/**
 * @brief P2P transfer metadata
 */
struct TransferMetadata {
    QString transferId;
    QString fileId;
    QString fileName;
    qint64 fileSize;
    QString sourcePeerId;
    QString destPeerId;
    bool isUpload;
    std::chrono::steady_clock::time_point startTime;
    qint64 transferredBytes;
    qint64 currentSpeed;
    
    TransferMetadata()
        : fileSize(0)
        , isUpload(false)
        , transferredBytes(0)
        , currentSpeed(0) {}
};

/**
 * @brief Core P2P engine handling all network communication
 * 
 * Implements a high-performance P2P protocol with:
 * - Automatic peer discovery integration
 * - Concurrent file transfers
 * - Flow control and congestion avoidance
 * - Checksum verification
 * - Resume capability for interrupted transfers
 */
class P2PEngine : public QObject {
    Q_OBJECT
    
    Q_PROPERTY(int connectedPeerCount READ connectedPeerCount NOTIFY peerCountChanged)
    Q_PROPERTY(qint64 uploadSpeed READ uploadSpeed NOTIFY speedUpdated)
    Q_PROPERTY(qint64 downloadSpeed READ downloadSpeed NOTIFY speedUpdated)
    
public:
    explicit P2PEngine(QObject* parent = nullptr);
    ~P2PEngine() override;
    
    /**
     * @brief Set the file manager for file operations
     * @param fileManager File manager instance
     */
    void setFileManager(FileManager* fileManager);
    
    /**
     * @brief Start listening for incoming connections
     * @param port Port to listen on (0 for random)
     * @return Actual listening port (may differ if 0 was specified)
     */
    quint16 startListening(quint16 port = 0);
    
    /**
     * @brief Stop listening for incoming connections
     */
    void stopListening();
    
    /**
     * @brief Check if server is listening
     * @return True if listening
     */
    bool isListening() const;
    
    /**
     * @brief Get the listening port
     * @return Port number
     */
    quint16 listeningPort() const;
    
    /**
     * @brief Connect to a peer
     * @param address Peer IP address
     * @param port Peer port
     * @return Connection ID
     */
    QString connectToPeer(const QHostAddress& address, quint16 port);
    
    /**
     * @brief Disconnect from a peer
     * @param peerId Peer ID to disconnect
     */
    void disconnectFromPeer(const QString& peerId);
    
    /**
     * @brief Get list of connected peers
     * @return Map of peer IDs to peer info
     */
    QMap<QString, PeerInfo> getConnectedPeers() const;
    
    /**
     * @brief Get number of connected peers
     * @return Peer count
     */
    int connectedPeerCount() const;
    
    /**
     * @brief Get current upload speed in bytes/second
     * @return Upload speed
     */
    qint64 uploadSpeed() const { return m_uploadSpeed; }
    
    /**
     * @brief Get current download speed in bytes/second
     * @return Download speed
     */
    qint64 downloadSpeed() const { return m_downloadSpeed; }
    
    /**
     * @brief Send a file to a peer
     * @param peerId Target peer ID
     * @param fileInfo File information
     * @return Transfer ID
     */
    QString sendFile(const QString& peerId, const FileInfo& fileInfo);
    
    /**
     * @brief Request a file from a peer
     * @param peerId Source peer ID
     * @param fileId File ID to request
     * @return Transfer ID
     */
    QString requestFile(const QString& peerId, const QString& fileId);
    
    /**
     * @brief Cancel an ongoing transfer
     * @param transferId Transfer ID to cancel
     */
    void cancelTransfer(const QString& transferId);
    
    /**
     * @brief Get active transfers
     * @return List of transfer metadata
     */
    QList<TransferMetadata> getActiveTransfers() const;
    
    /**
     * @brief Generate unique peer ID
     * @return Unique peer ID string
     */
    static QString generatePeerId();
    
signals:
    void peerCountChanged();
    void speedUpdated();
    void serverStarted(quint16 port);
    void serverStopped();
    void peerConnected(const QString& peerId, const PeerInfo& info);
    void peerDisconnected(const QString& peerId);
    void transferStarted(const QString& transferId, const TransferMetadata& metadata);
    void transferProgress(const QString& transferId, double progress, qint64 speed);
    void transferCompleted(const QString& transferId, const QString& filePath);
    void transferError(const QString& transferId, const QString& error);
    void incomingFileRequest(const QString& peerId, const QString& fileId);
    
private slots:
    void onNewConnection();
    void onSocketReadyRead();
    void onSocketDisconnected();
    void onSocketError(QAbstractSocket::SocketError error);
    void onTransferTimer();
    
private:
    struct Connection {
        QTcpSocket* socket;
        PeerInfo peerInfo;
        QByteArray buffer;
        bool isAuthenticated;
        std::chrono::steady_clock::time_point lastActivity;
    };
    
    // Message handling
    void processMessage(const QString& peerId, const QByteArray& data);
    void handleHandshake(const QString& peerId, const QByteArray& data);
    void handleFileInfo(const QString& peerId, const QByteArray& data);
    void handleFileRequest(const QString& peerId, const QByteArray& data);
    void handleFileData(const QString& peerId, const QByteArray& data);
    void handleFileAck(const QString& peerId, const QByteArray& data);
    void handleTransferComplete(const QString& peerId, const QByteArray& data);
    void handleError(const QString& peerId, const QByteArray& data);
    
    // Sending messages
    void sendMessage(const QString& peerId, MessageType type, const QByteArray& payload);
    void broadcastMessage(MessageType type, const QByteArray& payload);
    
    // Transfer management
    void initiateTransfer(const QString& peerId, const FileInfo& fileInfo, bool isUpload);
    void updateTransferSpeeds();
    
    // Utilities
    QByteArray createMessage(MessageType type, const QByteArray& payload);
    QString extractPeerId(QTcpSocket* socket) const;
    
    std::unique_ptr<QTcpServer> m_server;
    QMap<QString, std::unique_ptr<Connection>> m_connections;
    QMap<QString, TransferMetadata> m_activeTransfers;
    FileManager* m_fileManager;
    
    QTimer* m_transferTimer;
    std::atomic<qint64> m_uploadSpeed;
    std::atomic<qint64> m_downloadSpeed;
    std::atomic<qint64> m_bytesUploaded;
    std::atomic<qint64> m_bytesDownloaded;
    std::chrono::steady_clock::time_point m_lastSpeedUpdate;
    
    QString m_peerId;
    mutable std::mutex m_mutex;
    
    static constexpr qint64 CHUNK_SIZE = 64 * 1024;  // 64KB chunks
    static constexpr int KEEPALIVE_INTERVAL = 30000;  // 30 seconds
};

} // namespace p2p

#endif // P2PTRANSFER_NETWORK_P2PENGINE_HPP
