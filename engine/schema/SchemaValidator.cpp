#include "SchemaValidator.h"
#include <unordered_set>

namespace fresh {

bool SchemaValidator::validate(const SchemaDefinition& schema) {
    clear();

    if (schema.id.empty()) {
        addError("Schema ID must not be empty");
    }

    if (schema.version <= 0) {
        addError("Schema version must be > 0");
    }

    std::unordered_set<std::string> nodeIds;
    for (const auto& node : schema.nodes) {
        if (node.id.empty()) {
            addError("Node ID must not be empty");
            continue;
        }
        if (!nodeIds.insert(node.id).second) {
            addError("Duplicate node ID: " + node.id);
        }
        for (const auto& field : node.inputs) {
            if (field.name.empty()) {
                addError("Field name must not be empty in node: " + node.id);
            }
        }
        for (const auto& field : node.outputs) {
            if (field.name.empty()) {
                addError("Field name must not be empty in node: " + node.id);
            }
        }
    }

    return m_errors.empty();
}

const std::vector<SchemaError>& SchemaValidator::getErrors() const {
    return m_errors;
}

void SchemaValidator::clear() {
    m_errors.clear();
}

void SchemaValidator::addError(const std::string& msg) {
    m_errors.push_back({msg});
}

} // namespace fresh
