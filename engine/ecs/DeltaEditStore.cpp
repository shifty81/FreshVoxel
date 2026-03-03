#include "ecs/DeltaEditStore.h"
#include <sstream>
#include <cctype>
#include <stdexcept>

namespace fresh::ecs {

const char* deltaEditTypeName(DeltaEditType type) {
    switch (type) {
        case DeltaEditType::AddObject:    return "AddObject";
        case DeltaEditType::RemoveObject: return "RemoveObject";
        case DeltaEditType::MoveObject:   return "MoveObject";
        case DeltaEditType::SetProperty:  return "SetProperty";
    }
    return "Unknown";
}

void DeltaEditStore::record(const DeltaEdit& edit) {
    m_edits.push_back(edit);
}

void DeltaEditStore::clear() {
    m_edits.clear();
}

static std::string escapeJSON(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 4);
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:   out += c;      break;
        }
    }
    return out;
}

std::string DeltaEditStore::serializeToJSON() const {
    std::ostringstream os;
    os << "{\n  \"worldSeed\": " << m_seed << ",\n  \"deltaEdits\": [";

    for (size_t i = 0; i < m_edits.size(); ++i) {
        const auto& e = m_edits[i];
        if (i > 0) os << ",";
        os << "\n    {";
        os << "\"type\":\"" << deltaEditTypeName(e.type) << "\"";
        os << ",\"entityID\":" << e.entityID;
        if (!e.objectType.empty())
            os << ",\"objectType\":\"" << escapeJSON(e.objectType) << "\"";
        os << ",\"position\":[" << e.position[0] << "," << e.position[1]
           << "," << e.position[2] << "]";
        if (!e.propertyName.empty())
            os << ",\"propertyName\":\"" << escapeJSON(e.propertyName) << "\"";
        if (!e.propertyValue.empty())
            os << ",\"propertyValue\":\"" << escapeJSON(e.propertyValue) << "\"";
        os << "}";
    }

    os << "\n  ]\n}";
    return os.str();
}

static void skipWhitespace(const std::string& s, size_t& pos) {
    while (pos < s.size() && (s[pos] == ' ' || s[pos] == '\n' ||
                               s[pos] == '\r' || s[pos] == '\t'))
        ++pos;
}

static bool expect(const std::string& s, size_t& pos, char ch) {
    skipWhitespace(s, pos);
    if (pos < s.size() && s[pos] == ch) { ++pos; return true; }
    return false;
}

static std::string readString(const std::string& s, size_t& pos) {
    skipWhitespace(s, pos);
    if (pos >= s.size() || s[pos] != '"') return {};
    ++pos;
    std::string out;
    while (pos < s.size() && s[pos] != '"') {
        if (s[pos] == '\\' && pos + 1 < s.size()) {
            ++pos;
            switch (s[pos]) {
                case '"':  out += '"';  break;
                case '\\': out += '\\'; break;
                case 'n':  out += '\n'; break;
                case 'r':  out += '\r'; break;
                case 't':  out += '\t'; break;
                default:   out += s[pos]; break;
            }
        } else {
            out += s[pos];
        }
        ++pos;
    }
    if (pos < s.size()) ++pos;
    return out;
}

static double readNumber(const std::string& s, size_t& pos) {
    skipWhitespace(s, pos);
    size_t start = pos;
    if (pos < s.size() && (s[pos] == '-' || s[pos] == '+')) ++pos;
    bool hasDot = false;
    while (pos < s.size()) {
        if (std::isdigit(static_cast<unsigned char>(s[pos]))) {
            ++pos;
        } else if (s[pos] == '.' && !hasDot) {
            hasDot = true;
            ++pos;
        } else {
            break;
        }
    }
    if (pos == start) return 0.0;
    return std::stod(s.substr(start, pos - start));
}

static bool parseEditType(const std::string& name, DeltaEditType& out) {
    if (name == "AddObject")    { out = DeltaEditType::AddObject;    return true; }
    if (name == "RemoveObject") { out = DeltaEditType::RemoveObject; return true; }
    if (name == "MoveObject")   { out = DeltaEditType::MoveObject;   return true; }
    if (name == "SetProperty")  { out = DeltaEditType::SetProperty;  return true; }
    return false;
}

bool DeltaEditStore::deserializeFromJSON(const std::string& json) {
    m_edits.clear();
    m_seed = 0;

    size_t pos = 0;
    if (!expect(json, pos, '{')) return false;

    while (pos < json.size()) {
        skipWhitespace(json, pos);
        if (pos < json.size() && json[pos] == '}') { ++pos; break; }

        std::string key = readString(json, pos);
        if (key.empty()) return false;
        if (!expect(json, pos, ':')) return false;

        if (key == "worldSeed") {
            m_seed = static_cast<uint64_t>(readNumber(json, pos));
        } else if (key == "deltaEdits") {
            if (!expect(json, pos, '[')) return false;

            while (pos < json.size()) {
                skipWhitespace(json, pos);
                if (pos < json.size() && json[pos] == ']') { ++pos; break; }
                if (pos < json.size() && json[pos] == ',') ++pos;

                if (!expect(json, pos, '{')) return false;
                DeltaEdit edit{};
                edit.type = DeltaEditType::AddObject;

                while (pos < json.size()) {
                    skipWhitespace(json, pos);
                    if (pos < json.size() && json[pos] == '}') { ++pos; break; }
                    if (pos < json.size() && json[pos] == ',') ++pos;

                    std::string field = readString(json, pos);
                    if (!expect(json, pos, ':')) return false;

                    if (field == "type") {
                        std::string typeName = readString(json, pos);
                        if (!parseEditType(typeName, edit.type))
                            return false;
                    } else if (field == "entityID") {
                        edit.entityID = static_cast<uint32_t>(readNumber(json, pos));
                    } else if (field == "objectType") {
                        edit.objectType = readString(json, pos);
                    } else if (field == "position") {
                        if (!expect(json, pos, '[')) return false;
                        edit.position[0] = static_cast<float>(readNumber(json, pos));
                        expect(json, pos, ',');
                        edit.position[1] = static_cast<float>(readNumber(json, pos));
                        expect(json, pos, ',');
                        edit.position[2] = static_cast<float>(readNumber(json, pos));
                        expect(json, pos, ']');
                    } else if (field == "propertyName") {
                        edit.propertyName = readString(json, pos);
                    } else if (field == "propertyValue") {
                        edit.propertyValue = readString(json, pos);
                    }
                }

                m_edits.push_back(edit);
            }
        }

        skipWhitespace(json, pos);
        if (pos < json.size() && json[pos] == ',') ++pos;
    }

    return true;
}

} // namespace fresh::ecs
