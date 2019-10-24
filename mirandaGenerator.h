// Copyright 2009-2019 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2019, NTESS
// All rights reserved.
//
// Portions are copyright of other developers:
// See the file CONTRIBUTORS.TXT in the top level directory
// the distribution for more information.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#ifndef _H_SST_MEM_H_REQUEST_GEN
#define _H_SST_MEM_H_REQUEST_GEN

#include <sst/core/component.h>
#include <sst/core/output.h>
#include <sst/core/subcomponent.h>

#include <cstdint>  // deprecated lib
#include <queue>

namespace SST {
namespace Miranda {

typedef enum { READ, WRITE, REQ_FENCE, CUSTOM, OPCOUNT } ReqOperation;

static std::atomic<uint64_t> nextGeneratorRequestID(0);

class GeneratorRequest {
   public:
    GeneratorRequest() { reqID = nextGeneratorRequestID++; }

    virtual ~GeneratorRequest() = default;

    virtual auto getOperation() const -> ReqOperation = 0;

    auto getRequestID() const -> uint64_t { return reqID; }

    void addDependency(uint64_t depReq) { dependsOn.push_back(depReq); }

    void satisfyDependency(const GeneratorRequest *req) { satisfyDependency(req->getRequestID()); }

    void satisfyDependency(const uint64_t req) {
        std::vector<uint64_t>::iterator searchDeps;

        for (searchDeps = dependsOn.begin(); searchDeps != dependsOn.end(); searchDeps++) {
            if (req == (*searchDeps)) {
                dependsOn.erase(searchDeps);
                break;
            }
        }
    }

    auto canIssue() -> bool { return dependsOn.empty(); }

    auto getIssueTime() const -> uint64_t { return issueTime; }

    void setIssueTime(const uint64_t now) { issueTime = now; }

   protected:
    uint64_t reqID;
    uint64_t issueTime{};
    std::vector<uint64_t> dependsOn;
};

template <typename QueueType>
class MirandaRequestQueue {
   public:
    MirandaRequestQueue() {
        theQ = (QueueType *)malloc(sizeof(QueueType) * 16);
        maxCapacity = 16;
        curSize = 0;
    }

    ~MirandaRequestQueue() { free(theQ); }

    auto empty() const -> bool { return 0 == curSize; }

    void resize(const uint32_t newSize) {
        //		printf("Resizing MirandaQueue from: %" PRIu32 " to %" PRIu32 "\n",
        //			curSize, newSize);

        auto newQ = (QueueType *)malloc(sizeof(QueueType) * newSize);
        for (uint32_t i = 0; i < curSize; ++i) {
            newQ[i] = theQ[i];
        }

        free(theQ);
        theQ = newQ;
        maxCapacity = newSize;
        curSize = std::min(curSize, newSize);
    }

    auto size() const -> uint32_t { return curSize; }

    auto capacity() const -> uint32_t { return maxCapacity; }

    auto at(const uint32_t index) -> QueueType { return theQ[index]; }

    void erase(const std::vector<uint32_t> eraseList) {
        if (eraseList.empty()) {
            return;
        }

        auto newQ = (QueueType *)malloc(sizeof(QueueType) * maxCapacity);

        uint32_t nextSkipIndex = 0;
        uint32_t nextSkip = eraseList.at(nextSkipIndex);
        uint32_t nextNewQIndex = 0;

        for (uint32_t i = 0; i < curSize; ++i) {
            if (nextSkip == i) {
                nextSkipIndex++;

                if (nextSkipIndex >= eraseList.size()) {
                    nextSkip = curSize;
                } else {
                    nextSkip = eraseList.at(nextSkipIndex);
                }
            } else {
                newQ[nextNewQIndex] = theQ[i];
                nextNewQIndex++;
            }
        }

        free(theQ);

        theQ = newQ;
        curSize = nextNewQIndex;
    }

    void push_back(QueueType t) {
        if (curSize == maxCapacity) {
            resize(maxCapacity + 16);
        }

        theQ[curSize] = t;
        curSize++;
    }

   private:
    QueueType *theQ;
    uint32_t maxCapacity;
    uint32_t curSize;
};

class MemoryOpRequest : public GeneratorRequest {
   public:
    MemoryOpRequest(const uint64_t cAddr, const uint64_t cLength, const ReqOperation cOpType)
        :

          addr(cAddr),
          length(cLength),
          op(cOpType) {}

    ~MemoryOpRequest() override = default;

    auto getOperation() const -> ReqOperation override { return op; }

    auto isRead() const -> bool { return op == READ; }

    auto isWrite() const -> bool { return op == WRITE; }

    auto isCustom() const -> bool { return op == CUSTOM; }

    auto getAddress() const -> uint64_t { return addr; }

    auto getLength() const -> uint64_t { return length; }

   protected:
    uint64_t addr;
    uint64_t length;
    ReqOperation op;
};

class CustomOpRequest : public MemoryOpRequest {
   public:
    CustomOpRequest(const uint64_t cAddr, const uint64_t cLength, const uint32_t cOpcode)
        : MemoryOpRequest(cAddr, cLength, CUSTOM) {
        opcode = cOpcode;
    }

    ~CustomOpRequest() override = default;

    auto getOpcode() const -> uint32_t { return opcode; }

   protected:
    uint32_t opcode;
};

class FenceOpRequest : public GeneratorRequest {
   public:
    FenceOpRequest() = default;

    ~FenceOpRequest() override = default;

    auto getOperation() const -> ReqOperation override { return REQ_FENCE; }
};

class RequestGenerator : public SubComponent {
   public:
    SST_ELI_REGISTER_SUBCOMPONENT_API(SST::Miranda::RequestGenerator)

    RequestGenerator(Component *owner, Params & /*unused*/) : SubComponent(owner) {}

    RequestGenerator(ComponentId_t id, Params & /*unused*/) : SubComponent(id) {}

    ~RequestGenerator() override = default;

    virtual void generate(MirandaRequestQueue<GeneratorRequest *> *q) {}

    virtual auto isFinished() -> bool { return true; }

    virtual void completed() {}
};

}  // namespace Miranda
}  // namespace SST

#endif
