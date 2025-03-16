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
TypeID INCREASE_TYPE_COUNTER();

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

struct ECS {
    static EntityID newEntity();
    template <typename T>
        static void addComponent(EntityID entity, const T& component);
    template <typename T>
        static T* getComponent(EntityID entity);
    template <typename T>
        static TypeID getTypeIndex();
    template <typename T>
        static void registerType();

private:
    static std::vector<ComponentPool> componentPools;
    static std::vector<EntityTypeMap> entityMap;
    static EntityID _entityCount;
    static TypeID _maxTypeID;
};


//-------- IMPLEMENTATION ---------------//
template <typename T>
const TypeID ECSType<T>::id = []{
    const static TypeID counter = INCREASE_TYPE_COUNTER(); 
    return counter;
}();

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

TypeID INCREASE_TYPE_COUNTER() {
    static TypeID GLOBAL_STATICS_ECS_TYPE_COUNTER;
    return GLOBAL_STATICS_ECS_TYPE_COUNTER++; 
}
EntityID ECS::_entityCount=0;
TypeID ECS::_maxTypeID=0;
std::vector<ComponentPool> ECS::componentPools(0);
std::vector<EntityTypeMap> ECS::entityMap(0);
#endif // !ECS_IMPLEMEMENTATION

#endif // !SOULSHARD_ECS
