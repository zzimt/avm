#ifndef AVM_STACK_H_
#define AVM_STACK_H_

#include <cstddef>
#include <vector>
#include <utility>

namespace avm {

    template <typename T>
    class Stack {
    public:
        inline Stack() :
            m_elements() { }

        inline void reserve(std::size_t capacity) {
            m_elements.reserve(capacity);
        }

        inline void push(const T& elem) {
            m_elements.push_back(elem);
        }

        inline T pop() {
            T back = std::move(m_elements.back());
            m_elements.pop_back();
            return back;
        }

        inline const T& operator[](std::size_t index) const {
            return m_elements[m_elements.size() - 1 - index];
        }

        inline T& operator[](std::size_t index) {
            return m_elements[m_elements.size() - 1 - index];
        }

        inline bool empty() const {
            return m_elements.empty();
        }

        inline std::size_t size() const {
            return m_elements.size();
        }

        auto begin() {
            return m_elements.begin();
        }

        auto end() {
            return m_elements.end();
        }

        auto begin() const {
            return m_elements.begin();
        }

        auto end() const {
            return m_elements.end();
        }

    private:
        std::vector<T> m_elements;
    };

}

#endif // AVM_STACK_H_