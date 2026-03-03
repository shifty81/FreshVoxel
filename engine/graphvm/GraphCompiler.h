#pragma once
#include "graphvm/GraphIR.h"
#include "graphvm/GraphVM.h"

namespace fresh::vm {

class GraphCompiler {
public:
    Bytecode compile(const graph::Graph& graph);

private:
    void emitNode(const graph::Node& node);
    Bytecode m_bc;
};

} // namespace fresh::vm
