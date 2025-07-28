// FlowGraph Editor ImGui Configuration for Test Engine
// This file enables ImGui Test Engine integration

// Enable test engine in ImGui
#define IMGUI_ENABLE_TEST_ENGINE

// Enable coroutine implementation using std::thread
#define IMGUI_TEST_ENGINE_ENABLE_COROUTINE_STDTHREAD_IMPL 1

// Enable screen capture functionality
#define IMGUI_TEST_ENGINE_ENABLE_CAPTURE 1

// Disable ImPlot support for now (can be enabled later if needed)
#define IMGUI_TEST_ENGINE_ENABLE_IMPLOT 0

// Use function pointers instead of std::function for better compatibility
#define IMGUI_TEST_ENGINE_ENABLE_STD_FUNCTION 0

// Debug break macro definitions
#ifndef IM_DEBUG_BREAK
#if defined (_MSC_VER)
#define IM_DEBUG_BREAK()    __debugbreak()
#elif defined(__clang__)
#define IM_DEBUG_BREAK()    __builtin_debugtrap()
#elif defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
#define IM_DEBUG_BREAK()    __asm__ volatile("int $0x03")
#elif defined(__GNUC__) && defined(__thumb__)
#define IM_DEBUG_BREAK()    __asm__ volatile(".inst 0xde01")
#elif defined(__GNUC__) && defined(__arm__) && !defined(__thumb__)
#define IM_DEBUG_BREAK()    __asm__ volatile(".inst 0xe7f001f0");
#else
#define IM_DEBUG_BREAK()    IM_ASSERT(0)
#endif
#endif

// Custom assert macro for test engine
extern void ImGuiTestEngine_AssertLog(const char* expr, const char* file, const char* func, int line);
#define IM_TEST_ENGINE_ASSERT(_EXPR) do { if ((void)0, !(_EXPR)) { ImGuiTestEngine_AssertLog(#_EXPR, __FILE__, __func__, __LINE__); IM_DEBUG_BREAK(); } } while (0)

// Bind main assert macro to test engine assert for better error recovery
#ifndef IM_ASSERT
#define IM_ASSERT(_EXPR) IM_TEST_ENGINE_ASSERT(_EXPR)
#endif