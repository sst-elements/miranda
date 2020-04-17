// Copyright 2009-2020 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2020, NTESS
// All rights reserved.
//
// Portions are copyright of other developers:
// See the file CONTRIBUTORS.TXT in the top level directory
// the distribution for more information.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#ifndef _H_SST_MIRANDA_EMPTY_GEN
#define _H_SST_MIRANDA_EMPTY_GEN

#include "../mirandaGenerator.h"
#include <sst/core/output.h>

#include <queue>

using namespace SST::RNG;

namespace SST {
namespace Miranda {

class EmptyGenerator : public RequestGenerator {

  public:
    EmptyGenerator(ComponentId_t id, Params &params) : RequestGenerator(id, params) {}

    ~EmptyGenerator() override = default;

    void generate(MirandaRequestQueue<GeneratorRequest *> *) override {}

    bool isFinished() override { return true; }

    void completed() override {}

    SST_ELI_REGISTER_SUBCOMPONENT_DERIVED(EmptyGenerator, "miranda", "EmptyGenerator", SST_ELI_ELEMENT_VERSION(1, 0, 0),
                                          "Creates an empty (null) generator", SST::Miranda::RequestGenerator)

    SST_ELI_DOCUMENT_PARAMS()
};

} // namespace Miranda
} // namespace SST

#endif
