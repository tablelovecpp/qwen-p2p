# P2P File Transfer

A modern, high-performance peer-to-peer file transfer application built with C++23 and Qt6/QML.

## Features

### Core Capabilities
- **Direct P2P Transfers**: No central server required - files transfer directly between peers
- **Automatic Peer Discovery**: UDP broadcast/multicast discovery on local network
- **High Performance**: Optimized for speed with 64KB chunk transfers and async I/O
- **Modern UI**: Beautiful Material Design interface using Qt Quick Controls 2
- **Cross-Platform**: Works on Windows, macOS, and Linux

### Technical Highlights
- **C++23 Features**: Uses latest C++ standards for optimal performance
- **Qt6 Framework**: Leverages Qt6's modern APIs and QML integration
- **Secure Transfers**: SHA-256 file verification
- **Resume Support**: Interrupted transfers can be resumed
- **Flow Control**: Built-in congestion avoidance

### User Interface
- Clean, intuitive Material Design interface
- Real-time transfer progress and speed monitoring
- Dark/Light theme support
- Responsive layout for various screen sizes
- System tray integration (planned)

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│                      QML UI Layer                        │
│  ┌─────────────┬─────────────┬─────────────────────┐   │
│  │   Peers     │  Transfers  │     Settings        │   │
│  └─────────────┴─────────────┴─────────────────────┘   │
├─────────────────────────────────────────────────────────┤
│                   Application Controller                 │
│  ┌─────────────┬─────────────┬─────────────────────┐   │
│  │  P2PEngine  │ FileManager │    Discovery        │   │
│  └─────────────┴─────────────┴─────────────────────┘   │
├─────────────────────────────────────────────────────────┤
│                    Network Layer                         │
│  ┌─────────────┬─────────────┬─────────────────────┐   │
│  │   TCP/IP    │    UDP      │   Connection Mgmt   │   │
│  └─────────────┴─────────────┴─────────────────────┘   │
└─────────────────────────────────────────────────────────┘
```

## Building

### Prerequisites

- **Compiler**: GCC 13+, Clang 16+, or MSVC 2022
- **CMake**: 3.28 or higher
- **Qt**: 6.5 or higher with modules:
  - Qt Core
  - Qt Gui
  - Qt Qml
  - Qt Quick
  - Qt Network
  - Qt Quick Controls 2

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/yourusername/p2ptransfer.git
cd p2ptransfer

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . -j$(nproc)

# Install (optional)
sudo cmake --install .
```

### Platform-Specific Notes

#### Linux
```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install qt6-base-dev qt6-declarative-dev \
                     qt6-quick-controls2-dev cmake g++

# Install dependencies (Fedora)
sudo dnf install qt6-qtbase-devel qt6-qtdeclarative-devel \
                 qt6-qtquickcontrols2-devel cmake gcc-c++
```

#### macOS
```bash
# Install via Homebrew
brew install qt@6 cmake

# Link Qt
export PATH="/opt/homebrew/opt/qt@6/bin:$PATH"
```

#### Windows
```powershell
# Using vcpkg
vcpkg install qtbase:x64-windows qtdeclarative:x64-windows
vcpkg integrate install

# Then run CMake with Visual Studio generator
cmake -G "Visual Studio 17 2022" -A x64 ..
```

## Usage

### Starting the Application

1. Launch `p2ptransfer` executable
2. The application automatically starts listening on port 47473
3. Peers on the same network will be discovered automatically

### Sending Files

1. Navigate to the "Peers" tab
2. Select a peer from the discovered list
3. Click the "Send File" button (📤)
4. Choose the file(s) to send
5. Monitor progress in the "Transfers" tab

### Receiving Files

1. When a peer sends you a file, you'll receive a notification
2. Accept the transfer request
3. Files are saved to `~/Downloads/P2PTransfer` by default
4. Monitor progress in the "Transfers" tab

### Manual Connection

If automatic discovery doesn't work:
1. Go to Settings → Network
2. Note the IP address and port
3. Share these with the peer you want to connect to
4. Use "Connect to Peer" and enter the details

## Configuration

### Network Settings

| Setting | Default | Description |
|---------|---------|-------------|
| Listen Port | 47473 | TCP port for incoming connections |
| Discovery Port | 47474 | UDP port for peer discovery |
| Max Connections | 10 | Maximum simultaneous peer connections |
| Multicast | Off | Enable multicast discovery |

### Transfer Settings

| Setting | Default | Description |
|---------|---------|-------------|
| Download Location | ~/Downloads/P2PTransfer | Where received files are saved |
| Chunk Size | 64 KB | Data chunk size for transfers |
| Transfer Limit | Unlimited | Bandwidth throttling |

## Protocol

### Message Format

```
┌──────────┬──────────────┬─────────────────┐
│ Type(1B) │ Length(4B)   │ Payload(NB)     │
└──────────┴──────────────┴─────────────────┘
```

### Message Types

| Type | Code | Description |
|------|------|-------------|
| Handshake | 0x01 | Initial connection setup |
| FileInfo | 0x03 | File metadata exchange |
| FileRequest | 0x04 | Request file from peer |
| FileData | 0x05 | File data chunk |
| FileAck | 0x06 | Acknowledge received chunk |
| TransferComplete | 0x07 | Transfer finished |
| KeepAlive | 0x08 | Connection heartbeat |
| Error | 0xFF | Error message |

### Discovery Protocol

Uses JSON-based UDP packets:
```json
{
  "magic": "P2PDISC",
  "version": "1.0",
  "peerId": "uuid-string",
  "peerName": "hostname",
  "servicePort": 47473,
  "timestamp": 1234567890
}
```

## Performance

### Benchmarks

Typical performance on Gigabit LAN:
- Small files (< 10MB): ~80-90 MB/s
- Large files (> 1GB): ~100-110 MB/s
- Multiple concurrent transfers: ~90% link utilization

### Optimization Tips

1. **Wired Connection**: Use Ethernet for best performance
2. **Same Subnet**: Keep peers on the same network segment
3. **Disable Antivirus Scan**: Temporarily disable for large transfers
4. **SSD Storage**: Faster read/write improves throughput

## Security Considerations

- **Local Network Only**: Designed for trusted local networks
- **No Encryption**: Traffic is not encrypted (planned for future)
- **File Verification**: SHA-256 hashes verify file integrity
- **Firewall**: Ensure ports 47473 (TCP) and 47474 (UDP) are open

## Troubleshooting

### Common Issues

**No peers discovered:**
- Check firewall settings
- Verify both peers are on same subnet
- Try manual connection

**Slow transfer speeds:**
- Check network congestion
- Verify disk I/O isn't bottleneck
- Reduce number of concurrent transfers

**Connection refused:**
- Ensure application is running on target peer
- Check if correct port is being used
- Verify no other application is using the port

## Development

### Project Structure

```
p2ptransfer/
├── CMakeLists.txt          # Build configuration
├── include/
│   ├── core/               # Core business logic headers
│   ├── network/            # Network layer headers
│   └── ui/                 # UI controller headers
├── src/
│   ├── core/               # Core implementations
│   ├── network/            # Network implementations
│   ├── ui/                 # UI implementations
│   └── main.cpp            # Application entry point
├── qml/
│   ├── main.qml            # Main application window
│   └── components/         # Reusable QML components
└── resources/              # Assets and icons
```

### Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Run tests
5. Submit a pull request

## License

MIT License - See LICENSE file for details

## Acknowledgments

- Qt Company for the excellent Qt framework
- C++ community for modern language features
- All contributors and users

---

**Version**: 1.0.0  
**Build Date**: 2024  
**Author**: P2P Transfer Team
