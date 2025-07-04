cmake_minimum_required(VERSION 3.22)

if(APPLE)
    # Fix runtime path for installed executables
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
endif()

project(doppel)

if(MSVC)
    add_compile_options("$<$<CONFIG:Debug>:/MTd>$<$<CONFIG:Release>:/MT>")
    add_compile_options("$<$<CONFIG:Debug>:/ZI>$<$<CONFIG:Release>:/Zi>")
    add_compile_options(/FS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /Qpar")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB IMGUI_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/include/imgui/imgui.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/imgui/imgui_demo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/imgui/imgui_draw.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/imgui/imgui_tables.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/imgui/imgui_widgets.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/imgui/backends/imgui_impl_glfw.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/imgui/backends/imgui_impl_opengl3.cpp"
)

set(INCLUDE_DIRS 
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/json"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/physx"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/physx/characterkinematic"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/physx/cooking"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/imgui"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/imgui/backends"
)

set(LINK_LIBRARIES
    debug PhysXCooking_64_d optimized PhysXCooking_64
    debug glfw3d optimized glfw3
    debug glew32sd optimized glew32s
    debug GCG_GL_Lib_Debug optimized GCG_GL_Lib_Release
    OpenGL::GL
    debug PhysXCommon_64_d optimized PhysXCommon_64
    debug PhysX_64_d optimized PhysX_64
    debug PhysXFoundation_64_d optimized PhysXFoundation_64
    debug PhysXExtensions_static_64_d optimized PhysXExtensions_static_64
    debug PhysXPvdSDK_static_64_d optimized PhysXPvdSDK_static_64
    debug PhysXCharacterKinematic_static_64_d optimized PhysXCharacterKinematic_static_64
    SFML::Audio
)

if(UNIX AND NOT APPLE)
    set(LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/linux" ".")
elseif(UNIX AND APPLE)
    set(LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/macos" ".")
elseif(WIN32)
    set(LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/windows" ".")
endif()

find_package(OpenGL REQUIRED)
set(SFML_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/SFML/lib/cmake/SFML")
find_package(SFML 3 COMPONENTS Audio REQUIRED)

file(GLOB SOURCES "src/*.cpp" "src/*.h"  "src/**/*.cpp" "src/**/*.h")
add_executable(${PROJECT_NAME} ${SOURCES} ${IMGUI_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${LIBRARY_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LINK_LIBRARIES})

# IDE-specific settings
if(CMAKE_GENERATOR MATCHES "Visual Studio")
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
    set_property(DIRECTORY "${CMAKE_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
elseif(CMAKE_GENERATOR MATCHES "Xcode")
    set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_GENERATE_SCHEME TRUE CMAKE_XCODE_SCHEME_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
    set_property(DIRECTORY "${CMAKE_SOURCE_DIR}" PROPERTY XCODE_STARTUP_PROJECT ${PROJECT_NAME})
endif()

if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${X11_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${X11_INCLUDE_DIR})
endif()

if(APPLE)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Cocoa -framework IOKit")
endif()

# # DLL copy logic
file(GLOB PHYSX_DLLS "${CMAKE_CURRENT_SOURCE_DIR}/include/physx//dll/*.dll")
file(GLOB SFML_DLLS "${CMAKE_CURRENT_SOURCE_DIR}/include/SFML/bin/*.dll")

add_custom_target(copy_dlls ALL
    COMMENT "Copying required DLLs to output directory"$<TARGET_FILE_DIR:${PROJECT_NAME}>
)

foreach(DLL_FILE ${PHYSX_DLLS})
    add_custom_command(TARGET copy_dlls POST_BUILD
     COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DLL_FILE}"
       $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endforeach()

foreach(DLL_FILE ${SFML_DLLS})
    add_custom_command(TARGET copy_dlls POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DLL_FILE}"
       $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endforeach()

add_dependencies(${PROJECT_NAME} copy_dlls)

# Install rules
install(TARGETS ${PROJECT_NAME} CONFIGURATIONS Debug DESTINATION "debug")
install(TARGETS ${PROJECT_NAME} CONFIGURATIONS Release DESTINATION "release")
