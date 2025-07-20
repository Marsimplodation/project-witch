#ifndef TINY_ECS_UI_H
#define TINY_ECS_UI_H
#include "ECS.h"
#include "imgui.h"
#include <string>
#include <vector>

struct ECS_UI {
    enum ELEMENT_TYPE {
        FLOAT,
        INT,
        VEC2,
        VEC3,
        VEC4,
        COLOR,
        BOOL
    };
    enum RENDER_TYPE {
        CHILD,
        OWN_WINDOW
    };

    template<typename T>
    static void registerType(const char *name);
    template<typename T>
    static void addToType(const char *name, size_t offset, ELEMENT_TYPE);

    static void renderEntityList(EntityID * enitity, int amount);
    static void renderEntityInspector(RENDER_TYPE mode = OWN_WINDOW);


    static OptionalEntityID currentEntity;
private:
    struct UIComponent {
        struct ComponentData {
            ELEMENT_TYPE type;
            size_t offset;
            std::string name;
        };
        
        bool defined = false;
        TypeID id;
        size_t size;
        std::string name;
        std::vector<ComponentData> data;
    };
    static UIComponent uiComponents[MAX_ECS_TYPES];
};


// ---- Implementation ----- ///
template<typename T>
void ECS_UI::registerType(const char *name) {
    TypeID id = ECS::getTypeIndex<T>();
    size_t size = ECS::getTotalTypeSize<T>();
    uiComponents[id].id = id;
    uiComponents[id].size = size;
    uiComponents[id].name = std::string(name);
    uiComponents[id].defined = true; 
}

template<typename T>
void ECS_UI::addToType(const char *name, size_t offset, ELEMENT_TYPE type) {
    TypeID id = ECS::getTypeIndex<T>();
    uiComponents[id].data.push_back(
    {
        .type = type,
        .offset = offset,
        .name = name
    }
    );
}

#ifdef ECS_IMPLEMENTATION
void ECS_UI::renderEntityList(EntityID * entities, int amount) {
    ImGui::Begin("Scene");
    int instanceIndex = 0;
    /*maybe usefull later
    if (ImGui::BeginPopupContextWindow()) { // Right-click anywhere in the window
        if (ImGui::MenuItem("Add PointLight")) {
            engine.scene.createPointLight();
        }
        ImGui::EndPopup();
    }
    */
    for(int i = 0; i < amount; ++i) {
        auto name = std::string("Entity ") + std::to_string(entities[i]);
        if (ImGui::Selectable(name.c_str())) {  // Make the text clickable
            ECS_UI::currentEntity = entities[i]; 
        }
    }
    ImGui::End();
}
void ECS_UI::renderEntityInspector(ECS_UI::RENDER_TYPE mode) {
    if(!currentEntity.hasValue()) return;
    auto currentEntity = ECS_UI::currentEntity.getValue(); 
    if(mode == OWN_WINDOW) {
        ImGui::Begin("Inspector");
        auto eName = std::string("Entity ") + std::to_string(currentEntity);
        ImGui::Text("%s", eName.c_str());;
    }
    else if (mode == CHILD)ImGui::BeginChild("Components");

    auto renderHelper = [&](UIComponent::ComponentData & entry, void * ptr){
        switch (entry.type) {
            case FLOAT:
            ImGui::DragFloat(entry.name.c_str(), (float*)ptr);
            break;
            case INT:
            ImGui::DragInt(entry.name.c_str(), (int*)ptr);
            break;
            case VEC2:
            ImGui::DragFloat2(entry.name.c_str(), (float*)ptr);
            break;
            case VEC3:
            ImGui::DragFloat3(entry.name.c_str(), (float*)ptr);
            break;
            case VEC4:
            ImGui::DragFloat4(entry.name.c_str(), (float*)ptr);
            break;
            case COLOR:
            ImGui::ColorEdit3(entry.name.c_str(), (float*)ptr);
            break;
            case BOOL:
            ImGui::Checkbox(entry.name.c_str(), (bool*)ptr);
            break;
            default:
            break;
        }
    };
    for(auto & type : uiComponents) {
        if(!type.defined) continue;
        float entryHeight = ImGui::GetFrameHeightWithSpacing() * 1.2f;
        void * ptr = ECS::getComponentByID(currentEntity, type.id); 
        if(!ptr) continue;
        ImGui::BeginChild(type.name.c_str(), ImVec2(0,entryHeight * (type.data.size() + 1)),true);
        ImGui::Text("%s", type.name.c_str());
        ImGui::SameLine();
        if(ImGui::Button("X")) {
            ECS::removeComponentByID(currentEntity, type.id);
            ImGui::EndChild();
            continue;
        };

        for(auto & entry : type.data) {
            renderHelper(entry, (u8*)ptr + entry.offset);
        }
        ImGui::EndChild();
    }

    static bool closePopup = false;
    if (ImGui::Button("New Component")) {
        closePopup = false;
        ImGui::OpenPopup("Select Component");
    }
    if (!closePopup && ImGui::BeginPopupModal("Select Component")) {
        closePopup = ImGui::Button("Cancel");
        for(auto & type : uiComponents) {
            auto data = ECS::getComponentByID(currentEntity, type.id);
            if(data) continue;
            if(ImGui::Button(type.name.c_str())) {
                closePopup = true;
                ECS::addComponentByID(currentEntity, type.id, type.size);
            }
        }
        ImGui::EndPopup();
    }
   
    if(mode == OWN_WINDOW) ImGui::End();
    else if (mode == CHILD)ImGui::EndChild();

};
ECS_UI::UIComponent ECS_UI::uiComponents[MAX_ECS_TYPES];
OptionalEntityID ECS_UI::currentEntity;
#endif


#endif // !TINY_ECS_UI_H

