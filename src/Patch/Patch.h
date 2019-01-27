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
#ifndef PATCH_H
#define PATCH_H

#include <vector>

#include "Engine/Engine.h"
#include "Patch/Types.h"
#include "Patch/RelocatableInst.h"

namespace QBDI {

class Patch {
  public:

    InstMetadata metadata;
    RelocatableInst::SharedPtrVec insts;

    using Vec = std::vector<Patch>;

    Patch();
    Patch(llvm::MCInst inst, rword address, rword instSize, CPUMode cpuMode);

    void setMerge(bool merge);
    void setModifyPC(bool modifyPC);
    void setInst(llvm::MCInst inst, rword address, rword instSize);
    void append(const RelocatableInst::SharedPtrVec v);
    void prepend(const RelocatableInst::SharedPtrVec v);
    void append(const RelocatableInst::SharedPtr r);
    void prepend(const RelocatableInst::SharedPtr r);
};

}

#endif // PATCH_H
