#include "EditorApp.hpp"
#include <iostream>

int main() {
    auto app = FlowGraph::Editor::EditorApp::create();
    
    if (!app->Initialize()) {
        std::cerr << "Failed to initialize FlowGraph Editor" << std::endl;
        return -1;
    }
    
    int result = app->Run();
    
    app->Shutdown();
    
    return result;
}