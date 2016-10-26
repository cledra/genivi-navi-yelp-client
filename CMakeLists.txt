cmake_minimum_required(VERSION 2.8.11)
INCLUDE(FindPkgConfig)

# usefull flags:
# -DCMAKE_BUILD_TYPE=Debug
# -DCMAKE_BUILD_TYPE=Release
# -DCMAKE_INSTALL_PREFIX=install

project(yelp-client)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC ")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5Widgets REQUIRED)

pkg_check_modules(DBUSCXX REQUIRED dbus-c++-1)
pkg_check_modules(QT5_CORE REQUIRED Qt5Core)
pkg_check_modules(JSON REQUIRED json-c)
pkg_check_modules(QT5_WIDGETS REQUIRED Qt5Widgets)
pkg_check_modules(QT5_NETWORK REQUIRED Qt5Network)

include_directories( ${DBUSCXX_INCLUDE_DIRS} ${QT5_CORE_INCLUDE_DIRS} ${QT5_WIDGETS_INCLUDE_DIRS} ${QT5_NETWORK_INCLUDE_DIRS} ${JSON_INCLUDE_DIRS} )

qt5_wrap_cpp(yelp_client_SRC MainApp.h)

add_executable( yelp-client main.cpp MainApp.cpp InfoPanel.cpp ${yelp_client_SRC} )
target_link_libraries( yelp-client geniviwrapper )
qt5_use_modules(yelp-client Core Widgets Network)

add_library( geniviwrapper libgeniviwrapper/GeniviWrapper.cpp )
target_link_libraries( geniviwrapper ${DBUSCXX_LIBRARIES} ${JSON_LIBRARIES} )

install (TARGETS yelp-client DESTINATION bin)
