#ifndef AVM_STRINGSTORE_H_
#define AVM_STRINGSTORE_H_

#include <vector>
#include <string_view>
#include <cstdint>

#include "mem.h"

namespace avm {

    class StringStore {
    public:
        StringStore() :
            m_buffer(),
            m_strings() { 
            m_buffer.reserve(1024);
            m_strings.reserve(256);
        }

        inline void add(std::uint64_t slot, std::string_view string) {
            char* ptr = m_buffer.data() + m_buffer.size();
            m_buffer.insert(
                m_buffer.end(), 
                string.begin(), 
                string.end()
            );
            StrKey key(ptr, string.size());
            if (slot >= m_strings.size()) {
                m_strings.resize(slot + 1);
            }
            m_strings[slot] = key;
        }

        inline const StrKey& operator[](std::uint64_t slot) const {
            return m_strings[slot];
        }
        
    private:
        std::vector<char> m_buffer;
        std::vector<StrKey> m_strings;
    };

}

#endif // AVM_STRINGSTORE_H_