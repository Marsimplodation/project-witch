#ifndef SOULSHARD_ECS
#define SOULSHARD_ECS
#include <cstddef>
#include <cstdio>
#include <vector>
#include <cstring>
using EntityID = unsigned int;
using TypeID = unsigned int;
#ifndef MAX_ECS_TYPES
#define MAX_ECS_TYPES 64
#endif // !MAX_ECS_TYPES
TypeID INCREASE_TYPE_COUNTER();

struct OptionalEntityID {
    bool hasValue();
    EntityID getValue();
    OptionalEntityID& operator=(const EntityID a);
private:
    EntityID value = 0;
};

#ifdef ECS_IMPLEMENTATION
bool OptionalEntityID::hasValue(){
    return (this->value != 0);
}
EntityID OptionalEntityID::getValue(){
    return (this->value - 1);
}
OptionalEntityID&  OptionalEntityID::operator=(const EntityID id) {
    this->value = id + 1;
    return *this;
};
#endif

struct ECS {
    static EntityID newEntity();
    static void removeEntity(EntityID entity);
    static void clear();
    template <typename T>
        static void addComponent(EntityID entity, const T& component);
    template <typename T>
        static void removeComponent(EntityID entity);
    template <typename T>
        static T* getComponent(EntityID entity);

    static void* getComponentByID(EntityID entity, TypeID typeIdx);
    static void addComponentByID(EntityID entity, TypeID typeIdx, size_t size);
    static void removeComponentByID(EntityID entity, TypeID typeIdx);

    template <typename T>
        static size_t getTotalTypeSize();
    template <typename T>
        static TypeID getTypeIndex();
    template <typename T>
        static void registerType();

private:
    //private types/
    struct ComponentPool {
        std::vector<size_t> unusedSpace;
        std::vector<char> data; 
    };

    template <typename T>
    struct ECSType {
        const static TypeID id;  
    };
    struct EntityTypeMap {
        size_t offsets[MAX_ECS_TYPES];  
    };
    //private methods
    inline static bool hasComponent(EntityID entity, TypeID typeIdx);

    //private members
    const static size_t DOES_NOT_HAVE_COMPONENT = 0;
    static ComponentPool componentPools[MAX_ECS_TYPES];
    static std::vector<EntityTypeMap> entityMap;
    static EntityID _entityCount;
    static std::vector<EntityID> unusedEntities;
    static TypeID _maxTypeID;
};


//-------- IMPLEMENTATION ---------------//
template <typename T>
const TypeID ECS::ECSType<T>::id = []{
    const static TypeID counter = INCREASE_TYPE_COUNTER(); 
    return counter;
}();

inline bool ECS::hasComponent(EntityID entity, TypeID typeIdx) {
    return entityMap[entity].offsets[typeIdx] != DOES_NOT_HAVE_COMPONENT;
}
template <typename T>
size_t ECS::getTotalTypeSize() {
    auto mod = sizeof(T) % alignof(T);
    auto padding = (mod != 0) ? alignof(T) : 0;
    auto paddedSize = (sizeof(T) - mod) + padding;
    return paddedSize;
}

template <typename T>
void ECS::addComponent(EntityID entity, const T& component) {
    TypeID typeIdx = getTypeIndex<T>(); 
    if(entity >= _entityCount) return;
    auto & data = componentPools[typeIdx].data;
    auto & unused = componentPools[typeIdx].unusedSpace;

    size_t offset = 0;
    if(unused.size() == 0) {
        offset = data.size();
        auto paddedSize = getTotalTypeSize<T>();
        data.resize(offset + paddedSize);
    } else {
        offset = unused.back();
        unused.pop_back();
    }

    std::memcpy(&data[offset], &component, sizeof(T));
    entityMap[entity].offsets[typeIdx] = offset + 1;
}

template <typename T>
void ECS::removeComponent(EntityID entity) {
    TypeID typeIdx = getTypeIndex<T>(); 
    if(entity >= _entityCount) return;
    if(!hasComponent(entity, typeIdx)) return;

    auto & data = componentPools[typeIdx].data;
    auto & unused = componentPools[typeIdx].unusedSpace;
    auto offset = entityMap[entity].offsets[typeIdx];
    unused.push_back(offset);
    entityMap[entity].offsets[typeIdx] = DOES_NOT_HAVE_COMPONENT;
}



template <typename T>
T* ECS::getComponent(EntityID entity) {
    TypeID typeIdx = getTypeIndex<T>(); 
    if(entity >= _entityCount) return nullptr;
    if(!hasComponent(entity, typeIdx)) return nullptr;

    auto & pool = componentPools[typeIdx];
    auto & data = pool.data;

    unsigned int offset = entityMap[entity].offsets[typeIdx] - 1;
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
}
void ECS_BENCHMARK();


#ifdef ECS_IMPLEMENTATION

void ECS::removeComponentByID(EntityID entity, TypeID typeIdx) {
    if(entity >= _entityCount) return;
    if(!hasComponent(entity, typeIdx)) return;

    auto & data = componentPools[typeIdx].data;
    auto & unused = componentPools[typeIdx].unusedSpace;
    auto offset = entityMap[entity].offsets[typeIdx];
    unused.push_back(offset);
    entityMap[entity].offsets[typeIdx] = DOES_NOT_HAVE_COMPONENT;
}

void* ECS::getComponentByID(EntityID entity, TypeID typeIdx) {
    if(entity >= _entityCount) return nullptr;
    if(!hasComponent(entity, typeIdx)) return nullptr;

    auto & pool = componentPools[typeIdx];
    auto & data = pool.data;

    unsigned int offset = entityMap[entity].offsets[typeIdx] - 1;
    // Cast bytes back to struct
    return (&data[offset]); 
}

void ECS::addComponentByID(EntityID entity, TypeID typeIdx, size_t totalSize) {
    if(entity >= _entityCount) return;
    auto & data = componentPools[typeIdx].data;
    auto & unused = componentPools[typeIdx].unusedSpace;

    size_t offset = 0;
    if(unused.size() == 0) {
        offset = data.size();
        data.resize(offset + totalSize);
    } else {
        offset = unused.back();
        unused.pop_back();
    }

    std::memset(&data[offset], 0, totalSize);
    entityMap[entity].offsets[typeIdx] = offset + 1;
}

EntityID ECS::newEntity() {
    if(unusedEntities.size() > 0) {
        auto id = unusedEntities.back();
        unusedEntities.pop_back();
        return id;
    }
    const int batchCount = 100;
    if(_entityCount % batchCount == 0) {
        auto factor = ((_entityCount - (_entityCount % batchCount)) / batchCount) + 1;
        entityMap.resize(factor * batchCount);
    }
    return _entityCount++;
}
void ECS::clear() {
    _entityCount = 0;
    _maxTypeID = 0;
    entityMap.clear();
    for(auto & pool : componentPools) pool.data.clear();
}
TypeID INCREASE_TYPE_COUNTER() {
    static TypeID GLOBAL_STATICS_ECS_TYPE_COUNTER;
    return GLOBAL_STATICS_ECS_TYPE_COUNTER++; 
}

void ECS::removeEntity(EntityID entity) {
    if(entity >= _entityCount) return;
    for(auto e : unusedEntities) if(e == entity) return;
    for(TypeID typeIdx = 0; typeIdx <= _maxTypeID; ++typeIdx) {
        if(!hasComponent(entity, typeIdx)) continue;
        auto & data = componentPools[typeIdx].data;
        auto & unused = componentPools[typeIdx].unusedSpace;
        auto offset = entityMap[entity].offsets[typeIdx];
        unused.push_back(offset);
    }
    unusedEntities.push_back(entity);
}

EntityID ECS::_entityCount=0;
TypeID ECS::_maxTypeID=0;
ECS::ComponentPool ECS::componentPools[MAX_ECS_TYPES];
std::vector<ECS::EntityTypeMap> ECS::entityMap(0);
std::vector<EntityID> ECS::unusedEntities(0);
#endif // !ECS_IMPLEMEMENTATION

#endif // !SOULSHARD_ECS

