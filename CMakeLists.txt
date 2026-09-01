cmake_minimum_required(VERSION 3.10)

# Locate VITASDK directory BEFORE project()
if(NOT DEFINED VITASDK)
    if(DEFINED ENV{VITASDK})
        set(VITASDK $ENV{VITASDK})
    else()
        set(VITASDK "/usr/local/vitasdk")
    endif()
endif()

if(EXISTS "${VITASDK}/share/vita.cmake")
    include("${VITASDK}/share/vita.cmake")
endif()

project(GrooveSpeedVita C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(VITA_APP_NAME "GrooveSpeed")
set(VITA_TITLEID "GROOVE001")
set(VITA_VERSION "01.00")

# Prioritize local ImGui 1.92 headers over legacy VITASDK 1.61 system headers
include_directories(BEFORE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${VITASDK}/arm-vita-eabi/include/SDL2
)

# Source files
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.c"
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -O3
    -ffast-math
    -ffunction-sections
    -fdata-sections
)

target_link_options(${PROJECT_NAME} PRIVATE
    -Wl,--gc-sections
)

target_link_libraries(${PROJECT_NAME}
    -Wl,--start-group
    SDL2
    SDL2_image
    mathneon
    SceMotion_stub
    SceDisplay_stub
    SceGxm_stub
    SceCtrl_stub
    SceTouch_stub
    SceHid_stub
    SceIme_stub
    ScePower_stub
    SceCommonDialog_stub
    SceSysmodule_stub
    SceAppMgr_stub
    SceAppUtil_stub
    SceAudio_stub
    SceAudioIn_stub
    SceKernelDmacMgr_stub
    m
    -Wl,--end-group
)

vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME})

# Bundle VPK with LiveArea icon0.png and bg.png if present
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vpk/icon0.png")
    vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
        NAME ${VITA_APP_NAME}
        VERSION ${VITA_VERSION}
        ICON0 ${CMAKE_CURRENT_SOURCE_DIR}/vpk/icon0.png
        BG ${CMAKE_CURRENT_SOURCE_DIR}/vpk/bg.png
        STARTUP ${CMAKE_CURRENT_SOURCE_DIR}/vpk/startup.png
        TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/vpk/template.xml
    )
else()
    vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
        NAME ${VITA_APP_NAME}
        VERSION ${VITA_VERSION}
    )
endif()
