set(QBDI_PATCH_ARM_SRC
  "${CMAKE_CURRENT_LIST_DIR}/InstInfo.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/InstrRules.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/Layer2.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/PatchRules.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/RelocatableInst.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/InstTransform.cpp"
)

set(QBDI_PATCH_ARM_INC
  "${CMAKE_CURRENT_LIST_DIR}/InstInfo.h"
  "${CMAKE_CURRENT_LIST_DIR}/InstrRules.h"
  "${CMAKE_CURRENT_LIST_DIR}/Layer2.h"
  "${CMAKE_CURRENT_LIST_DIR}/PatchGenerator.h"
  "${CMAKE_CURRENT_LIST_DIR}/PatchRules.h"
  "${CMAKE_CURRENT_LIST_DIR}/RelocatableInst.h"
  "${CMAKE_CURRENT_LIST_DIR}/InstTransform.h"
)

target_sources(QBDI_obj
  INTERFACE
    ${QBDI_PATCH_ARM_SRC}
    ${QBDI_PATCH_ARM_INC}
)

