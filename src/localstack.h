#ifndef AVM_LOCALSTACK_H_
#define AVM_LOCALSTACK_H_

#include <cstddef>
#include <vector>

#include "value.h"

namespace avm {

    class LocalStack {
    public:
        inline LocalStack() :
            m_values(),
            m_frame_indices() { }

        inline void reserve(std::size_t capacity) {
            m_values.reserve(capacity);
        }

        inline void push() {
            m_frame_indices.push_back(m_values.size());
        }

        inline void pop() {
            m_frame_indices.pop_back();
        }

        inline Value& operator[](std::size_t id) {
            std::size_t last_index = m_frame_indices.back();
            m_values.resize(last_index + id + 1);
            return m_values[last_index + id];
        }

        auto begin() {
            return m_values.begin();
        }

        auto end() {
            return m_values.end();
        }

        auto begin() const {
            return m_values.begin();
        }

        auto end() const {
            return m_values.end();
        }

    private:
        std::vector<Value> m_values;
        std::vector<std::size_t> m_frame_indices;
    };

}

#endif // AVM_LOCALSTACK_H_