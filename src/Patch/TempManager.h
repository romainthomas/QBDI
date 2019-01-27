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
#ifndef TEMP_MANAGER_H
#define TEMP_MANAGER_H

#include <memory>
#include <utility>
#include <vector>
#include <iostream>

#include "Platform.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "Patch/Types.h"
#include "Utility/LogSys.h"

#if defined(QBDI_ARCH_X86_64) || defined(QBDI_ARCH_X86)
// skip RAX as it is very often used implicitly and LLVM
// sometimes don't tell us...
#define _QBDI_FIRST_FREE_REGISTER 1
#else
#define _QBDI_FIRST_FREE_REGISTER 0
#endif

namespace QBDI {

class TempManager {
  // Assign temp_id to reg_id
  std::map<uint32_t, uint32_t> temps;
  const llvm::MCInst* inst;
  llvm::MCInstrInfo* MCII;
  llvm::MCRegisterInfo* MRI;

  public:

    TempManager(const llvm::MCInst *inst, llvm::MCInstrInfo* MCII, llvm::MCRegisterInfo *MRI);

    Reg getRegForTemp(unsigned int temp_id);

    Reg::Vec getUsedRegisters();

    size_t getUsedRegisterNumber();

    unsigned getRegSize(unsigned reg);

    unsigned getSizedSubReg(unsigned reg, unsigned size);
};

}

#endif
