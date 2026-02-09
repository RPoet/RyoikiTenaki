# =============================================================================
# Sinkansoai CMakeLists.txt - Android Build Configuration
# =============================================================================
# This CMake file is used ONLY for Android NDK builds.
# Windows builds continue to use the existing vcxproj.
# =============================================================================

cmake_minimum_required(VERSION 3.22.1)

project(Sinkansoai VERSION 1.0.0 LANGUAGES CXX)

# =============================================================================
# C++ Standard
# =============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =============================================================================
# Platform Detection
# =============================================================================
if(ANDROID)
    message(STATUS "Building for Android")
    add_definitions(-DPLATFORM_ANDROID=1)
    add_definitions(-DPLATFORM_WINDOWS=0)
else()
    message(FATAL_ERROR "This CMakeLists.txt is only for Android builds. Use vcxproj for Windows.")
endif()

# =============================================================================
# Source Files - Platform Independent
# =============================================================================
set(SOURCES_COMMON
    # Core
    Sources/Component.cpp
    Sources/Engine.cpp
    Sources/Input.cpp
    Sources/Launcher.cpp
    Sources/Module.cpp
    Sources/ObjectBase.cpp
    Sources/Timer.cpp
    Sources/World.cpp
    Sources/SceneSelection.cpp
    
    # Engine
    Sources/Engine/Mesh.cpp
    Sources/Engine/Texture.cpp
    
    # Entities
    Sources/Entities/Camera.cpp
    Sources/Entities/PlaceableEntity.cpp
    Sources/Entities/Components/Transform.cpp
    
    # EntityComponent
    Sources/EntityComponent/ComponentManger.cpp
    Sources/EntityComponent/Entity.cpp
    Sources/EntityComponent/EntityManager.cpp
    
    # Module
    Sources/Module/Mesh/MeshBuilder.cpp
    Sources/Module/Mesh/MeshBuilderGLTF.cpp
    Sources/Module/Texture/TextureBuilder.cpp
    
    # Render
    Sources/Render/Renderer.cpp
    Sources/Render/Scene.cpp
    
    # RenderBackend Common
    Sources/RenderBackend/DynamicBuffer.cpp
    Sources/RenderBackend/RenderBackend.cpp
    Sources/RenderBackend/RenderCommandList.cpp
    Sources/RenderBackend/ResourceBuffer.cpp
    Sources/RenderBackend/Texture.cpp
    
    # RenderBackend Vulkan
    Sources/RenderBackend/RenderBackendVulkan/RenderBackendVulkan.cpp
    
    # RenderBackend SIMD (CPU Software Rasterizer)
    Sources/RenderBackend/RenderBackendSIMD/RenderBackendSIMD.cpp
    
    # RenderBackend Compute
    Sources/RenderBackend/RenderBackendCompute/RenderBackendCompute.cpp
    
    # SceneGraph
    Sources/SceneGraphSystem/Component.cpp
    Sources/SceneGraphSystem/GraphEntity.cpp
    Sources/SceneGraphSystem/SceneGraphSystem.cpp
    
    # Tasks
    Sources/Tasks/TaskSystem.cpp
)

# =============================================================================
# Source Files - Android Specific
# =============================================================================
set(SOURCES_ANDROID
    Sources/Platform/Android/MainAndroid.cpp
)

# =============================================================================
# Include Directories
# =============================================================================
set(INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/Sources
    ${CMAKE_SOURCE_DIR}/Sources/ThirdParty/json
)

# =============================================================================
# Android Build Configuration
# =============================================================================
if(ANDROID)
    # Find Vulkan
    find_package(Vulkan REQUIRED)
    
    # Create shared library for Android
    add_library(${PROJECT_NAME} SHARED
        ${SOURCES_COMMON}
        ${SOURCES_ANDROID}
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
    )
    
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${INCLUDE_DIRS}
        ${ANDROID_NDK}/sources/android/native_app_glue
        ${Vulkan_INCLUDE_DIRS}
    )
    
    target_link_libraries(${PROJECT_NAME}
        android
        log
        Vulkan::Vulkan
    )
    
    # Set library output name
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LIBRARY_OUTPUT_NAME "sinkansoai"
    )
endif()

# =============================================================================
# Compiler Flags
# =============================================================================
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wno-unused-parameter
        -Wno-missing-field-initializers
    )
endif()
