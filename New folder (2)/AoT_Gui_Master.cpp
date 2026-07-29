#include "AoT_Gui_Master.h"
#include "AoT_Spell_Book.h"
#include <windows.h>
#include <iostream>
AoT_Gui_Master gui_master = AoT_Gui_Master();
D3D9Hooks hooks = D3D9Hooks();

HRESULT __stdcall original_beginscene(IDirect3DDevice9* device);
HRESULT __stdcall original_reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params);
DETOUR_TRAMPOLINE_EMPTY(HRESULT __stdcall original_beginscene(IDirect3DDevice9* device));
DETOUR_TRAMPOLINE_EMPTY(HRESULT __stdcall original_reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params));


void updateCon(DWORD* con)
{
    if (gui_master.IsRunning(0))
        gui_master.UpdateCon(con);
}


AoT_Gui_Master::AoT_Gui_Master()
    : running(false) {}  // Initialize running to false

// Destructor
AoT_Gui_Master::~AoT_Gui_Master() {
    CleanUp();  // Ensure any remaining GUI is cleaned up
}


void AoT_Gui_Master::UpdateCon(DWORD* con) {
    if (running)
        spellsUI->UpdateCon(con);
}


// Initialize the GUI
void AoT_Gui_Master::InitializeGui(IDirect3DDevice9* dev, size_t index, std::string data) {
    if (index == 0 && !running) {  // Only supports one GUI for now
        device = dev;
        

        if (spellsUI->isOpen()) {
            printf("Warning, master and spellsUI desync.\n");
            return;
        }
        printf("test\n");
        running = spellsUI->InitializeWindow(device, data);
        
    }
}


// Update the GUI if it is running
void AoT_Gui_Master::UpdateAll(IDirect3DDevice9* dev) { 
    if (running) {
        spellsUI->UpdateDevice(dev);
    }
}

// Render the GUI if it is running
void AoT_Gui_Master::RenderAll(LPDIRECT3DDEVICE9 dev) {
    if (spellsUI->isOpen()) {
        spellsUI->Render(dev);
    } else if (running) {
        printf("close attempt\n");
        gui_master.CleanUp();
    }
}

// Set whether the GUI is running
void AoT_Gui_Master::SetGuiRunning(size_t index, bool isRunning) {
    if (index == 0) {  // Only one GUI is supported
        running = isRunning;
    }
}

// Check if the GUI is running
bool AoT_Gui_Master::IsRunning(size_t index) {
    if (index == 0)
        return running;
    return false;
}

// Clean up the GUI resources
void AoT_Gui_Master::CleanUp() {
    if (running) {
        running = false;
        device->Release();
        hooks.Teardown();
    }
}


bool D3D9Hooks::Init(std::string data) {
    if (initialized) {
        return true;
    }
    initialized = true;

    // Get the Direct3D 9 module handle
    auto hD3D9 = GetModuleHandle("d3d9.dll");
    if (!hD3D9) {
        std::cerr << "Failed to get d3d9.dll module handle." << std::endl;
        return false;
    }

    // Get the Direct3DCreate9 function address
    pDirect3DCreate9 = GetProcAddress(hD3D9, "Direct3DCreate9");
    if (!pDirect3DCreate9) {
        std::cerr << "Failed to get Direct3DCreate9 function address. Error: " << GetLastError() << std::endl;
        return false;
    }

    // Create a Direct3D 9 instance
    using Direct3DCreate9_t = IDirect3D9 * (WINAPI*)(UINT);
    auto Direct3DCreate9 = reinterpret_cast<Direct3DCreate9_t>(pDirect3DCreate9);
    auto pD3D9 = Direct3DCreate9(D3D_SDK_VERSION);
    if (!pD3D9) {
        std::cerr << "Failed to create IDirect3D9 instance." << std::endl;
        return false;
    }

    // Create a dummy device to get the vtable
    HWND hwnd = CreateWindowEx(0, "STATIC", nullptr, WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, nullptr, nullptr, nullptr, nullptr);
    D3DPRESENT_PARAMETERS d3dpp{};
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = hwnd;

    IDirect3DDevice9* pDummyDevice = nullptr;
    HRESULT hr = pD3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDummyDevice);
    if (FAILED(hr) || !pDummyDevice) {
        std::cerr << "Failed to create dummy IDirect3DDevice9 instance. HRESULT: " << hr << std::endl;
        pD3D9->Release();
        DestroyWindow(hwnd);
        return false;
    }

    auto vtable = *reinterpret_cast<void***>(pDummyDevice);

    // Hook the target function using EzDetour
    DWORD targetFuncAddr;
    if (!hooked)
    {
        targetFuncAddr = reinterpret_cast<DWORD>(vtable[42]);
        EzDetour(targetFuncAddr, &HookedBeginScene, &original_beginscene);
        targetFuncAddr = reinterpret_cast<DWORD>(vtable[16]);
        EzDetour(targetFuncAddr, &HookedReset, &original_reset);
        hooked = true;

    }

    // Set up the device pointer
    pD3DDevice = pDummyDevice;

    // Clean up after successful hook creation
    pD3D9->Release();
    DestroyWindow(hwnd);

    // Only initialize the old spell UI when there is actual spell data to show
    if (!data.empty()) {
        printf("check\n");
        gui_master.InitializeGui(pD3DDevice, 0, data);
    }
    return true;
}



bool IsExecutable(void* ptr)
{
    MEMORY_BASIC_INFORMATION mbi{};
    if (VirtualQuery(ptr, &mbi, sizeof(mbi)))
    {
        return (mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)) != 0;
    }
    return false; // Unable to query
}



HRESULT WINAPI HookedBeginScene(IDirect3DDevice9* device)
{
    //printf("rendering?");
    //gui_master.UpdateAll(device);
    if (!original_beginscene)
    {
        printf("\n no begin scene... uh oh");
        return 0;
    }
        
    if (gui_master.IsRunning(0))
        gui_master.RenderAll(device);
    //RenderWithSprite(device);
    return original_beginscene(device);
}

HRESULT WINAPI HookedReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    printf("reset occured.\n");
    if (!original_reset)
        return 0;
    HRESULT hr = device->TestCooperativeLevel();



    hr = original_reset(device, pPresentationParameters);
    return hr;
}

void HandleOpcode(int16_t opcode, std::string data)
{
    
    
    switch (opcode) {
    case 6500:
        if (gui_master.IsRunning(0))
            break;
        if (!hooks.Init(data))
        {
            printf("Something went wrong with hooks init.");
            return;
        }
        return;

    case 6501:
        // Window is now opened client-side via the SpellBook vtable hook.
        // This opcode is reserved for future server-pushed spell data.
        if (pAoTSpellBook)
            pAoTSpellBook->PopulateSpellList();
        return;

    case 6502: {
        // Roll options: payload = "level:sp1,sp2,sp3"
        if (!pAoTSpellBook || data.empty()) return;
        size_t colon = data.find(':');
        if (colon == std::string::npos) return;
        int level = std::stoi(data.substr(0, colon));
        std::vector<uint16_t> options;
        std::string rest = data.substr(colon + 1);
        std::stringstream ss(rest);
        std::string token;
        while (std::getline(ss, token, ',')) {
            if (!token.empty())
                options.push_back(static_cast<uint16_t>(std::stoi(token)));
        }
        if (!options.empty())
            pAoTSpellBook->ShowRollOptions(level, options);
        return;
    }

    case 6503: {
        // Pending restore: payload = "level:sp1,sp2,sp3"
        if (!pAoTSpellBook || data.empty()) return;
        size_t colon = data.find(':');
        if (colon == std::string::npos) return;
        int level = std::stoi(data.substr(0, colon));
        std::vector<uint16_t> options;
        std::string rest = data.substr(colon + 1);
        std::stringstream ss(rest);
        std::string token;
        while (std::getline(ss, token, ',')) {
            if (!token.empty())
                options.push_back(static_cast<uint16_t>(std::stoi(token)));
        }
        if (!options.empty())
            pAoTSpellBook->RestorePendingRoll(level, options);
        return;
    }

    case 6504: {
        // Discovered spells: payload = "P:sp1,sp2,...|D:sp3,sp4,..."
        if (!pAoTSpellBook || data.empty()) return;
        pAoTSpellBook->RestoreDiscovered(data);
        return;
    }

    default:
        // This shouldn't be possible without player intervention. Let's spook them a bit (:
        MessageBoxA(NULL, "Unexpected opcode received!", "Warning", MB_OK | MB_ICONEXCLAMATION);
    }
}
