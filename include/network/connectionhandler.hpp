#ifndef P2PTRANSFER_NETWORK_CONNECTIONHANDLER_HPP
#define P2PTRANSFER_NETWORK_CONNECTIONHANDLER_HPP

#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <queue>
#include <memory>

namespace p2p {

/**
 * @brief Handles low-level connection management and message framing
 * 
 * Provides reliable message delivery with:
 * - Message framing and deframing
 * - Compression support
 * - Encryption readiness (TLS wrapper)
 * - Flow control
 */
class ConnectionHandler : public QObject {
    Q_OBJECT
    
public:
    explicit ConnectionHandler(QTcpSocket* socket, QObject* parent = nullptr);
    ~ConnectionHandler() override;
    
    /**
     * @brief Send a message over the connection
     * @param data Message payload
     * @return True if queued successfully
     */
    bool sendMessage(const QByteArray& data);
    
    /**
     * @brief Get the underlying socket
     * @return Socket pointer
     */
    QTcpSocket* socket() const { return m_socket; }
    
    /**
     * @brief Check if connection is valid
     * @return True if connected
     */
    bool isConnected() const;
    
    /**
     * @brief Get peer address
     * @return Peer IP address
     */
    QHostAddress peerAddress() const;
    
    /**
     * @brief Get peer port
     * @return Peer port number
     */
    quint16 peerPort() const;
    
    /**
     * @brief Enable compression for this connection
     * @param enabled Whether to enable compression
     */
    void setCompressionEnabled(bool enabled);
    
    /**
     * @brief Check if compression is enabled
     * @return True if compression is enabled
     */
    bool isCompressionEnabled() const { return m_compressionEnabled; }
    
signals:
    void messageReceived(const QByteArray& message);
    void connectionClosed();
    void errorOccurred(const QString& error);
    void bytesSent(qint64 bytes);
    void bytesReceived(qint64 bytes);
    
private slots:
    void onReadyRead();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);
    void onBytesWritten(qint64 bytes);
    void processSendQueue();
    
private:
    // Message framing
    static constexpr quint32 MESSAGE_HEADER_SIZE = 4;
    static constexpr quint32 MAX_MESSAGE_SIZE = 100 * 1024 * 1024;  // 100MB
    
    QByteArray frameMessage(const QByteArray& message);
    std::optional<QByteArray> deframeMessage();
    
    // Compression
    QByteArray compressData(const QByteArray& data);
    QByteArray decompressData(const QByteArray& data);
    
    QTcpSocket* m_socket;
    QByteArray m_receiveBuffer;
    std::queue<QByteArray> m_sendQueue;
    bool m_sending;
    bool m_compressionEnabled;
    
    qint64 m_totalBytesSent;
    qint64 m_totalBytesReceived;
};

} // namespace p2p

#endif // P2PTRANSFER_NETWORK_CONNECTIONHANDLER_HPP
