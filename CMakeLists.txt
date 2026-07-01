cmake_minimum_required(VERSION 3.10)
project(ctorrent VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(bencode STATIC src/bencode.cpp)
target_include_directories(bencode PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(ctorrent src/main.cpp)
target_link_libraries(ctorrent PRIVATE bencode)

add_executable(test_bencode tests/test_bencode.cpp)
target_link_libraries(test_bencode PRIVATE bencode)

find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

add_library(storage STATIC src/storage.cpp)
target_include_directories(storage PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(storage PUBLIC OpenSSL::Crypto)

add_executable(test_storage tests/test_storage.cpp)
target_link_libraries(test_storage PRIVATE storage OpenSSL::Crypto)

add_library(peer_wire STATIC src/peer_wire.cpp)
target_include_directories(peer_wire PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(tracker STATIC src/tracker.cpp)
target_include_directories(tracker PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(tracker PUBLIC bencode Boost::system Threads::Threads)

add_executable(test_peer_wire tests/test_peer_wire.cpp)
target_link_libraries(test_peer_wire PRIVATE peer_wire)

add_executable(test_tracker tests/test_tracker.cpp)
target_link_libraries(test_tracker PRIVATE tracker)

enable_testing()
add_test(NAME bencode-test COMMAND test_bencode)
add_test(NAME storage-test COMMAND test_storage)
add_test(NAME peer-wire-test COMMAND test_peer_wire)
add_test(NAME tracker-test COMMAND test_tracker)
