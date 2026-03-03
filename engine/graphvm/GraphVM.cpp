#include "graphvm/GraphVM.h"
#include "core/EventBus.h"
#include <cassert>

namespace fresh::vm {

void GraphVM::push(Value v) {
    m_stack.push_back(v);
}

Value GraphVM::pop() {
    assert(!m_stack.empty());
    Value v = m_stack.back();
    m_stack.pop_back();
    return v;
}

bool GraphVM::popBool() {
    return pop() != 0;
}

void GraphVM::execute(const Bytecode& bc, VMContext& ctx) {
    m_stack.clear();
    m_locals.clear();

    uint32_t ip = 0;

    while (ip < bc.instructions.size()) {
        const Instruction& inst = bc.instructions[ip];

        switch (inst.opcode) {
            case OpCode::NOP:
                break;

            case OpCode::LOAD_CONST:
                push(bc.constants[inst.a]);
                break;

            case OpCode::LOAD_VAR:
                push(m_locals[inst.a]);
                break;

            case OpCode::STORE_VAR:
                m_locals[inst.a] = pop();
                break;

            case OpCode::ADD: {
                auto b = pop();
                auto a = pop();
                push(a + b);
                break;
            }

            case OpCode::SUB: {
                auto b = pop();
                auto a = pop();
                push(a - b);
                break;
            }

            case OpCode::MUL: {
                auto b = pop();
                auto a = pop();
                push(a * b);
                break;
            }

            case OpCode::DIV: {
                auto b = pop();
                auto a = pop();
                push(b != 0 ? a / b : 0);
                break;
            }

            case OpCode::CMP_EQ: {
                auto b = pop();
                auto a = pop();
                push(a == b ? 1 : 0);
                break;
            }

            case OpCode::CMP_LT: {
                auto b = pop();
                auto a = pop();
                push(a < b ? 1 : 0);
                break;
            }

            case OpCode::CMP_GT: {
                auto b = pop();
                auto a = pop();
                push(a > b ? 1 : 0);
                break;
            }

            case OpCode::JUMP:
                ip = inst.a;
                continue;

            case OpCode::JUMP_IF_FALSE:
                if (!popBool()) {
                    ip = inst.a;
                    continue;
                }
                break;

            case OpCode::EMIT_EVENT: {
                Value param = m_stack.empty() ? 0 : pop();
                if (ctx.eventBus && inst.a < bc.eventNames.size()) {
                    fresh::Event evt;
                    evt.type = bc.eventNames[inst.a];
                    evt.senderId = ctx.entity;
                    evt.intParam = param;
                    ctx.eventBus->publish(evt);
                }
                break;
            }

            case OpCode::END:
                return;
        }

        ++ip;
    }
}

Value GraphVM::getLocal(uint32_t idx) const {
    auto it = m_locals.find(idx);
    return it != m_locals.end() ? it->second : 0;
}

} // namespace fresh::vm
