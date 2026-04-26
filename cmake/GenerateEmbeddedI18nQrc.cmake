if(NOT DEFINED OUTPUT_FILE OR OUTPUT_FILE STREQUAL "")
    message(FATAL_ERROR "OUTPUT_FILE is required")
endif()

if(NOT DEFINED QM_FILES_CMAKE OR QM_FILES_CMAKE STREQUAL "")
    message(FATAL_ERROR "QM_FILES_CMAKE is required")
endif()

include("${QM_FILES_CMAKE}")

if(NOT DEFINED QM_FILES OR QM_FILES STREQUAL "")
    message(FATAL_ERROR "QM_FILES list is empty")
endif()

set(qrc_content "<RCC>\n    <qresource prefix=\"/i18n\">\n")

foreach(qm_file IN LISTS QM_FILES)
    if(NOT EXISTS "${qm_file}")
        message(FATAL_ERROR "QM file does not exist: ${qm_file}")
    endif()

    get_filename_component(qm_name "${qm_file}" NAME)
    file(TO_CMAKE_PATH "${qm_file}" qm_path)
    string(APPEND qrc_content
        "        <file alias=\"${qm_name}\">${qm_path}</file>\n")
endforeach()

string(APPEND qrc_content "    </qresource>\n</RCC>\n")
file(WRITE "${OUTPUT_FILE}" "${qrc_content}")
