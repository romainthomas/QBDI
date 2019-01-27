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

#include "Patch/PatchGenerator.h"

namespace QBDI {

// PatchGenerator
// ==============
PatchGenerator::~PatchGenerator() = default;

bool PatchGenerator::modifyPC() {
  return false;
}

bool PatchGenerator::doNotInstrument() {
  return false;
}

// ModifyInstruction
// =================

ModifyInstruction::ModifyInstruction(InstTransform::SharedPtrVec transforms) :
  transforms(transforms)
{}

RelocatableInst::SharedPtrVec ModifyInstruction::generate(const llvm::MCInst *inst, rword address, rword instSize,
    CPUMode cpuMode, TempManager *temp_manager, const Patch *toMerge) {

    llvm::MCInst a(*inst);
    for(auto t: transforms) {
        t->transform(a, address, instSize, temp_manager);
    }

    RelocatableInst::SharedPtrVec out;
    if(toMerge != nullptr) {
        append(out, toMerge->insts);
    }
    out.push_back(NoReloc(a));
    return out;
}

// DoNotInstrument
// ===============

DoNotInstrument::DoNotInstrument() = default;

RelocatableInst::SharedPtrVec DoNotInstrument::generate(const llvm::MCInst *inst, rword address, rword instSize,
    CPUMode cpuMode, TempManager *temp_manager, const Patch *toMerge) {
    return {};
}

bool DoNotInstrument::doNotInstrument() {
  return true;
}


}
