set(ZX0_EXE ${CMAKE_SOURCE_DIR}/ZX0/win/zx0.exe)
set(SCREENS_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})

file(GLOB SCREEN_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.scr)

set(ASM_CONTENT ".syntax unified\n.cpu cortex-m0plus\n.thumb\n\n.section .rodata\n")
set(HEADER_CONTENT "#ifndef SCREENS_H\n#define SCREENS_H\n\n#include <stdint.h>\n\n#ifdef __cplusplus\nextern \"C\" {\n#endif\n\nuint8_t *dzx0_turbo(const uint8_t *src, uint8_t *dst);\n\n")
set(COMPRESSED_FILES)

foreach(SCREEN_FILE ${SCREEN_FILES})
    get_filename_component(SCREEN_NAME ${SCREEN_FILE} NAME_WE)
    string(REGEX REPLACE "[^A-Za-z0-9_]" "" SCREEN_ID "${SCREEN_NAME}")
    set(COMPRESSED_FILE ${SCREENS_BIN_DIR}/${SCREEN_NAME}.scr.zx0)

    add_custom_command(
        OUTPUT ${COMPRESSED_FILE}
        COMMAND ${ZX0_EXE} -f "${SCREEN_FILE}" "${COMPRESSED_FILE}"
        DEPENDS ${SCREEN_FILE}
        VERBATIM
    )
    list(APPEND COMPRESSED_FILES ${COMPRESSED_FILE})

    string(APPEND ASM_CONTENT "\n.global ${SCREEN_ID}\n${SCREEN_ID}:\n.incbin \"${COMPRESSED_FILE}\"\n")
    string(APPEND HEADER_CONTENT "extern const uint8_t ${SCREEN_ID}[];\n")
endforeach()

string(APPEND HEADER_CONTENT "\n#ifdef __cplusplus\n}\n#endif\n\n#endif\n")

file(GENERATE OUTPUT ${SCREENS_BIN_DIR}/screens.s CONTENT "${ASM_CONTENT}")
file(GENERATE OUTPUT ${SCREENS_BIN_DIR}/screens.h CONTENT "${HEADER_CONTENT}")

add_custom_target(screens_data DEPENDS ${COMPRESSED_FILES})

add_library(screens INTERFACE)
add_dependencies(screens screens_data)

target_sources(
    screens INTERFACE
        ${SCREENS_BIN_DIR}/screens.s
        ${CMAKE_CURRENT_SOURCE_DIR}/dzx0_turbo.s
)

target_include_directories(
    screens INTERFACE
        ${SCREENS_BIN_DIR}
)
