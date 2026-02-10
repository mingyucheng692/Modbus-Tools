add_library(core STATIC)

# We will add source files here later
target_sources(core PRIVATE
    logging/Logger.cpp
)

# Create dummy Logger.cpp and Logger.h to make it compile for now
file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/logging/Logger.h "#pragma once\n")
file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/logging/Logger.cpp "#include \"Logger.h\"\n")

target_link_libraries(core PUBLIC
    Qt6::Core
    Qt6::Network
    Qt6::SerialPort
    spdlog::spdlog
)

target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
