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

#ifndef CIRCULAR_ARRAY_HPP_
#define CIRCULAR_ARRAY_HPP_

#include <algorithm>
#include <cassert>
#include <cinttypes>
#include <cstring> // memcpy
#include <memory>
#include <iostream>
#include <stdexcept>

/**
 * A simple queue implemented as a circular array. The data structure resizes the underlying storage when it becomes full, but like a Vector,
 * it never decreases its capacity once increased.
 * The class is not thread safe.
 *
 * The operations that can be performed are: Q.append(element), Q.prepend(element), Q.pop(), Q.size(), Q.empty() and Q[i] (accessor).
 *
 * The data structure is not thread safe.
 */
template <typename T>
class CircularArray {
    T* m_array; //  the actual container of the elements
    uint64_t m_start; // start index (incl)
    uint64_t m_end; // last index (excl)
    uint64_t m_capacity; // current capacity of the array m_array
    bool m_empty; // whether the data structure is empty

protected:
    // is the array full?
    bool full() const {
        return !empty() && m_start == m_end;
    }

    // resize the capacity of the underlying array, and copy the elements
    void resize(uint64_t capacity){
        assert(capacity >= size() && "Cannot resize this array, it already contains more elements than the new capacity");
        std::unique_ptr<T[]> new_array{ new T[capacity] };

        // copy the elements from the old array to the new one
        if(!empty()){
            if(m_end > m_start){
                memcpy(new_array.get(), m_array + m_start, sizeof(T) * (m_end - m_start));
            } else { // m_end <= m_start
                memcpy(new_array.get(), m_array + m_start, sizeof(T) * (m_capacity - m_start));
                memcpy(new_array.get() + (m_capacity - m_start), m_array, sizeof(T) * m_end);
            }
        }

        // swap new_array with m_array
        delete[] m_array; m_array = new_array.get();
        new_array.release(); // do not delete the array just allocated

        m_end = size(); // do not swap the order with m_start
        m_start = 0;
        m_capacity = capacity;
    }

    // convert the given absolute index to its actual position in the array
    uint64_t to_array_index(int64_t index) const {
        if( index >= size() || index < 0 ) throw std::runtime_error("Index out of bounds");
        if( m_end > m_start ){
            return m_start + index;
        } else { // m_end <= m_start
            if(index < m_capacity - m_start) {
                return m_start + index;
            } else {
                return index - (m_capacity - m_start);
            }
        }
    }

public:
    /**
     * Initialise the container with the given initial capacity
     */
    CircularArray(uint64_t capacity = 64) : m_array(nullptr), m_start(0), m_end(0), m_capacity(capacity), m_empty(true) {
        m_array = new T[capacity];
    }

    /**
     * Destructor
     */
    ~CircularArray(){
        delete[] m_array; m_array = nullptr;
    }

    /**
     * Is the container empty ?
     */
    bool empty() const {
        assert((!m_empty || m_start == m_end) && "Precondition not respected");
        return m_empty;
    }

    /**
     * Retrieve the number of elements contained
     */
    size_t size() const {
        if(empty())
            return 0;
        else if (m_end > m_start)
            return m_end - m_start;
        else
            return m_end + (m_capacity - m_start);
    }

    /**
     * Append a new element at the end
     */
    void append(const T& item){
        if(full()){ resize(m_capacity << 1 /* x2 */); }
        m_array[m_end] = item;
        m_end++;
        if(m_end == m_capacity) m_end = 0;
        m_empty = false;
    }

    /**
     * Prepend a new element at the start
     */
    void prepend(const T& item){
        if(full()){ resize(m_capacity << 1 /* x2 */); }
        if(m_start == 0)
            m_start = m_capacity -1;
        else
            m_start--;
        m_array[m_start] = item;
        m_empty = false;
    }

    /**
     * Remove the element at the start
     */
    void pop(){
        if(empty()) throw std::runtime_error("The queue is empty");
        m_start++;
        if(m_start >= m_capacity) m_start = 0;
        m_empty = (m_start == m_end);
    }

    /**
     * Retrieve an element at given position
     */
    T& operator[](int64_t i){
        return m_array[to_array_index(i)];
    }

    /**
     * Retrieve an element at given position
     */
    const T& operator[](int64_t i) const{
        return m_array[to_array_index(i)];
    }

    /**
     * Remove all elements in the queue.
     * @param capacity if != 0, it sets the capacity of the underlying array
     */
    void clear(uint64_t capacity = 0) {
        m_start = m_end = 0;
        m_empty = true;
        if(capacity > 0 && capacity != m_capacity){
            T* new_array = new T[capacity];
            delete[] m_array;
            m_array = new_array;
            m_capacity = capacity;
        }
    }

    /**
     * Dump the content of the array to stdout
     */
    void dump() const  {
        using namespace std;
        cout << "[CircularArray size: " << size() << ", start: " << m_start << ", end: " << m_end << ", capacity: " << m_capacity << "\n";
        for(size_t i =0, sz = size(); i < sz; i++){
            cout << "[" << i << "] " << m_array[to_array_index(i)] << "\n";
        }
        cout << endl;
    }
};

#endif /* CIRCULAR_ARRAY_HPP_ */
