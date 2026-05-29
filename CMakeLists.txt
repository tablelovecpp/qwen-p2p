cmake_minimum_required(VERSION 3.16)
project(P2PTransfer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Network)
find_package(Threads REQUIRED)

# Source files
set(SOURCES
    src/core/application.cpp
    src/core/filemanager.cpp
    src/network/p2pengine.cpp
    src/network/discovery.cpp
    src/ui/mainview.cpp
    src/main.cpp
)

set(HEADERS
    include/core/application.hpp
    include/core/filemanager.hpp
    include/network/p2pengine.hpp
    include/network/discovery.hpp
    include/ui/mainview.hpp
)

set(QML_FILES
    qml/main.qml
    qml/components/PeerList.qml
    qml/components/FileTransferItem.qml
    qml/components/StatusBar.qml
)

# Create executable
qt_add_executable(p2ptransfer
    ${SOURCES}
    ${HEADERS}
    ${QML_FILES}
)

# Include directories
target_include_directories(p2ptransfer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(p2ptransfer PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::Network
    Threads::Threads
    stdc++
)

# Compiler options
target_compile_options(p2ptransfer PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -O2>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -O2>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /O2>
)

# Install rules
install(TARGETS p2ptransfer DESTINATION bin)
