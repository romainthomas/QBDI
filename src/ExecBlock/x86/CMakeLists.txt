# =======================
# Stub
# =======================
set(QBDI_EXEC_BLOCK_STUB)

if (${OS} STREQUAL "macOS" OR ${OS} STREQUAL "linux")
  list(APPEND QBDI_EXEC_BLOCK_STUB ${CMAKE_CURRENT_LIST_DIR}/linux-osx.s)
else()
  message(FATAL_ERROR "No stub for ${OS} (${ARCH})")
endif()

target_sources(QBDI_obj INTERFACE
  ${QBDI_EXEC_BLOCK_STUB}
)

# =======================
# Other
# =======================
set(QBDI_EXEC_BLOCK_OTHER
  ${CMAKE_CURRENT_LIST_DIR}/utils.cpp)
)

target_sources(QBDI_obj INTERFACE
  ${QBDI_EXEC_BLOCK_OTHER}
)
