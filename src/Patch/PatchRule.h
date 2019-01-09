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
#ifndef PATCHRULES_H
#define PATCHRULES_H

#include <memory>
#include <vector>

#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCRegisterInfo.h"

#include "Engine/LLVMCPU.h"
#include "Patch/PatchGenerator.h"
#include "Patch/PatchCondition.h"
#include "Patch/Patch.h"
#include "Platform.h"

#if defined(QBDI_ARCH_X86_64) || defined(QBDI_ARCH_X86)
#include "Patch/X86_64/PatchRules_X86_64.h"
#elif defined(QBDI_ARCH_ARM)
#include "Patch/ARM/PatchRules_ARM.h"
#endif

namespace QBDI {

/*! A patch rule written in PatchDSL.
*/
class PatchRule : public AutoAlloc<PatchRule, PatchRule> {
    PatchCondition::SharedPtr     condition;
    PatchGenerator::SharedPtrVec  generators;

public:

    using SharedPtr    = std::shared_ptr<PatchRule>;
    using SharedPtrVec = std::vector<std::shared_ptr<PatchRule>>;

    /*! Allocate a new patch rule with a condition and a list of generators.
     *
     * @param[in] condition   A PatchCondition which determine wheter or not this PatchRule applies.
     * @param[in] generators  A vector of PatchGenerator which will produce the patch instructions.
    */
    PatchRule(PatchCondition::SharedPtr condition, PatchGenerator::SharedPtrVec generators)
        : condition(condition), generators(generators) {};

    /*! Determine wheter this rule applies by evaluating this rule condition on the current
     *  context.
     *
     * @param[in] inst      The current instruction.
     * @param[in] address   The current instruction address.
     * @param[in] instSize  The current instruction size.
     * @param[in] llvmCPU   The LLVM CPU instance
     *
     * @return True if this patch condition evaluate to true on this context.
    */
    bool canBeApplied(const llvm::MCInst *inst, rword address, rword instSize, LLVMCPU* llvmCPU) {
        return condition->test(inst, address, instSize, llvmCPU->getMII());
    }

    /*! Generate this rule output patch by evaluating its generators on the current context. Also
     *  handles the temporary register management for this patch.
     *
     * @param[in] inst      The current instruction.
     * @param[in] address   The current instruction address.
     * @param[in] instSize  The current instruction size.
     * @param[in] llvmCPU   The LLVM CPU instance
     * @param[in] toMerge   An eventual previous patch which is to be merged with the current 
     *                      instruction.
     *
     * @return A Patch which is composed of the input context and a series of RelocatableInst.
    */
    Patch generate(const llvm::MCInst *inst, rword address, rword instSize, CPUMode cpuMode, LLVMCPU* llvmCPU, const Patch* toMerge = nullptr) {
        Patch patch(*inst, address, instSize, cpuMode);
        if(toMerge != nullptr) {
            patch.metadata.address = toMerge->metadata.address;
            patch.metadata.instSize += toMerge->metadata.instSize;
        }
        TempManager temp_manager(inst, llvmCPU->getMII(), llvmCPU->getMRI());
        bool modifyPC = false;
        bool merge = false;

        for(auto g : generators) {
            patch.append(g->generate(inst, address, instSize, cpuMode, &temp_manager, toMerge));
            modifyPC |= g->modifyPC();
            merge |= g->doNotInstrument();
        }
        patch.setMerge(merge);
        patch.setModifyPC(modifyPC);

        Reg::Vec usedRegisters = temp_manager.getUsedRegisters();

        for(unsigned int i = 0; i < usedRegisters.size(); i++) {
            patch.prepend(SaveReg(usedRegisters[i], Offset(usedRegisters[i])).generate(cpuMode));
        }

        for(unsigned int i = 0; i < usedRegisters.size(); i++) {
            patch.append(LoadReg(usedRegisters[i], Offset(usedRegisters[i])).generate(cpuMode));
        }

        return patch;
    }
};

}

#endif //PATCHRULES_H
