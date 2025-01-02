#ifndef NODE_EDITOR_H
#define NODE_EDITOR_H

#include "imgui.h"
#include <vector>
struct Pin {
    int id;
    ImVec2 position; // Relative to the node
};

struct Node {
    int id;
    ImVec2 position;
    ImVec2 size;
    std::vector<Pin> inputs;
    std::vector<Pin> outputs;
};

struct Link {
    int fromNode;
    int fromPin;
    int toNode;
    int toPin;
};

struct NodeEditor {
    std::vector<Node> nodes;
    std::vector<Link> links;
    void draw();
    void drawLinks();
    void drawNode(Node& node);
};

#endif // !NODE_EDITOR_H
