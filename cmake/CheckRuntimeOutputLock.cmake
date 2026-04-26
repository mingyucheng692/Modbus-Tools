if(NOT DEFINED TARGET_PATH OR TARGET_PATH STREQUAL "")
    message(FATAL_ERROR "TARGET_PATH is required")
endif()

if(NOT DEFINED TARGET_NAME OR TARGET_NAME STREQUAL "")
    set(TARGET_NAME "target")
endif()

if(NOT EXISTS "${TARGET_PATH}")
    return()
endif()

if(WIN32)
    find_program(POWERSHELL_EXECUTABLE
        NAMES powershell pwsh
    )

    if(NOT POWERSHELL_EXECUTABLE)
        message(FATAL_ERROR "PowerShell is required to check whether '${TARGET_NAME}' output is locked")
    endif()

    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}"
                -NoProfile
                -NonInteractive
                -ExecutionPolicy
                Bypass
                -File
                "${CMAKE_CURRENT_LIST_DIR}/CheckFileUnlocked.ps1"
                -Path
                "${TARGET_PATH}"
                -TargetName
                "${TARGET_NAME}"
        RESULT_VARIABLE check_result
        OUTPUT_VARIABLE check_stdout
        ERROR_VARIABLE check_stderr
    )

    if(NOT check_result EQUAL 0)
        string(STRIP "${check_stderr}" check_stderr)
        if(check_stderr STREQUAL "")
            set(check_stderr "Target '${TARGET_NAME}' output is locked: ${TARGET_PATH}")
        endif()
        message(FATAL_ERROR "${check_stderr}")
    endif()

    return()
endif()

message(VERBOSE "Skip runtime output lock check for '${TARGET_NAME}' on non-Windows platform")
