#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <typeindex>
#include <any>

namespace fresh {

// Forward declarations
class TypeInfo;
class PropertyInfo;
class Object;

/**
 * @brief Type of a property (for serialization and editor display)
 */
enum class PropertyType {
    Bool,
    Int,
    Float,
    Double,
    String,
    Vec2,
    Vec3,
    Vec4,
    Color,
    Object,
    Unknown
};

/**
 * @brief Property flags for metadata
 */
enum class PropertyFlags : uint32_t {
    None = 0,
    ReadOnly = 1 << 0,
    Serializable = 1 << 1,
    EditorVisible = 1 << 2,
    Advanced = 1 << 3
};

inline PropertyFlags operator|(PropertyFlags a, PropertyFlags b) {
    return static_cast<PropertyFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline bool operator&(PropertyFlags a, PropertyFlags b) {
    return (static_cast<uint32_t>(a) & static_cast<uint32_t>(b)) != 0;
}

/**
 * @brief Information about a property on a type
 */
class PropertyInfo {
public:
    PropertyInfo(const std::string& name, PropertyType type, PropertyFlags flags = PropertyFlags::Serializable | PropertyFlags::EditorVisible)
        : m_name(name)
        , m_type(type)
        , m_flags(flags)
    {}

    const std::string& getName() const { return m_name; }
    PropertyType getType() const { return m_type; }
    PropertyFlags getFlags() const { return m_flags; }

    bool isReadOnly() const { return m_flags & PropertyFlags::ReadOnly; }
    bool isSerializable() const { return m_flags & PropertyFlags::Serializable; }
    bool isEditorVisible() const { return m_flags & PropertyFlags::EditorVisible; }

    // Getter and setter functions
    using Getter = std::function<std::any(void*)>;
    using Setter = std::function<void(void*, const std::any&)>;

    void setGetter(Getter getter) { m_getter = getter; }
    void setSetter(Setter setter) { m_setter = setter; }

    std::any getValue(void* instance) const {
        if (m_getter) {
            return m_getter(instance);
        }
        return std::any();
    }

    void setValue(void* instance, const std::any& value) const {
        if (m_setter && !isReadOnly()) {
            m_setter(instance, value);
        }
    }

private:
    std::string m_name;
    PropertyType m_type;
    PropertyFlags m_flags;
    Getter m_getter;
    Setter m_setter;
};

/**
 * @brief Type information for reflected types
 */
class TypeInfo {
public:
    TypeInfo(const std::string& name, std::type_index typeIndex)
        : m_name(name)
        , m_typeIndex(typeIndex)
    {}

    const std::string& getName() const { return m_name; }
    std::type_index getTypeIndex() const { return m_typeIndex; }

    // Property management
    void addProperty(std::shared_ptr<PropertyInfo> property) {
        m_properties[property->getName()] = property;
    }

    const std::unordered_map<std::string, std::shared_ptr<PropertyInfo>>& getProperties() const {
        return m_properties;
    }

    std::shared_ptr<PropertyInfo> getProperty(const std::string& name) const {
        auto it = m_properties.find(name);
        return it != m_properties.end() ? it->second : nullptr;
    }

    // Factory function for creating instances
    using Factory = std::function<void*()>;
    void setFactory(Factory factory) { m_factory = factory; }
    void* createInstance() const {
        return m_factory ? m_factory() : nullptr;
    }

private:
    std::string m_name;
    std::type_index m_typeIndex;
    std::unordered_map<std::string, std::shared_ptr<PropertyInfo>> m_properties;
    Factory m_factory;
};

/**
 * @brief Central reflection registry
 */
class ReflectionRegistry {
public:
    static ReflectionRegistry& getInstance() {
        static ReflectionRegistry instance;
        return instance;
    }

    void registerType(std::shared_ptr<TypeInfo> typeInfo) {
        m_types[typeInfo->getTypeIndex()] = typeInfo;
        m_typesByName[typeInfo->getName()] = typeInfo;
    }

    std::shared_ptr<TypeInfo> getType(std::type_index typeIndex) const {
        auto it = m_types.find(typeIndex);
        return it != m_types.end() ? it->second : nullptr;
    }

    std::shared_ptr<TypeInfo> getType(const std::string& name) const {
        auto it = m_typesByName.find(name);
        return it != m_typesByName.end() ? it->second : nullptr;
    }

    const std::unordered_map<std::type_index, std::shared_ptr<TypeInfo>>& getAllTypes() const {
        return m_types;
    }

private:
    ReflectionRegistry() = default;
    std::unordered_map<std::type_index, std::shared_ptr<TypeInfo>> m_types;
    std::unordered_map<std::string, std::shared_ptr<TypeInfo>> m_typesByName;
};

/**
 * @brief Helper class for registering types
 */
template<typename T>
class TypeRegistrar {
public:
    TypeRegistrar(const std::string& name) {
        m_typeInfo = std::make_shared<TypeInfo>(name, std::type_index(typeid(T)));
        
        // Set factory function
        m_typeInfo->setFactory([]() -> void* {
            return new T();
        });
        
        ReflectionRegistry::getInstance().registerType(m_typeInfo);
    }

    template<typename PropertyType>
    TypeRegistrar& property(const std::string& name, PropertyType T::*member, PropertyFlags flags = PropertyFlags::Serializable | PropertyFlags::EditorVisible) {
        auto propertyInfo = std::make_shared<PropertyInfo>(name, getPropertyType<PropertyType>(), flags);
        
        // Set getter
        propertyInfo->setGetter([member](void* instance) -> std::any {
            T* obj = static_cast<T*>(instance);
            return obj->*member;
        });
        
        // Set setter
        propertyInfo->setSetter([member](void* instance, const std::any& value) {
            T* obj = static_cast<T*>(instance);
            try {
                obj->*member = std::any_cast<PropertyType>(value);
            } catch (const std::bad_any_cast&) {
                // Handle type mismatch
            }
        });
        
        m_typeInfo->addProperty(propertyInfo);
        return *this;
    }

    template<typename PropertyType>
    TypeRegistrar& property(const std::string& name, 
                           std::function<PropertyType(const T*)> getter,
                           std::function<void(T*, PropertyType)> setter,
                           PropertyFlags flags = PropertyFlags::Serializable | PropertyFlags::EditorVisible) {
        auto propertyInfo = std::make_shared<PropertyInfo>(name, getPropertyType<PropertyType>(), flags);
        
        // Set getter
        propertyInfo->setGetter([getter](void* instance) -> std::any {
            const T* obj = static_cast<const T*>(instance);
            return getter(obj);
        });
        
        // Set setter
        propertyInfo->setSetter([setter](void* instance, const std::any& value) {
            T* obj = static_cast<T*>(instance);
            try {
                setter(obj, std::any_cast<PropertyType>(value));
            } catch (const std::bad_any_cast&) {
                // Handle type mismatch
            }
        });
        
        m_typeInfo->addProperty(propertyInfo);
        return *this;
    }

private:
    template<typename PropType>
    fresh::PropertyType getPropertyType() {
        if constexpr (std::is_same_v<PropType, bool>) {
            return fresh::PropertyType::Bool;
        } else if constexpr (std::is_same_v<PropType, int> || std::is_same_v<PropType, int32_t>) {
            return fresh::PropertyType::Int;
        } else if constexpr (std::is_same_v<PropType, float>) {
            return fresh::PropertyType::Float;
        } else if constexpr (std::is_same_v<PropType, double>) {
            return fresh::PropertyType::Double;
        } else if constexpr (std::is_same_v<PropType, std::string>) {
            return fresh::PropertyType::String;
        } else {
            return fresh::PropertyType::Unknown;
        }
    }

    std::shared_ptr<TypeInfo> m_typeInfo;
};

// Macro helpers for easy reflection registration
#define REFLECT_TYPE(Type) \
    static fresh::TypeRegistrar<Type> _##Type##_registrar(#Type)

#define REFLECT_PROPERTY(Type, Property) \
    _##Type##_registrar.property(#Property, &Type::Property)

#define REFLECT_PROPERTY_FLAGS(Type, Property, Flags) \
    _##Type##_registrar.property(#Property, &Type::Property, Flags)

} // namespace fresh
