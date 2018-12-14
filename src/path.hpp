#ifndef dank_path_hpp
#define dank_path_hpp

#include "dynamic.hpp"
#include "handle_types.hpp"
#include "handle_helper.hpp"

namespace dankgraph {

struct step_t {
    id_t id = 0;
    bool strand = 0;
    std::string seq;
};

class path_t {
public:
    /// the path name
    std::string name;
    /// constructor
    path_t(const std::string& n) { name = n; }
    /// append a step across the given id with the orientation given by strand
    void append_occurrence(const handle_t& handle);
    /// remove all elements
    void clear(void);
    /// the number of steps in the path
    uint64_t occurrence_count(void) const;
    /// construct a step object that describes the given step, which may include non-graph sequence
    step_t get_occurrence(uint64_t rank) const;
    /// unlink the occurrence from the graph handle, storing the sequence in the path itself
    void unlink_occurrence(uint64_t rank, const std::string& seq);
private:
    /// store the ids in the path
    /// zeros indicate privately stored sequences in unlinked occurrences
    dyn::wt_string<dyn::suc_bv> ids_wt;
    /// the strand of each step
    dyn::suc_bv strands_wt;
    /// sequence that is in this path, but not represented in the graph
    /// for instance, after the removal of nodes from the graph
    dyn::wt_string<dyn::suc_bv> seq_wt;
};

inline void path_t::clear(void) {
    dyn::wt_string<dyn::suc_bv> null_wt;
    dyn::suc_bv null_bv;
    ids_wt = null_wt;
    strands_wt = null_bv;
    seq_wt = null_wt;
}

inline void path_t::append_occurrence(const handle_t& handle) {
    ids_wt.push_back(handle_helper::unpack_number(handle));
    strands_wt.push_back(handle_helper::unpack_bit(handle));
}

inline uint64_t path_t::occurrence_count(void) const {
    return ids_wt.size();
}

inline step_t path_t::get_occurrence(uint64_t rank) const {
    step_t result;
    result.id = ids_wt.at(rank);
    result.strand = strands_wt.at(rank);
    if (!result.id) {
        for (uint64_t i = seq_wt.select(ids_wt.rank(rank, 0), 0);
             ; ++i) {
            char c = seq_wt.at(i);
            if (!c) break;
            result.seq += c;
        }
    }
    return result;
}

inline void path_t::unlink_occurrence(uint64_t rank, const std::string& seq) {
    // set the path step id to 0
    ids_wt.remove(rank);
    ids_wt.insert(rank, 0);
    // append the sequence to seq_wt
    if (seq_wt.size() == 0) seq_wt.push_back(0); // start and end with 0s
    for (auto c : seq) seq_wt.push_back(c);
    seq_wt.push_back(0);
    // CAUTION we've appended the seq in its natural orientation in the graph
    // and the orientation is maintained in the strand_bv
    // we have to refer to this when e.g. serializing the path
}

}

#endif
