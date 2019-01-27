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
    PatchRule(PatchCondition::SharedPtr condition, PatchGenerator::SharedPtrVec generators);

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
    bool canBeApplied(const llvm::MCInst *inst, rword address, rword instSize, LLVMCPU* llvmCPU);

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
    Patch generate(const llvm::MCInst *inst, rword address, rword instSize, CPUMode cpuMode, LLVMCPU* llvmCPU, const Patch* toMerge = nullptr);
};

}

#endif //PATCHRULES_H
