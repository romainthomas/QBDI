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
#include "Patch/Patch.h"
namespace QBDI {

Patch::Patch() {
  metadata.patchSize = 0;
  metadata.cpuMode = (CPUMode) 0;
}

Patch::Patch(llvm::MCInst inst, rword address, rword instSize, CPUMode cpuMode) {
  metadata.patchSize = 0;
  metadata.cpuMode = cpuMode;
  setInst(inst, address, instSize);
}

void Patch::setMerge(bool merge) {
    metadata.merge = merge;
}

void Patch::setModifyPC(bool modifyPC) {
    metadata.modifyPC = modifyPC;
}

void Patch::setInst(llvm::MCInst inst, rword address, rword instSize) {
    metadata.inst = inst;
    metadata.address = address;
    metadata.instSize = instSize;
}

void Patch::append(const RelocatableInst::SharedPtrVec v) {
    insts.insert(insts.end(), v.begin(), v.end());
    metadata.patchSize += v.size();
}

void Patch::prepend(const RelocatableInst::SharedPtrVec v) {
    insts.insert(insts.begin(), v.begin(), v.end());
    metadata.patchSize += v.size();
}

void Patch::append(const RelocatableInst::SharedPtr r) {
    insts.push_back(r);
    metadata.patchSize += 1;
}

void Patch::prepend(const RelocatableInst::SharedPtr r) {
    insts.insert(insts.begin(), r);
    metadata.patchSize += 1;
}
}
