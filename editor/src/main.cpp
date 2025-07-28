#include "EditorApp.hpp"
#include <iostream>

int main() {
    FlowGraph::Editor::EditorApp app;
    
    if (!app.Initialize()) {
        std::cerr << "Failed to initialize FlowGraph Editor" << std::endl;
        return -1;
    }
    
    int result = app.Run();
    
    app.Shutdown();
    
    return result;
}