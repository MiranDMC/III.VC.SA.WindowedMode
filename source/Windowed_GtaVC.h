#include "WindowedMode.h"

void WindowedMode::InitGtaVC()
{
	inst = new WindowedMode(
		GameTitle::GTA_VC,
		0x9B5F08, // gameState
		0x9B48D8, // rsGlobal
		0x7897A8, // d3dDevice
		0xA0FD04, // d3dPresentParams
		0x7897D0, // rwVideoModes
		0x642B40, // RwEngineGetNumVideoModes
		0x642BA0, // RwEngineGetCurrentVideoMode
		0x869630  // FrontEndMenuManager
	);

	// check for ASI loader
	if (inst->rsGlobal->ps && inst->rsGlobal->ps->window) // app window already created
		ShowError("ASI Loader is required for correct operation of this plugin!");

	strcpy_s(inst->windowClassName, "Grand theft auto 3");
	inst->windowIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(100));

	// Debug: check if somebody already modified memory we want to hook
#ifdef DEBUG
	VerifyMemory("CreateWindow", 0x5FFC75, 6, 0xFE9E2136);
	VerifyMemory("InitPresentationParams", 0x65C0B4, 6, 0x2C46D383);
	VerifyMemory("InitD3dDevice", 0x65C4E2, 6, 0xAE882A19);
#endif

	// patch call to CreateWindowExA
	injector::MakeNOP(0x5FFC75, 6);
	injector::MakeCALL(0x5FFC75, WindowedMode::InitWindow);

	// just before D3D device is created
	struct Patch_InitPresentationParams 
	{
		void operator()(injector::reg_pack& regs)
		{
			*(DWORD*)(0xA0FD24) = regs.ebx; // original action replaced by the patch

			inst->WindowCalculateGeometry();
		}
	}; injector::MakeInline<Patch_InitPresentationParams>(0x65C0B4, 0x65C0BA);

	// just after D3D device has been created
	struct Path_InitD3dDevice 
	{
		void operator()(injector::reg_pack& regs)
		{
			*(DWORD*)(0x789BF4) = regs.ebp; // original action replaced by the patch

			inst->InitD3dDevice();
		}
	}; injector::MakeInline<Path_InitD3dDevice>(0x65C4E2, 0x65C4E8);
}
