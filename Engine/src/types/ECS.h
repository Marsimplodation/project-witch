#ifndef SOULSHARD_ECS
#define SOULSHARD_ECS
#include "defines.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <cstring>
using EntityID = u32;
using TypeID = u32;
const TypeID MAX_ECS_TYPES = 64;
static TypeID GLOBAL_STATICS_ECS_TYPE_COUNTER = 0;

// Per-component type pool
struct ComponentPool {
    std::vector<u8> data; //contiguous memory for all components of a type
};

template <typename T>
struct ECSType {
    const static TypeID id;  
};
struct EntityTypeMap {
    size_t offsets[MAX_ECS_TYPES];  
    bool hasType[MAX_ECS_TYPES];  
};

// Generate unique TypeID for each component type
template <typename T>
const TypeID ECSType<T>::id = [] {
    const static TypeID counter = GLOBAL_STATICS_ECS_TYPE_COUNTER++;
    return counter;
}();

struct ECS {
    EntityID newEntity();
    template <typename T>
        void addComponent(EntityID entity, const T& component);
    template <typename T>
        T* getComponent(EntityID entity);
    template <typename T>
        TypeID getTypeIndex();
    template <typename T>
        void registerType();

private:
    std::vector<ComponentPool> componentPools;
    std::vector<EntityTypeMap> entityMap;
    EntityID _entityCount = 0;
    TypeID _maxTypeID = 0;
};


//-------- IMPLEMENTATION ---------------//
template <typename T>
void ECS::addComponent(EntityID entity, const T& component) {
    TypeID typeIdx = getTypeIndex<T>(); 
    auto & data = componentPools[typeIdx].data;

    // Allocate space in the global data vector
    size_t offset = data.size();
    auto mod = sizeof(T) % alignof(T);
    auto padding =  ((mod != 0) ? alignof(T) : 0);
    auto paddedSize = (sizeof(T) - mod) + padding;
    data.resize(offset + paddedSize);

    // Copy component bytes into data vector
    std::memcpy(&data[offset], &component, sizeof(T));

    // Store offset metadata for the entity
    entityMap[entity].offsets[typeIdx] = offset;
    entityMap[entity].hasType[typeIdx] = true;
}

template <typename T>
T* ECS::getComponent(EntityID entity) {
    TypeID typeIdx = getTypeIndex<T>(); 
    if(!entityMap[entity].hasType[typeIdx]) return 0x0;

    auto& pool = componentPools[typeIdx];
    auto & data = pool.data;

    u32 offset = entityMap[entity].offsets[typeIdx];
    return (T*)(&data[offset]); // Cast bytes back to struct
    return nullptr;
}

template <typename T>
TypeID ECS::getTypeIndex() {
    TypeID typeIdx = ECSType<T>::id;
    return typeIdx;
}

template <typename T>
void ECS::registerType() {
    TypeID typeIdx = ECSType<T>::id;

    //----- Type registration ------//
    if(typeIdx >= _maxTypeID) {
        _maxTypeID = typeIdx;
        componentPools.push_back(ComponentPool{
            .data = std::vector<u8>(),
        });
    }
}
void ECS_BENCHMARK();


#ifdef ECS_IMPLEMENTATION
EntityID ECS::newEntity() {
    const int batchCount = 100;
    if(_entityCount % batchCount == 0) {
        auto factor = ((_entityCount - (_entityCount % batchCount)) / batchCount) + 1;
        entityMap.resize(factor * batchCount);
    }
    return _entityCount++;
}
#endif // !ECS_IMPLEMEMENTATION

#endif // !SOULSHARD_ECS
