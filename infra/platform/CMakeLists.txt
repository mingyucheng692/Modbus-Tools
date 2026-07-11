add_library(infra_platform STATIC)

target_sources(infra_platform
    PRIVATE
        IPlatformPaths.h
        IPlatformProcessRunner.h
        PathResolver.h
        PathResolver.cpp
        PlatformEncoding.h
        PlatformProcessRunnerFactory.h
        PlatformInfo.h
        QtStandardPlatformPaths.h
        UnsupportedProcessRunner.h
        UnsupportedProcessRunner.cpp
)

if(WIN32)
    target_sources(infra_platform PRIVATE
        win/Win32ProcessRunner.h
        win/Win32ProcessRunner.cpp
    )
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
