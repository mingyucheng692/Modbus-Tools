add_library(infra_platform STATIC)

target_sources(infra_platform
    PRIVATE
        IPlatformPaths.h
        IPlatformProcessRunner.h
        PlatformEncoding.h
        PlatformProcessRunnerFactory.h
        PlatformInfo.h
        QtStandardPlatformPaths.h
        QtStandardPlatformPaths.cpp
        UnsupportedProcessRunner.h
        UnsupportedProcessRunner.cpp
)

if(WIN32)
    target_sources(infra_platform PRIVATE
        win/PlatformProcessRunnerFactory.cpp
        win/PlatformEncoding.cpp
        win/PlatformInfo.cpp
        win/Win32ProcessRunner.h
        win/Win32ProcessRunner.cpp
    )
elseif(APPLE)
    target_sources(infra_platform PRIVATE
        macos/PlatformProcessRunnerFactory.cpp
        macos/PlatformEncoding.cpp
        macos/PlatformInfo.cpp
    )
elseif(UNIX)
    target_sources(infra_platform PRIVATE
        linux/PlatformProcessRunnerFactory.cpp
        linux/PlatformEncoding.cpp
        linux/PlatformInfo.cpp
    )
else()
    message(FATAL_ERROR "infra_platform currently supports WIN32, APPLE, or UNIX toolchains only")
endif()

target_link_libraries(infra_platform
    PUBLIC
        Qt6::Core
        spdlog::spdlog
)

target_include_directories(infra_platform
    PUBLIC
        ${PROJECT_SOURCE_DIR}
)
