#ifndef P2PTRANSFER_CORE_PEERINFO_HPP
#define P2PTRANSFER_CORE_PEERINFO_HPP

#include <QString>
#include <QHostAddress>
#include <chrono>

namespace p2p {

/**
 * @brief Represents information about a peer in the P2P network
 */
struct PeerInfo {
    QString id;
    QString name;
    QHostAddress address;
    quint16 port;
    qint64 transferSpeed;      // bytes per second
    qint64 totalTransferred;   // total bytes transferred
    bool isOnline;
    std::chrono::steady_clock::time_point lastSeen;
    
    PeerInfo() 
        : port(0)
        , transferSpeed(0)
        , totalTransferred(0)
        , isOnline(false)
        , lastSeen(std::chrono::steady_clock::now()) {}
    
    PeerInfo(const QString& peerId, const QString& peerName, 
             const QHostAddress& addr, quint16 p)
        : id(peerId)
        , name(peerName)
        , address(addr)
        , port(p)
        , transferSpeed(0)
        , totalTransferred(0)
        , isOnline(true)
        , lastSeen(std::chrono::steady_clock::now()) {}
    
    bool operator==(const PeerInfo& other) const {
        return id == other.id;
    }
    
    bool operator!=(const PeerInfo& other) const {
        return !(*this == other);
    }
};

} // namespace p2p

#endif // P2PTRANSFER_CORE_PEERINFO_HPP
