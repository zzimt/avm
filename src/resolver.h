#ifndef AVM_RESOLVER_H_
#define AVM_RESOLVER_H_

#include <string_view>
#include <optional>
#include <vector>
#include <unordered_map>
#include <cstdint>

#include "value.h"
#include "inst.h"

namespace avm {

    class Elem {
    public:
        enum class Kind {
            Label,
            If,
            CallIm,
            GotoIm,
            PushLabel,
            Inst,
        };

        struct Label {
            std::string_view value;

            inline Label(std::string_view value) :
                value(value) { }
        };

        struct If {
            std::string_view true_label;
            std::string_view false_label; 

            inline If(
                std::string_view true_label, 
                std::string_view false_label
            ) :
                true_label(true_label),
                false_label(false_label) { }
        };

        struct CallIm {
            std::string_view label;

            inline CallIm(std::string_view label) :
                label(label) { }
        };

        struct GotoIm {
            std::string_view label;

            inline GotoIm(std::string_view label) :
                label(label) { }
        };

        struct PushLabel {
            std::string_view label;

            inline PushLabel(std::string_view label) :
                label(label) { }
        };

        union Data {
            Label label;
            If if_;
            CallIm call_im;
            GotoIm goto_im;
            PushLabel push_label;
            Inst inst;

            Data(const Label& label) :
                label(label) { }

            Data(const If& if_) :
                if_(if_) { }

            Data(const CallIm& call_im) :
                call_im(call_im) { }

            Data(const GotoIm& goto_im) :
                goto_im(goto_im) { }

            Data(const PushLabel& push_label) :
                push_label(push_label) { }

            Data(const Inst& inst) :
                inst(inst) { }
        };

        static inline Elem label(std::string_view value) {
            return Elem(
                Kind::Label, 
                Data { 
                    Label(value)
                }
            );
        }

        inline Label label() const {
            return m_data.label;
        }

        static inline Elem if_(
            std::string_view true_label, 
            std::string_view false_label
        ) {
            return Elem(
                Kind::If, 
                Data(If(true_label, false_label))
            );
        }

        inline If if_() const {
            return m_data.if_;
        }

        static inline Elem call_im(std::string_view label) {
            return Elem(
                Kind::CallIm, 
                Data(CallIm(label))
            );
        }

        inline CallIm call_im() const {
            return m_data.call_im;
        }

        static inline Elem goto_im(std::string_view label) {
            return Elem(
                Kind::GotoIm,
                Data(GotoIm(label))
            );
        }

        inline GotoIm goto_im() const {
            return m_data.goto_im;
        }

        static inline Elem push_label(std::string_view label) {
            return Elem(
                Kind::PushLabel,
                Data (PushLabel(label))
            );
        }

        inline PushLabel push_label() {
            return m_data.push_label;
        }

        static inline Elem inst(const Inst& inst) {
            return Elem(Kind::Inst, Data(inst));
        }

        inline Inst inst() const {
            return m_data.inst;
        }

        inline Kind kind() const {
            return m_kind;
        }

        inline Elem(Kind kind, const Data& data) :
            m_kind(kind),
            m_data(data) { }

    private:
        Kind m_kind;
        Data m_data;
    };

    class Resolver {
    public:
        Resolver(const std::vector<Elem>& elems) :
            m_elems(elems),
            m_labels_to_addrs() { }
        
        std::vector<Inst> resolve() {
            collect_labels();
            return resolve_labels();
        }

        std::optional<std::uint64_t> get_label_addr(std::string_view label) {
            auto label_found = m_labels_to_addrs.find(label);
            if (label_found != m_labels_to_addrs.end()) {
                return label_found->second;
            } else {
                return std::nullopt;
            }
        }
        
    private:
        void collect_labels() {
            size_t inst_addr = 0;
            for (size_t elem_id = 0; elem_id < m_elems.size(); elem_id++) {
                auto& elem = m_elems[elem_id];
                switch (elem.kind()) {
                case Elem::Kind::Label:
                    m_labels_to_addrs.insert({elem.label().value, inst_addr});
                    break;
                case Elem::Kind::If:
                case Elem::Kind::CallIm:
                case Elem::Kind::GotoIm:
                case Elem::Kind::PushLabel:
                case Elem::Kind::Inst:
                    inst_addr++;
                    break;
                }
            }
        }

        std::vector<Inst> resolve_labels() {
            std::vector<Inst> res;
            for (size_t elem_id = 0; elem_id < m_elems.size(); elem_id++) {
                auto& elem = m_elems[elem_id];
                switch (elem.kind()) {
                case Elem::Kind::Label:
                    break;
                case Elem::Kind::If: {
                    std::uint64_t true_addr = 
                        m_labels_to_addrs[elem.if_().true_label];
                    std::uint64_t false_addr = 
                        m_labels_to_addrs[elem.if_().false_label];
                    res.push_back(
                        Inst(
                            Op::If, 
                            Value::uinteger(true_addr), 
                            Value::uinteger(false_addr)
                        )
                    );
                } break;
                case Elem::Kind::CallIm: {
                    std::uint64_t addr = 
                        m_labels_to_addrs[elem.call_im().label];
                    res.push_back(
                        Inst(
                            Op::CallIm,
                            Value::uinteger(addr)
                        )
                    );
                } break;
                case Elem::Kind::GotoIm: {
                    std::uint64_t addr = 
                        m_labels_to_addrs[elem.goto_im().label];
                    res.push_back(
                        Inst(
                            Op::GotoIm,
                            Value::uinteger(addr)
                        )
                    );
                } break;
                case Elem::Kind::PushLabel: {
                    std::uint64_t addr =
                        m_labels_to_addrs[elem.push_label().label];
                    res.push_back(
                        Inst(
                            Op::Push,
                            Value::uinteger(addr)
                        )
                    );
                } break;
                case Elem::Kind::Inst:
                    res.push_back(elem.inst());
                    break;
                }
            }
            return res;
        }

        std::vector<Elem> m_elems;
        std::unordered_map<std::string_view, std::uint64_t> m_labels_to_addrs;
    };

}

#endif // AVM_RESOLVER_H_