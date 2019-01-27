set(QBDI_UTILITY_IOS_SRC
  "${CMAKE_CURRENT_LIST_DIR}/System.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/server-iOS-jit-user.c"
)

set(QBDI_UTILITY_IOS_INC
  "${CMAKE_CURRENT_LIST_DIR}/server-iOS-jit-user.h"
)

target_sources(QBDI_obj INTERFACE
  ${QBDI_UTILITY_IOS_SRC}
  ${QBDI_UTILITY_IOS_INC}
)

