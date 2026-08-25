#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include <array>
#include <cstddef>
#include <stdexcept>

// Fixed-capacity FIFO ring buffer. Pushing into a full buffer overwrites the
// oldest element.
template <typename T, size_t N> class RingBuffer
{
    std::array<T, N> buffer = {};
    size_t           head   = 0; // Write position
    size_t           tail   = 0; // Read position
    size_t           count  = 0; // Actual element count

public:
    // Add element to buffer (overwrite oldest when full)
    // Returns "true" if the oldest was overwritten
    bool push(const T& value) noexcept
    {
        buffer[head] = value;
        head         = (head + 1) % N;

        if (count < N)
        {
            ++count;
        } else
        {
            tail = (tail + 1) % N;
            return true;
        }
        return false;
    }

    // Try to remove oldest element
    bool pop(T& output) noexcept
    {
        if (count == 0)
            return false;
        output = buffer[tail];
        tail   = (tail + 1) % N;
        --count;
        return true;
    }

    // Try to read the oldest element without removal
    bool front(T& output) const noexcept
    {
        if (count == 0)
            return false;
        output = buffer[tail];
        return true;
    }

    // Try to read the newest element without removal
    bool back(T& output) const noexcept
    {
        if (count == 0)
            return false;
        size_t last_index = (head == 0) ? (N - 1) : (head - 1);
        output            = buffer[last_index];
        return true;
    }

    // Returns a reference to the most recent element.
    // Caller must ensure the buffer is not empty before calling.
    T& back_ref()
    {
        if (count == 0)
        {
            throw std::out_of_range("RingBuffer is empty");
        }
        size_t last_index = (head == 0) ? (N - 1) : (head - 1);
        return buffer[last_index];
    }

    // Accessors
    T& at(const size_t index) noexcept
    {
        return buffer[(tail + index) % N];
    }
    const T& at(const size_t index) const noexcept
    {
        return buffer[(tail + index) % N];
    }

    T& operator[](const size_t index) noexcept
    {
        return at(index);
    }
    const T& operator[](const size_t index) const noexcept
    {
        return at(index);
    }

    // Get number of stored elements
    size_t size() const noexcept
    {
        return count;
    }

    // Check if buffer is full
    bool full() const noexcept
    {
        return count == N;
    }

    // Check if buffer is empty
    bool empty() const noexcept
    {
        return count == 0;
    }

    // Iterate elements from oldest to newest
    template <typename F> void for_each(F&& callback) noexcept
    {
        size_t current = tail;
        for (size_t i = 0; i < count; ++i)
        {
            callback(buffer[current]);
            current = (current + 1) % N;
        }
    }

    // Method to extend data starting from a specific start_point with any iterable container
    template <typename Iterable> void extend(const Iterable& new_data, size_t start_point = 0)
    {
        if (start_point >= N)
            throw std::out_of_range("start_point is out of range");

        size_t idx = start_point;
        for (const auto& value : new_data)
        {
            buffer[idx] = value;

            // Move to the next index
            idx = (idx + 1) % N; // Ensuring circular buffer

            if (count < N)
                ++count;
            else
                tail = (tail + 1) % N; // Move the tail if the buffer is full
        }

        // Update head to reflect the new position
        head = idx % N;
    }

    std::array<T, N> data()
    {
        return buffer;
    }

    void clear() noexcept
    {
        buffer.fill(T{});
        head  = 0;
        tail  = 0;
        count = 0;
    }
};
