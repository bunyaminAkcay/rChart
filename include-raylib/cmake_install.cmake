# Install script for directory: /home/bunyamin/Documents/Repos/rChart/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/home/bunyamin/Documents/Repos/rChart/include/AudioDevice.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/AudioStream.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/AutomationEventList.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/BoundingBox.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/Camera2D.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/Camera3D.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/Color.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/FileData.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/FileText.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/Font.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/Functions.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/Gamepad.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/Image.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/Keyboard.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/Material.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/Matrix.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/Mesh.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/MeshUnmanaged.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/Model.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/ModelAnimation.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/Mouse.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/Music.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/Ray.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/RayCollision.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/RaylibException.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/raylib-cpp-utils.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/raylib-cpp.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/raylib.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/raymath.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/Rectangle.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/RenderTexture.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/ShaderUnmanaged.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/Shader.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/Sound.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/Text.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/Texture.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/TextureUnmanaged.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/Touch.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/Vector2.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/Vector3.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/Vector4.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/VrStereoConfig.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/Wave.hpp"
    "/home/bunyamin/Documents/Repos/rChart/include/Window.hpp"
    )
endif()

