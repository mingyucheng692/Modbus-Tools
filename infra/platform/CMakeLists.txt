add_library(infra_platform STATIC)

target_sources(infra_platform
    PRIVATE
        IPlatformProcessRunner.h
        IPlatformProcessRunner.cpp
        PathResolver.h
        PathResolver.cpp
        PlatformInfo.h
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
