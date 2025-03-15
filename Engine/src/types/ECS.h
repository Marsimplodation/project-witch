#ifndef SOULSHARD_ECS
#define SOULSHARD_ECS
#include "defines.h"
#include <cstddef>
#include <cstdio>
#include <vector>
#include <cstring>
using EntityID = u32;
using TypeID = u32;
const TypeID MAX_ECS_TYPES = 64;
static TypeID GLOBAL_STATICS_ECS_TYPE_COUNTER = 0;

struct ComponentPool {
    std::vector<u8> data; 
};

template <typename T>
struct ECSType {
    const static TypeID id;  
};
struct EntityTypeMap {
    size_t offsets[MAX_ECS_TYPES];  
    bool hasType[MAX_ECS_TYPES];  
};

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

    // adding a padding to size for proper alignement
    size_t offset = data.size();
    auto mod = sizeof(T) % alignof(T);
    auto padding = (mod != 0) ? alignof(T) : 0;
    auto paddedSize = (sizeof(T) - mod) + padding;
    data.resize(offset + paddedSize);

    std::memcpy(&data[offset], &component, sizeof(T));
    entityMap[entity].offsets[typeIdx] = offset;
    entityMap[entity].hasType[typeIdx] = true;
}

template <typename T>
T* ECS::getComponent(EntityID entity) {
    TypeID typeIdx = getTypeIndex<T>(); 
    if(!entityMap[entity].hasType[typeIdx]) return nullptr;

    auto & pool = componentPools[typeIdx];
    auto & data = pool.data;

    u32 offset = entityMap[entity].offsets[typeIdx];
    // Cast bytes back to struct
    return (T*)(&data[offset]); 
}

template <typename T>
TypeID ECS::getTypeIndex() {
    TypeID typeIdx = ECSType<T>::id;
    return typeIdx;
}

template <typename T>
void ECS::registerType() {
    TypeID typeIdx = ECSType<T>::id;
    if(typeIdx < _maxTypeID) return;
    _maxTypeID = typeIdx;
    componentPools.push_back(ComponentPool{
        .data = std::vector<u8>(),
    });
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
