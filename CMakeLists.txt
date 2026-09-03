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
    -Wl,-q
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

# -------------------------------------------------------------
# VitaSDK Standard Packaging: ELF -> eboot.bin -> VPK
# -------------------------------------------------------------
vita_create_self(eboot.bin ${PROJECT_NAME} STRIPPED)

vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} eboot.bin
    VERSION ${VITA_VERSION}
    NAME ${VITA_APP_NAME}
    FILE ${CMAKE_CURRENT_SOURCE_DIR}/vpk/icon0.png sce_sys/icon0.png
    FILE ${CMAKE_CURRENT_SOURCE_DIR}/vpk/bg.png sce_sys/livearea/contents/bg.png
    FILE ${CMAKE_CURRENT_SOURCE_DIR}/vpk/startup.png sce_sys/livearea/contents/startup.png
    FILE ${CMAKE_CURRENT_SOURCE_DIR}/vpk/template.xml sce_sys/livearea/contents/template.xml
    FILE ${CMAKE_CURRENT_SOURCE_DIR}/vpk/web_button.png sce_sys/livearea/contents/web_button.png
    FILE ${CMAKE_CURRENT_SOURCE_DIR}/vpk/bgm.at9 sce_sys/livearea/contents/bgm.at9
    FILE ${CMAKE_CURRENT_SOURCE_DIR}/vpk/manual/001.png sce_sys/manual/001.png
    FILE ${CMAKE_CURRENT_SOURCE_DIR}/vpk/manual/002.png sce_sys/manual/002.png
    FILE ${CMAKE_CURRENT_SOURCE_DIR}/vpk/manual/003.png sce_sys/manual/003.png
    FILE ${CMAKE_CURRENT_SOURCE_DIR}/vpk/manual/004.png sce_sys/manual/004.png
)

# Mirror output VPK to project root directory
if(TARGET ${PROJECT_NAME}.vpk-vpk)
    add_custom_command(TARGET ${PROJECT_NAME}.vpk-vpk POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.vpk"
            "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.vpk"
        COMMENT "Mirroring ${PROJECT_NAME}.vpk to project root"
    )
elseif(TARGET ${PROJECT_NAME}.vpk)
    add_custom_command(TARGET ${PROJECT_NAME}.vpk POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.vpk"
            "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.vpk"
        COMMENT "Mirroring ${PROJECT_NAME}.vpk to project root"
    )
endif()
