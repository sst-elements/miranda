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

#ifndef _H_SST_MEM_H_REQ_GEN_CPU
#define _H_SST_MEM_H_REQ_GEN_CPU

#include <sst/core/component.h>
#include <sst/core/interfaces/simpleMem.h>
#include <sst/core/statapi/stataccumulator.h>

#include "mirandaEvent.h"
#include "mirandaGenerator.h"
#include "mirandaMemMgr.h"

using namespace SST;
using namespace SST::Interfaces;
using namespace SST::Statistics;

namespace SST {
namespace Miranda {

class CPURequest {
   public:
    explicit CPURequest(const uint64_t origID)
        : originalID(origID), issueTime(0), outstandingParts(0) {}

    void incPartCount() { outstandingParts++; }

    void decPartCount() { outstandingParts--; }

    auto completed() const -> bool { return 0 == outstandingParts; }

    void setIssueTime(const uint64_t now) { issueTime = now; }

    auto getIssueTime() const -> uint64_t { return issueTime; }

    auto getOriginalReqID() const -> uint64_t { return originalID; }

    auto countParts() const -> uint32_t { return outstandingParts; }

   protected:
    uint64_t originalID;
    uint64_t issueTime;
    uint32_t outstandingParts;
};

class RequestGenCPU : public SST::Component {
   public:
    RequestGenCPU(SST::ComponentId_t id, SST::Params &params);

    void finish() override;

    void init(unsigned int phase) override;

    SST_ELI_REGISTER_COMPONENT(
        RequestGenCPU, "miranda", "BaseCPU", SST_ELI_ELEMENT_VERSION(1, 0, 0),
        "Creates a base Miranda CPU ready to execute an address generator/access pattern",
        COMPONENT_CATEGORY_PROCESSOR)

    SST_ELI_DOCUMENT_PARAMS(
        {"max_reqs_cycle",
         "Maximum number of requests the CPU can issue per cycle (this is for all reads and "
         "writes)",
         "2"},
        {"max_reorder_lookups",
         "Maximum number of operations the CPU is allowed to lookup for memory reorder", "16"},
        {"cache_line_size",
         "The size of the cache line that this prefetcher is attached to, default is 64-bytes",
         "64"},
        {"maxmemreqpending", "Set the maximum number of requests allowed to be pending", "16"},
        {"verbose", "Sets the verbosity of output produced by the CPU", "0"},
        {"generator", "The generator to be loaded for address creation",
         "miranda.SingleStreamGenerator"},
        {"clock", "Clock for the base CPU", "2GHz"},
        {"memoryinterface", "Sets the memory interface module to use", "memHierarchy.memInterface"},
        {"pagecount", "Sets the number of pages the system can allocate", "4194304"},
        {"pagesize",
         "Sets the size of the page in the system, MUST be a multiple of cache_line_size", "4096"},
        {"pagemap",
         "Mapping scheme, string set to LINEAR or RANDOMIZED, default is LINEAR "
         "(virtual==physical), RANDOMIZED randomly shuffles virtual to physical map.",
         "LINEAR"})

    SST_ELI_DOCUMENT_STATISTICS(
        {"split_read_reqs", "Number of read requests split over a cache line boundary", "requests",
         2},
        {"split_write_reqs", "Number of write requests split over a cache line boundary",
         "requests", 2},
        {"split_custom_reqs", "NUmber of custom requests split over a cache line boundary",
         "requests", 2},
        {"read_reqs", "Number of read requests issued", "requests", 1},
        {"write_reqs", "Number of write requests issued", "requests", 1},
        {"custom_reqs", "Number of custom requests issued", "requests", 1},
        {"total_bytes_read", "Count the total bytes requested by read operations", "bytes", 1},
        {"total_bytes_write", "Count the total bytes requested by write operations", "bytes", 1},
        {"total_bytes_custom", "Count the total bytes requested by custom operations", "bytes", 1},
        {"req_latency", "Running total of all latency for all requests", "ns", 2},
        {"cycles_with_issue", "Number of cycles which CPU was able to issue requests", "cycles", 1},
        {"cycles_no_issue", "Number of cycles which CPU was not able to issue requests", "cycles",
         1},
        {"time", "Nanoseconds spent issuing requests", "ns", 1},
        {"cycles_hit_fence", "Number of issue cycles which stop issue at a fence", "cycles", 2},
        {"cycles_max_reorder", "Number of issue cycles which hit maximum reorder lookup", "cycles",
         2},
        {"cycles_max_issue", "Cycles with maximum operation issue", "cycles", 2},
        {"cycles", "Cycles executed", "cycles", 1})

    SST_ELI_DOCUMENT_PORTS({"cache_link",
                            "Link to Memory Controller",
                            {"memHierarchy.memEvent", ""}},
                           {"src", "Link to generator source", {"memHierarchy.memEvent", ""}})

    SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS({"generator", "What address generator to load",
                                         "SST::Miranda::RequestGenerator"},
                                        {"memory",
                                         "The memory interface to use (e.g., interface to caches)",
                                         "SST::Interfaces::SimpleMem"})

   private:
    RequestGenCPU() = delete;                        // for serialization only
    RequestGenCPU(const RequestGenCPU &) = delete;   // do not implement
    void operator=(const RequestGenCPU &) = delete;  // do not implement
    ~RequestGenCPU() override;

    void loadGenerator(MirandaReqEvent * /*event*/);

    void loadGenerator(const std::string &name, SST::Params &params);

    void handleEvent(SimpleMem::Request *ev);

    auto clockTick(SST::Cycle_t /*unused*/) -> bool;

    void issueRequest(MemoryOpRequest *req);

    void handleSrcEvent(SST::Event * /*ev*/);

    Output *out;

    TimeConverter *timeConverter;
    Clock::HandlerBase *clockHandler;
    RequestGenerator *reqGen;
    std::map<SimpleMem::Request::id_t, CPURequest *> requestsInFlight;
    SimpleMem *cache_link;
    Link *srcLink;
    MirandaReqEvent *srcReqEvent{};

    MirandaRequestQueue<GeneratorRequest *> pendingRequests;
    MirandaMemoryManager *memMgr;

    uint32_t maxRequestsPending[OPCOUNT]{};
    uint32_t requestsPending[OPCOUNT]{};
    uint32_t reqMaxPerCycle;
    uint64_t cacheLine;
    uint32_t maxOpLookup;

    Statistic<uint64_t> *statReqs[OPCOUNT]{};
    Statistic<uint64_t> *statSplitReqs[OPCOUNT]{};
    Statistic<uint64_t> *statCyclesWithIssue;
    Statistic<uint64_t> *statMaxIssuePerCycle;
    Statistic<uint64_t> *statCyclesWithoutIssue;
    Statistic<uint64_t> *statBytes[OPCOUNT]{};
    Statistic<uint64_t> *statReqLatency;
    Statistic<uint64_t> *statTime;
    Statistic<uint64_t> *statCyclesHitFence;
    Statistic<uint64_t> *statCyclesHitReorderLimit;
    Statistic<uint64_t> *statCycles;
};

}  // namespace Miranda
}  // namespace SST
#endif /* _RequestGenCPU_H */
