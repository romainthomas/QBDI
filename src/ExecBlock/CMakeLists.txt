set(QBDI_EXEC_BLOCK_SRC
  "${CMAKE_CURRENT_LIST_DIR}/ExecBlockManager.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/ExecBlock.cpp"
)

set(QBDI_EXEC_BLOCK_INC
  "${CMAKE_CURRENT_LIST_DIR}/Context.h"
  "${CMAKE_CURRENT_LIST_DIR}/ExecBlock.h"
  "${CMAKE_CURRENT_LIST_DIR}/ExecBlockManager.h"
  "${CMAKE_CURRENT_LIST_DIR}/Context.h"
)

target_sources(QBDI_obj INTERFACE
  ${QBDI_EXEC_BLOCK_SRC}
  ${QBDI_EXEC_BLOCK_INC}
)


if (${ARCH} STREQUAL "X86_64")
  include("${CMAKE_CURRENT_LIST_DIR}/x86-64/CMakeLists.txt")

elseif (${ARCH} STREQUAL "X86")
  include("${CMAKE_CURRENT_LIST_DIR}/x86/CMakeLists.txt")

elseif (${ARCH} STREQUAL "ARM")
  include("${CMAKE_CURRENT_LIST_DIR}/arm/CMakeLists.txt")

else()
  message(FATAL_ERROR "No stub for architecture ${ARCH}")
endif()
