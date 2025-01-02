#include "NodeEditor.h"
#include "imgui.h"
#include <string>

ImVec2 getRenderPosition(ImVec2 pos) {
    auto wPos = ImGui::GetWindowPos();

    return { pos.x + wPos.x, pos.y + wPos.y}; 
}

void NodeEditor::drawNode(Node& node) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    // Node background
    ImVec2 nodePos = node.position;
    ImVec2 nodeEnd = ImVec2(node.position.x + node.size.x, node.position.y + node.size.y);
    drawList->AddRectFilled(
        getRenderPosition(node.position),
        getRenderPosition(nodeEnd), IM_COL32(50, 50, 150, 255));
    
    // Node title
    drawList->AddText(getRenderPosition(ImVec2(node.position.x + 10, node.position.y + 5)), IM_COL32(255, 255, 255, 255), "Node");

    // Draw pins
    float pinOffsetY = 20.0f;
    for (Pin& input : node.inputs) {
        input.position = ImVec2(node.position.x - 10, node.position.y + pinOffsetY);
        input.position = getRenderPosition(input.position);
        drawList->AddCircleFilled(input.position, 5.0f, IM_COL32(200, 100, 100, 255));
        pinOffsetY += 20.0f;
    }

    pinOffsetY = 20.0f;
    for (Pin& output : node.outputs) {
        output.position = ImVec2(node.position.x + node.size.x + 10, node.position.y + pinOffsetY);
        output.position = getRenderPosition(output.position);
        drawList->AddCircleFilled(output.position, 5.0f, IM_COL32(100, 200, 100, 255));
        pinOffsetY += 20.0f;
    }
}

void HandleNodeDragging(Node& node) {
    ImGui::SetCursorScreenPos(getRenderPosition(node.position));
    ImGui::InvisibleButton(("node_drag" + std::to_string(node.id)).c_str(), node.size);

    if (ImGui::IsItemActive()) {
        ImVec2 delta = ImGui::GetIO().MouseDelta;
        node.position.x += delta.x;
        node.position.y += delta.y;
    }
}

void NodeEditor::drawLinks() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    for (const Link& link : links) {
        Node& fromNode = nodes[link.fromNode];
        Node& toNode = nodes[link.toNode];

        Pin& fromPin = fromNode.outputs[0]; // Simplification for example
        Pin& toPin = toNode.inputs[0];     // Simplification for example


        drawList->AddBezierCubic(
            fromPin.position,
            ImVec2(fromPin.position.x + 50, fromPin.position.y),
            ImVec2(toPin.position.x - 50, toPin.position.y),
            toPin.position,
            IM_COL32(255, 255, 0, 255),
            2.0f
        );
    }
}

void NodeEditor::draw() {
    for (Node& node : nodes) {
        HandleNodeDragging(node);
        drawNode(node);
    }
    drawLinks();
}
