set(QBDI_UTILITY_SRC
  "${CMAKE_CURRENT_LIST_DIR}/Memory.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/Assembly.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/LogSys.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/System.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/String.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/memory_ostream.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/Version.cpp"
)

set(QBDI_UTILITY_INC
  "${CMAKE_CURRENT_LIST_DIR}/LogSys.h"
  "${CMAKE_CURRENT_LIST_DIR}/Assembly.h"
  "${CMAKE_CURRENT_LIST_DIR}/Errors-private.h"
  "${CMAKE_CURRENT_LIST_DIR}/System.h"
  "${CMAKE_CURRENT_LIST_DIR}/memory_ostream.h"
  "${CMAKE_CURRENT_LIST_DIR}/String.h"
)

target_sources(QBDI_obj INTERFACE
  ${QBDI_UTILITY_SRC}
  ${QBDI_UTILITY_INC}
)


if (${OS} STREQUAL "iOS")
  include("${CMAKE_CURRENT_LIST_DIR}/ios/CMakeLists.txt")
else()
  target_sources(QBDI_obj INTERFACE
    "${CMAKE_CURRENT_LIST_DIR}/System_generic.cpp"
  )
endif()
