/**
 * Copyright (C) 2018 Dean De Leo, email: dleo[at]cwi.nl
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef PMA_ADAPTIVE_INT3_SPREAD_WITH_REWIRING_HPP_
#define PMA_ADAPTIVE_INT3_SPREAD_WITH_REWIRING_HPP_

#include <cinttypes>
#include <cstddef>
#include <deque>

#include "partition.hpp"

namespace pma { namespace adaptive { namespace int3 {

class PackedMemoryArray; // forward decl.

class SpreadWithRewiring {
protected:
// user parameters:
    PackedMemoryArray& m_instance; // underlying instance
    const size_t m_window_start; // the first segment
    const size_t m_window_length; // the number of consecutive segments in the window being spread
    const size_t m_segments_per_extent; // total number of segments per extent
    const VectorOfPartitions& m_partitions; // the length required for each segment

// internal state
    int64_t m_position; // current position in the window being rebalanced
    struct Extent2Rewire{ int64_t m_extent_id; int64_t* m_buffer_keys; int64_t* m_buffer_values; };
    std::deque<Extent2Rewire> m_extents_to_rewire; // a list of extents to be rewired
    size_t m_partition_id = 0; // current partition
    size_t m_partition_offset = 0; // current offset in the partition


// whether to insert a new element during the rebalancing
    bool m_insert = false;
    int64_t m_insert_key = -1;
    int64_t m_insert_value = -1;
    int64_t m_insert_to_segment = -1;
    int64_t m_insert_predecessor = -1;
    int64_t m_insert_successor = -1;

    /**
     * Get the capacity of a single segment
     */
    size_t get_segment_capacity() const;

    /**
     * Reset the current position at the start of the window
     */
    void reset_current_position();


    /**
     * Reset the current partition to zero
     */
    void reset_current_partition();


    /**
     * Get the cardinality of the current partition
     */
    size_t get_partition_current() const;


    /**
     * Get the cardinality of the next partition
     */
    size_t get_partition_next() const;

    /**
     * Move the current partition by N
     */
    void move_current_partition_forwards_by(size_t N); // move ahead
    void move_current_partition_backwards_by(size_t N); // move back
    void move_current_partition_by(int64_t N); // move ahead if N > 0, else move back

    /**
     * Get the relative extent ID for the given position. The ID 0 represents the first extent for the current window.
     */
    int64_t position2extent(int64_t position) const;

    /**
     * Get the relative segment ID for the given position. The ID 0 represents the first segment in the current window.
     */
    int64_t position2segment(int64_t position) const;


    /**
     * Get the absolute segment ID for the given relative extent.
     */
    int64_t extent2segment(int64_t extent) const;

    /**
     * Get the extent ID for the current position
     */
    int64_t get_current_extent() const;


    /**
     * Retrieve the starting offset, in multiple of sizeof(uint64_t), for the given extent, relative to window being rebalanced
     */
    size_t get_offset(int64_t relative_extent_id) const;

    /**
     * Get the address of the array, starting from the given extent
     * @param array either m_storage.m_keys or m_storage.m_values
     * @param relative_extent the extent id, relative to the window being rebalanced
     */
    int64_t* get_start_address(int64_t* array, int64_t relative_extent_id) const;

    /**
     * Retrieve free space from the rewiring facility
     */
    void acquire_free_space(int64_t** space_keys, int64_t** space_values);

    /**
     * Rewire these two virtual memory addresses and reclaim the space for the address associated to a buffer
     */
    void rewire_keys(int64_t* addr1, int64_t* addr2);
    void rewire_values(int64_t* addr1, int64_t* addr2);

    /**
     * Rewire the used buffers with their associated extents in the PMA
     */
    void reclaim_past_extents();

    /**
     * Merge & insert manually
     */
    void merge_to_insert(int64_t* __restrict output_keys, int64_t* __restrict output_values, int64_t* __restrict input_keys, int64_t* __restrict input_values,
            size_t elements_to_copy, size_t output_segment_id, int64_t output_lhs_start, int64_t output_rhs_start, int64_t output_rhs_end);

    /**
     * Finalize an insertion. Update the size of the segment where the element has been inserted and record the insertion in the detector
     */
    void finalize_insert();


    /**
     * Spread the elements into the given destinations
     */
//    void spread_elements_left2right(int64_t* __restrict destination_keys, int64_t* __restrict destination_values, size_t extent_id); // move forwards
//    void spread_elements_right2left(int64_t* __restrict destination_keys, int64_t* __restrict destination_values, size_t extent_id); // move backwards
    void spread_elements(int64_t* __restrict destination_keys, int64_t* __restrict destination_values, size_t extent_id);

    /**
     * Spread the elements for the given extent
     */
    void spread_extent(int64_t extent_id);

    /**
     * Spread the elements for the related window
     */
    void spread_window();

    /**
     * Alter the stored sizes
     */
    void update_segment_sizes();


public:

    SpreadWithRewiring(PackedMemoryArray* instance, size_t window_start, size_t window_length, const VectorOfPartitions& partitions);

    /**
     * Register a new element to insert
     */
    void set_element_to_insert(int64_t key, int64_t value);

    /**
     * Execute
     */
    void execute();

    /**
     * Set the starting position for the input
     * @param position absolute offset for the input arrays
     */
    void set_absolute_position(size_t position);

    /**
     * Helper, get the cardinality for the partition at the given position/offset
     */
    static size_t get_partition_current(const VectorOfPartitions& partitions, size_t partition_id, size_t partition_offset);

    /**
     * Helper, get the cardinality for the partition immediately after the given position/offset
     */
    static size_t get_partition_next(const VectorOfPartitions& partitions, size_t partition_id, size_t partition_offset);
};


}}} // pma::adaptive::int3

#endif /* PMA_ADAPTIVE_INT3_SPREAD_WITH_REWIRING_HPP_ */
