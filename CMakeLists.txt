cmake_minimum_required(VERSION 3.16)

project(MEZEKSTR LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CORE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/core/include)
set(CORE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/core/src)

set(CORE_SOURCES
    ${CORE_SRC_DIR}/pcmtowav.cpp
    ${CORE_SRC_DIR}/utils.cpp
)

add_executable(MEZEKSTR
    main.cpp
    ${CORE_SOURCES}
)

target_include_directories(MEZEKSTR PRIVATE
    ${CORE_INCLUDE_DIR}
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(MEZEKSTR PRIVATE
        -Os
        -fno-ident
        -fno-exceptions
        -fno-rtti
        -ffunction-sections
        -fdata-sections
        -flto
        -Wno-deprecated
    )

    target_link_options(MEZEKSTR PRIVATE
        -s
        -Wl,--gc-sections
        -flto
    )

    if(WIN32)
        target_link_options(MEZEKSTR PRIVATE -static-libgcc -static-libstdc++)

        if(CMAKE_SIZEOF_VOID_P EQUAL 4)
            target_link_options(MEZEKSTR PRIVATE -Wl,--subsystem,console:5.01)
            set_target_properties(MEZEKSTR PROPERTIES OUTPUT_NAME "MEZEKSTR_Win_i686")
        else()
            target_link_options(MEZEKSTR PRIVATE -Wl,--subsystem,console:5.02)
            set_target_properties(MEZEKSTR PROPERTIES OUTPUT_NAME "MEZEKSTR_Win_x86-64")
        endif()
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_link_options(MEZEKSTR PRIVATE -static-libstdc++)

        if(CMAKE_SIZEOF_VOID_P EQUAL 4)
            set_target_properties(MEZEKSTR PROPERTIES OUTPUT_NAME "MEZEKSTR_Linux_i686")
        else()
            set_target_properties(MEZEKSTR PROPERTIES OUTPUT_NAME "MEZEKSTR_Linux_x86-64")
        endif()
    endif()

endif()

include(GNUInstallDirs)
install(TARGETS MEZEKSTR
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
