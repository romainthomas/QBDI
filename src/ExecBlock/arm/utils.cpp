/*
 * This file is part of QBDI.
 *
 * Copyright 2017 Quarkslab
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "llvm/MC/MCInst.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Memory.h"

#include <iterator>
#include <memory>
#include <vector>

#include "ExecBlock/utils.h"

#include "Callback.h"
#include "Patch/Types.h"
#include "Utility/memory_ostream.h"
#include "Utility/Assembly.h"


namespace QBDI {
size_t registerIndex(unsigned regId) {
  static const std::map<unsigned, size_t> regMap {
    {llvm::ARM::R0,   0},
    {llvm::ARM::R1,   1},
    {llvm::ARM::R2,   2},
    {llvm::ARM::R3,   3},
    {llvm::ARM::R4,   4},
    {llvm::ARM::R5,   5},
    {llvm::ARM::R6,   6},
    {llvm::ARM::R7,   7},
    {llvm::ARM::R8,   8},
    {llvm::ARM::R9,   9},
    {llvm::ARM::R10, 10},
    {llvm::ARM::R11, 11},
    {llvm::ARM::R12, 12},
  };
  return regMap.at(regId);
}


} // namespace QBDI
