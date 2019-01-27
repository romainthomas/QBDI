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

#include "Patch/PatchRule.h"

#if defined(QBDI_ARCH_X86_64) || defined(QBDI_ARCH_X86)
#include "Patch/x86-64/PatchRules.h"
#elif defined(QBDI_ARCH_ARM)
#include "Patch/arm/PatchRules.h"
#endif

namespace QBDI {
PatchRule::PatchRule(PatchCondition::SharedPtr condition, PatchGenerator::SharedPtrVec generators)
  : condition(condition), generators(generators)
{}

bool PatchRule::canBeApplied(const llvm::MCInst *inst, rword address, rword instSize, LLVMCPU* llvmCPU) {
    return condition->test(inst, address, instSize, llvmCPU->getMII());
}

Patch PatchRule::generate(const llvm::MCInst *inst, rword address, rword instSize, CPUMode cpuMode, LLVMCPU* llvmCPU, const Patch* toMerge) {
    Patch patch(*inst, address, instSize, cpuMode);
    if(toMerge != nullptr) {
        patch.metadata.address = toMerge->metadata.address;
        patch.metadata.instSize += toMerge->metadata.instSize;
    }
    TempManager tempManager(inst, llvmCPU->getMII(), llvmCPU->getMRI());
    bool modifyPC = false;
    bool merge = false;

    for(auto g : generators) {
        patch.append(g->generate(inst, address, instSize, cpuMode, &tempManager, toMerge));
        modifyPC |= g->modifyPC();
        merge |= g->doNotInstrument();
    }
    patch.setMerge(merge);
    patch.setModifyPC(modifyPC);

    Reg::Vec usedRegisters = tempManager.getUsedRegisters();

    for(unsigned int i = 0; i < usedRegisters.size(); i++) {
        patch.prepend(SaveReg(usedRegisters[i], Offset(usedRegisters[i])).generate(cpuMode));
    }

    for(unsigned int i = 0; i < usedRegisters.size(); i++) {
        patch.append(LoadReg(usedRegisters[i], Offset(usedRegisters[i])).generate(cpuMode));
    }

    return patch;
}
}
