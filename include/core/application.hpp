#ifndef P2PTRANSFER_CORE_APPLICATION_HPP
#define P2PTRANSFER_CORE_APPLICATION_HPP

#include <QObject>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <memory>

namespace p2p {

class P2PEngine;
class FileManager;
class Discovery;

/**
 * @brief Main application controller
 * 
 * Manages the application lifecycle, coordinates between UI and backend,
 * and provides QML-accessible interface for the P2P functionality.
 */
class Application : public QObject {
    Q_OBJECT
    
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(int connectedPeers READ connectedPeersCount NOTIFY peersChanged)
    Q_PROPERTY(qint64 totalUploadSpeed READ totalUploadSpeed NOTIFY speedChanged)
    Q_PROPERTY(qint64 totalDownloadSpeed READ totalDownloadSpeed NOTIFY speedChanged)
    
public:
    explicit Application(QObject* parent = nullptr);
    ~Application() override;
    
    /**
     * @brief Initialize the application
     * @param argc Command line argument count
     * @param argv Command line arguments
     * @return True if initialization successful
     */
    bool initialize(int argc, char* argv[]);
    
    /**
     * @brief Run the application event loop
     * @return Application exit code
     */
    int run();
    
    /**
     * @brief Check if application is running
     * @return True if running
     */
    bool isRunning() const { return m_isRunning; }
    
    /**
     * @brief Get number of connected peers
     * @return Number of connected peers
     */
    int connectedPeersCount() const;
    
    /**
     * @brief Get total upload speed in bytes/second
     * @return Upload speed
     */
    qint64 totalUploadSpeed() const;
    
    /**
     * @brief Get total download speed in bytes/second
     * @return Download speed
     */
    qint64 totalDownloadSpeed() const;
    
public slots:
    /**
     * @brief Start listening for incoming connections
     * @param port Port to listen on
     * @return True if started successfully
     */
    bool startListening(quint16 port);
    
    /**
     * @brief Stop listening for incoming connections
     */
    void stopListening();
    
    /**
     * @brief Connect to a peer
     * @param address Peer IP address
     * @param port Peer port
     * @return True if connection initiated
     */
    bool connectToPeer(const QString& address, quint16 port);
    
    /**
     * @brief Disconnect from a peer
     * @param peerId Peer ID to disconnect
     */
    void disconnectFromPeer(const QString& peerId);
    
    /**
     * @brief Send a file to a peer
     * @param peerId Target peer ID
     * @param filePath Path to the file
     */
    void sendFile(const QString& peerId, const QString& filePath);
    
    /**
     * @brief Request a file from a peer
     * @param peerId Source peer ID
     * @param fileId File ID to request
     */
    void requestFile(const QString& peerId, const QString& fileId);
    
    /**
     * @brief Cancel a file transfer
     * @param transferId Transfer ID to cancel
     */
    void cancelTransfer(const QString& transferId);
    
    /**
     * @brief Get list of discovered peers (QML accessible)
     * @return QVariantList of peer information
     */
    Q_INVOKABLE QVariantList getDiscoveredPeers() const;
    
    /**
     * @brief Get list of active transfers (QML accessible)
     * @return QVariantList of transfer information
     */
    Q_INVOKABLE QVariantList getActiveTransfers() const;
    
    /**
     * @brief Get application version
     * @return Version string
     */
    Q_INVOKABLE static QString getVersion();
    
signals:
    void runningChanged();
    void peersChanged();
    void speedChanged();
    void peerDiscovered(const QVariantMap& peerInfo);
    void peerConnected(const QVariantMap& peerInfo);
    void peerDisconnected(const QString& peerId);
    void transferStarted(const QVariantMap& transferInfo);
    void transferProgress(const QString& transferId, double progress, qint64 speed);
    void transferCompleted(const QString& transferId, const QString& fileName);
    void transferError(const QString& transferId, const QString& error);
    
private:
    void setupQmlEngine();
    void registerQmlTypes();
    void connectSignals();
    
    std::unique_ptr<QQmlApplicationEngine> m_engine;
    std::unique_ptr<P2PEngine> m_p2pEngine;
    std::unique_ptr<FileManager> m_fileManager;
    std::unique_ptr<Discovery> m_discovery;
    
    bool m_isRunning = false;
    quint16 m_listenPort = 0;
};

} // namespace p2p

#endif // P2PTRANSFER_CORE_APPLICATION_HPP
