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
#include "Patch/RelocatableInst.h"
#include <sstream>
namespace QBDI {

// RelocatableInst
// ================

RelocatableInst::RelocatableInst(llvm::MCInst inst) :
  inst(std::move(inst))
{}

llvm::MCInst RelocatableInst::reloc(ExecBlock *execBlock, CPUMode cpuMode) {
  return this->inst;
}

RelocatableInst::RegisterUsed RelocatableInst::regUsed(ExecBlock *execBlock, CPUMode cpuMode) {
  std::set<unsigned> regs;
  llvm::MCInst inst = this->reloc(execBlock, cpuMode);
  for (const llvm::MCOperand& op : inst) {
    if (op.isReg()) {
      regs.insert(op.getReg());
    }
  }

  return {std::begin(regs), std::end(regs)};
}

RelocatableInst::~RelocatableInst() = default;

// NoReloc
// =======
NoReloc::NoReloc(llvm::MCInst inst) :
  RelocatableInst(inst)
{}

llvm::MCInst NoReloc::reloc(ExecBlock *execBlock, CPUMode cpuMode) {
  return this->inst;
}

}
