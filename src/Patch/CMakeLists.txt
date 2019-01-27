set(QBDI_PATCH_SRC
  "${CMAKE_CURRENT_LIST_DIR}/InstrRules.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/InstrRule.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/InstTransform.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/Patch.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/PatchGenerator.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/PatchRule.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/PatchCondition.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/RelocatableInst.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/TempManager.cpp"
)

set(QBDI_PATCH_INC
  "${CMAKE_CURRENT_LIST_DIR}/InstInfo.h"
  "${CMAKE_CURRENT_LIST_DIR}/InstrRule.h"
  "${CMAKE_CURRENT_LIST_DIR}/InstrRules.h"
  "${CMAKE_CURRENT_LIST_DIR}/InstTransform.h"
  "${CMAKE_CURRENT_LIST_DIR}/Patch.h"
  "${CMAKE_CURRENT_LIST_DIR}/PatchCondition.h"
  "${CMAKE_CURRENT_LIST_DIR}/PatchGenerator.h"
  "${CMAKE_CURRENT_LIST_DIR}/PatchRule.h"
  "${CMAKE_CURRENT_LIST_DIR}/PatchUtils.h"
  "${CMAKE_CURRENT_LIST_DIR}/RelocatableInst.h"
  "${CMAKE_CURRENT_LIST_DIR}/Types.h"
  "${CMAKE_CURRENT_LIST_DIR}/TempManager.h"
)

target_sources(QBDI_obj INTERFACE
  ${QBDI_PATCH_SRC}
  ${QBDI_PATCH_INC}
)


if (${ARCH} STREQUAL "X86_64")
  include("${CMAKE_CURRENT_LIST_DIR}/x86-64/CMakeLists.txt")

elseif (${ARCH} STREQUAL "ARM")
  include("${CMAKE_CURRENT_LIST_DIR}/arm/CMakeLists.txt")

else()
  message(FATAL_ERROR "Missing patch for ${ARCH}")
endif()
