#ifndef AoT_GUI_MASTER_H
#define AoT_GUI_MASTER_H
//
// This class is here to manage multiple Guis and handle issues that the client raises, namely resets.
// No rendering occurs in this class.
//

#include <d3d9.h>          // Direct3D9 API
#include <d3dx9.h>       // Direct3D9 Utility functions (optional for math and textures) kept in case its useful.
#include <Windows.h>
#include <string>
#include <iostream>
#include "detours.h"
#include <math.h>
#include <stdio.h>
#include <fenv.h>
#include <functional>
#include <vector>
#include <memory>
#include <sstream>
#include "AoT_Spell_UI.h"

// definitions
#define AoT_MIN_OPCODE 6500
#define AoT_MAX_OPCODE 6504


using DeviceCallback = void (*)(IDirect3DDevice9* device);

class D3D9Hooks
    // I think I can remove this class.
{
public:
    bool Init(std::string data);
    void Teardown()
    {
        initialized = false;
    }

private:




    bool hooked = false;
    FARPROC pDirect3DCreate9 = nullptr;
    IDirect3DDevice9* pD3DDevice = nullptr;
    bool initialized = false;
};

HRESULT WINAPI HookedBeginScene(IDirect3DDevice9* device);
HRESULT WINAPI HookedReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters);

class AoT_Gui_Master {
public:
    AoT_Gui_Master();
    ~AoT_Gui_Master();

    // Initialize the GUI at a specific index
    void InitializeGui(IDirect3DDevice9* dev, size_t index, std::string data);

    // Update all running GUIs. Im not entirely sure what you could need this for, but its here. 
    // May be more useful with a param
    void UpdateAll(IDirect3DDevice9* dev);
    void UpdateCon(DWORD* con);

    // Render all running GUIs
    void RenderAll(LPDIRECT3DDEVICE9);

    // Control whether a GUI is running
    void SetGuiRunning(size_t index, bool isRunning);

    bool IsRunning(size_t index);

    void CleanUp();

private:
    static constexpr size_t MaxGUIs = 1;
    AoT_Spells_UI*  spellsUI  = new AoT_Spells_UI();
    IDirect3DDevice9* device  = nullptr;
    bool running = false;
};

void updateCon(DWORD* );
void HandleOpcode(int16_t opcode, std::string data);

#endif  // AOT_GUI_MASTER_H
