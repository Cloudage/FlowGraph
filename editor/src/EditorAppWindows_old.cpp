// Windows-specific implementation for EditorApp
#ifdef _WIN32

#include <d3d11.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <imgui.h>
#include <imgui_impl_dx11.h>

#include <iostream>
#include "EditorApp.hpp"

extern "C" bool SetupRenderingBackendImpl(GLFWwindow* window, void** deviceOut, void** contextOut, void** swapChainOut, void** renderTargetOut, void** layerOut) {
    // Initialize DirectX 11 for Windows
    HWND hwnd = glfwGetWin32Window(window);
    
    // Get window dimensions
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    
    // Create DXGI factory
    ComPtr<IDXGIFactory2> dxgiFactory;
    HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
    if (FAILED(hr)) {
        std::cerr << "Failed to create DXGI factory" << std::endl;
        return false;
    }
    
    // Create D3D11 device and context
    D3D_FEATURE_LEVEL featureLevel;
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    
    ComPtr<ID3D11Device> d3dDevice;
    ComPtr<ID3D11DeviceContext> d3dContext;
    
    hr = D3D11CreateDevice(
        nullptr,                    // Use default adapter
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,                    // No software module
        D3D11_CREATE_DEVICE_DEBUG,  // Enable debug layer in debug builds
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &d3dDevice,
        &featureLevel,
        &d3dContext
    );
    
    if (FAILED(hr)) {
        std::cerr << "Failed to create D3D11 device" << std::endl;
        return false;
    }
    
    // Create swap chain
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = 0;
    
    ComPtr<IDXGISwapChain1> swapChain;
    hr = dxgiFactory->CreateSwapChainForHwnd(
        d3dDevice.Get(),
        hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
    );
    
    if (FAILED(hr)) {
        std::cerr << "Failed to create swap chain" << std::endl;
        return false;
    }
    
    // Create render target view
    ComPtr<ID3D11Texture2D> backBuffer;
    hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (FAILED(hr)) {
        std::cerr << "Failed to get back buffer" << std::endl;
        return false;
    }
    
    ComPtr<ID3D11RenderTargetView> renderTargetView;
    hr = d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTargetView);
    if (FAILED(hr)) {
        std::cerr << "Failed to create render target view" << std::endl;
        return false;
    }
    
    // Store results
    *deviceOut = d3dDevice.Detach();
    *contextOut = d3dContext.Detach();
    *swapChainOut = swapChain.Detach();
    *renderTargetOut = renderTargetView.Detach();
    *layerOut = nullptr; // Not used on Windows
    
    std::cout << "DirectX 11 initialized successfully" << std::endl;
    return true;
}

extern "C" bool InitializeImGuiImpl(GLFWwindow* window, float contentScaleX, float contentScaleY, void* metalDevice, void* d3dDevice, void* d3dContext) {
    // DirectX 11 backend for Windows
    return ImGui_ImplDX11_Init(static_cast<ID3D11Device*>(d3dDevice), static_cast<ID3D11DeviceContext*>(d3dContext));
}

extern "C" void StartImGuiFrameImpl(void* metalLayer) {
    // DirectX 11 new frame
    ImGui_ImplDX11_NewFrame();
}

extern "C" void RenderImGuiFrameImpl(void* metalDevice, void* metalCommandQueue, void* metalLayer, void* d3dDevice, void* d3dContext, void* renderTargetView, void* swapChain) {
    // DirectX 11 rendering for Windows
    const float clearColor[4] = { 0.25f, 0.25f, 0.25f, 1.00f };
    static_cast<ID3D11DeviceContext*>(d3dContext)->OMSetRenderTargets(1, reinterpret_cast<ID3D11RenderTargetView* const*>(&renderTargetView), nullptr);
    static_cast<ID3D11DeviceContext*>(d3dContext)->ClearRenderTargetView(static_cast<ID3D11RenderTargetView*>(renderTargetView), clearColor);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    
    // Present the frame
    static_cast<IDXGISwapChain1*>(swapChain)->Present(1, 0); // VSync enabled
}

extern "C" bool RecreateDirectXRenderTargetImpl(void* swapChain, void* d3dDevice, void** renderTargetView, int width, int height) {
    if (!swapChain || !d3dDevice) {
        return false;
    }
    
    IDXGISwapChain1* dx_swapChain = static_cast<IDXGISwapChain1*>(swapChain);
    ID3D11Device* dx_device = static_cast<ID3D11Device*>(d3dDevice);
    
    // Release existing render target view
    if (*renderTargetView) {
        static_cast<ID3D11RenderTargetView*>(*renderTargetView)->Release();
        *renderTargetView = nullptr;
    }
    
    // Resize the swap chain buffers
    HRESULT hr = dx_swapChain->ResizeBuffers(
        0,                              // Keep existing buffer count
        width,                          // New width
        height,                         // New height  
        DXGI_FORMAT_UNKNOWN,           // Keep existing format
        0                              // No flags
    );
    
    if (FAILED(hr)) {
        std::cerr << "Failed to resize swap chain buffers" << std::endl;
        return false;
    }
    
    // Recreate render target view with new buffer
    ComPtr<ID3D11Texture2D> backBuffer;
    hr = dx_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (FAILED(hr)) {
        std::cerr << "Failed to get back buffer after resize" << std::endl;
        return false;
    }
    
    ID3D11RenderTargetView* newRenderTargetView;
    hr = dx_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &newRenderTargetView);
    if (FAILED(hr)) {
        std::cerr << "Failed to create render target view after resize" << std::endl;
        return false;
    }
    
    *renderTargetView = newRenderTargetView;
    return true;
}

extern "C" void CleanupImGuiImpl() {
    ImGui_ImplDX11_Shutdown();
}

extern "C" void CleanupRenderingImpl(void** metalDevice, void** metalCommandQueue, void** metalLayer, void** d3dDevice, void** d3dContext, void** swapChain, void** renderTargetView) {
    // Cleanup DirectX resources
    if (*renderTargetView) {
        static_cast<ID3D11RenderTargetView*>(*renderTargetView)->Release();
        *renderTargetView = nullptr;
    }
    if (*swapChain) {
        static_cast<IDXGISwapChain1*>(*swapChain)->Release();
        *swapChain = nullptr;
    }
    if (*d3dContext) {
        static_cast<ID3D11DeviceContext*>(*d3dContext)->Release();
        *d3dContext = nullptr;
    }
    if (*d3dDevice) {
        static_cast<ID3D11Device*>(*d3dDevice)->Release();
        *d3dDevice = nullptr;
    }
}

float FlowGraph::Editor::EditorApp::GetStatusBarHeight() const {
    return 25.0f * std::max(m_contentScaleX, m_contentScaleY);
}

#endif // _WIN32