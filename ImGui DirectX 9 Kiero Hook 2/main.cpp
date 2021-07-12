#include "includes.h"

template<typename T> T RPM(uintptr_t address) {
	try { return *(T*)address; }
	catch (...) { return T(); }
}

template<typename T> void WPM(uintptr_t address, T value) {
	try { *(T*)address = value; }
	catch (...) { return; }
}

struct Vec3 {
	float x, y, z;
	Vec3 operator+(Vec3 d) {
		return { x + d.x,y + d.y,z + d.z };
	}
	Vec3 operator-(Vec3 d) {
		return { x - d.x,y - d.y,z - d.z };
	}
	Vec3 operator*(float d) {
		return { x * d,y * d,z * d};
	}
	void normalize() {
		while (y < -180) {
			y = 360;
		}
		while (y > 180) {
			y = -360;
		}
		while (x > 89) {
			x = 89;
		}
		while (y < -89) {
			x = -89;
		}
	}

};

HANDLE process_handle;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

EndScene oEndScene = NULL;
WNDPROC oWndProc;
static HWND window = NULL;

void InitImGui(LPDIRECT3DDEVICE9 pDevice)
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(pDevice);
}
Vec3* punchAngle;
bool init = false;
bool show = false;
bool thirdPers = false;
bool Fov = false;
bool Esp = false;
int boneline = 9;
bool lineesp = false;
float BoxWitdh = 0.5;
struct ColorStr {
	int r, g, b, a;
};
ColorStr BoxColor;
int BoxThickness = 2;
int backingAlpha = 35;
bool drawBacking = true;

bool Recoil = false;
bool aimbot = false;
float NoRecAmount = 0.0;
int fov = 90;
Vec3 oPunch{ 0,0,0 };
DWORD clientModule;
DWORD engineModule;
DWORD LocalPlayer;
Vec3* aimRecoilPunch;
Vec3* ViewAngles;
int iShots;
bool bhopp = false;
bool trigger = false;
int delayTrigger = 100;
bool triggerbotdelay = false;
uintptr_t clientstate;

void HackInit() {
	clientModule = (DWORD)GetModuleHandle("client.dll");
	engineModule = (DWORD)GetModuleHandle("engine.dll");

	LocalPlayer = RPM<uintptr_t>(clientModule + dwLocalPlayer);
	punchAngle = (Vec3*)(LocalPlayer + m_aimPunchAngle);
	iShots = RPM<int>(LocalPlayer + m_iShotsFired);
	ViewAngles = (Vec3*)(*(uintptr_t*)(engineModule + dwClientState) + dwClientState_ViewAngles);
}
long __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice)
{
	if (!init)
	{
		HackInit();
		InitImGui(pDevice);
		init = true;
	}

	if (GetAsyncKeyState(VK_DELETE)) {
		kiero::shutdown();
		return 0;
	}

	if (GetAsyncKeyState(VK_INSERT) & 1)
	{
		show = !show;
	}

	if (show) {
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("0x3301 Cheats");

		ImGui::Checkbox("Third Person", &thirdPers);
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Checkbox("TriggerBot (Now off)", &trigger);
		if (trigger) {
			ImGui::Checkbox("Delay", &triggerbotdelay);
			if(triggerbotdelay){ 
				ImGui::SliderInt("Trigger Bot Delay (Milisecons)", &delayTrigger, 0, 1000); 
			}
		}
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Checkbox("Bhop", &bhopp);
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Checkbox("Fov Changer", &Fov);
		if (Fov) {
			ImGui::SliderInt("Fov", &fov, 0, 180);
		}
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Checkbox("Esp", &Esp);
		if (Esp) {
			ImGui::Checkbox("Esp Line", &lineesp);
		}
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Checkbox("Recoil Control (Maybe Work)", &Recoil);
		if (Recoil) {
			ImGui::SliderFloat("No Recoil Amount", &NoRecAmount, 0, 1);
		}
		
		ImGui::End();

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}
	return oEndScene(pDevice);
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
{
	DWORD wndProcId;
	GetWindowThreadProcessId(handle, &wndProcId);

	if (GetCurrentProcessId() != wndProcId)
		return TRUE; // skip to next window

	window = handle;
	return FALSE; // window found abort search
}

HWND GetProcessWindow()
{
	window = NULL;
	EnumWindows(EnumWindowsCallback, NULL);
	return window;
}

DWORD WINAPI MainThread(LPVOID lpReserved)
{
	bool attached = false;
	do
	{
		if (kiero::init(kiero::RenderType::D3D9) == kiero::Status::Success)
		{
			kiero::bind(42, (void**)&oEndScene, hkEndScene);
			do
				window = GetProcessWindow();
			while (window == NULL);
			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWL_WNDPROC, (LONG_PTR)WndProc);
			attached = true;
		}
	} while (!attached);
	return TRUE;
}

DWORD WINAPI CheatThread(LPVOID lpRes) {	
	while (LocalPlayer == NULL) {
		LocalPlayer = *(DWORD*)(clientModule + dwLocalPlayer);
	}
	while (true) {
		
		if (LocalPlayer != NULL) {
			//third person
			/*if (thirdPers) {
				*(int*)(LocalPlayer + m_iObserverMode) = 1;
			}
			else {
				*(int*)(LocalPlayer + m_iObserverMode) = 3;
			}*/
			//fov
			if (Fov) {
				*(int*)(LocalPlayer + m_iDefaultFOV) = fov;
			}
			//recoil control
			if (Recoil && !aimbot) {
				Vec3 punch = *punchAngle * (NoRecAmount * 2);
				if (iShots > 1 && GetAsyncKeyState(VK_LBUTTON)) {
					Vec3 newangle = *ViewAngles + oPunch - punch;
					newangle.normalize();
					*ViewAngles = newangle;
				}
				oPunch = punch;
			}
			if (bhopp) {
				int flags = *(int*)(LocalPlayer + m_fFlags);
				if (GetAsyncKeyState(VK_SPACE) && (flags == 257) & 1) {
					*(uintptr_t*)(clientModule + dwForceJump) = 5;
					*(uintptr_t*)(clientModule + dwForceJump) = 4;	
				}
			}
			if (trigger) {
				int crosshair = *(int*)(LocalPlayer + m_iCrosshairId);
				if (crosshair != 0 && crosshair < 64) {
					uintptr_t entity = *(uintptr_t*)(clientModule + dwEntityList + (crosshair - 1) * 0x10);
					int localteam = *(int*)(LocalPlayer + m_iTeamNum);
					int team = *(int*)(entity + m_iTeamNum);
					int health = *(int*)(entity + m_iHealth);
					if ((team != localteam) & (health > 0 && health < 101)) {
						if (triggerbotdelay) {
							Sleep(delayTrigger);
							*(int*)(clientModule + dwForceAttack) = 5;
							Sleep(20);
							*(int*)(clientModule + dwForceAttack) = 4;
						}
						else {
							*(int*)(clientModule + dwForceAttack) = 5;
							Sleep(20);
							*(int*)(clientModule + dwForceAttack) = 4;
						}
					}
				}
			}
		}
		
	}
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hMod);
		CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
		CreateThread(nullptr, 0, CheatThread, hMod, 0, nullptr);
		break;
	case DLL_PROCESS_DETACH:
		kiero::shutdown();
		break;
	}
	return TRUE;
}