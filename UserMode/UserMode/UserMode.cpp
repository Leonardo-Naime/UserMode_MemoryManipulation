// clang-format off
#include <Windows.h>
#include <TlHelp32.h>
// clang-format on
#include <iostream>

// Offsets — build 14160 (May 2026), source: cs2-dumper
uintptr_t dwLocalPlayerPawn        = 0x2056700;
uintptr_t dwLocalPlayerController  = 0x230A4F0;
uintptr_t dwEntityList             = 0x24D0DC0;

uintptr_t m_iHealth                = 0x34C;
uintptr_t m_flFlashDuration        = 0x1400;
uintptr_t m_pCameraServices        = 0x1218;
uintptr_t m_iFOV                   = 0x290;
uintptr_t m_pWeaponServices        = 0x11E0;
uintptr_t m_hActiveWeapon          = 0x60;
uintptr_t m_iClip1                 = 0x16D8;
uintptr_t m_pReserveAmmo           = 0x16E0;
uintptr_t m_pInGameMoneyServices   = 0x808;
uintptr_t m_iAccount               = 0x40;

DWORD GetProcessId(const wchar_t *procName) {
  DWORD procId = 0;
  HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

  if (hSnap != INVALID_HANDLE_VALUE) {
    PROCESSENTRY32W procEntry;
    procEntry.dwSize = sizeof(procEntry);

    if (Process32FirstW(hSnap, &procEntry)) {
      do {
        if (!_wcsicmp(procEntry.szExeFile, procName)) {
          procId = procEntry.th32ProcessID;
          break;
        }
      } while (Process32NextW(hSnap, &procEntry));
    }
  }
  CloseHandle(hSnap);
  return procId;
}

uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t *modName) {
  uintptr_t modBaseAddr = 0;
  HANDLE hSnap =
      CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);

  if (hSnap != INVALID_HANDLE_VALUE) {
    MODULEENTRY32W modEntry;
    modEntry.dwSize = sizeof(modEntry);

    if (Module32FirstW(hSnap, &modEntry)) {
      do {
        if (!_wcsicmp(modEntry.szModule, modName)) {
          modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
          break;
        }
      } while (Module32NextW(hSnap, &modEntry));
    }
  }
  CloseHandle(hSnap);
  return modBaseAddr;
}

template <typename T> T ReadMem(HANDLE hProcess, uintptr_t address) {
  T value{};
  ReadProcessMemory(hProcess, (LPCVOID)address, &value, sizeof(T), NULL);
  return value;
}

// Resolves a Source 2 CHandle to a raw entity pointer.
// Entity system: 64 pages of 512 entries, stride = 0x70.
uintptr_t ResolveEntityHandle(HANDLE hProcess, uintptr_t clientBase,
                              uint32_t handle) {
  if (!handle || handle == 0xFFFFFFFF) return 0;

  uint32_t entityIndex = handle & 0x7FFF;
  uint32_t pageIndex = entityIndex >> 9;
  uint32_t slot = entityIndex & 0x1FF;

  if (pageIndex >= 64) return 0;

  uintptr_t entitySystem = ReadMem<uintptr_t>(hProcess, clientBase + dwEntityList);
  if (!entitySystem) return 0;

  uintptr_t page = ReadMem<uintptr_t>(hProcess, entitySystem + 0x10 + (uintptr_t)pageIndex * 8);
  if (!page) return 0;

  uintptr_t identity = page + (uintptr_t)slot * 0x70;

  uint32_t storedHandle = ReadMem<uint32_t>(hProcess, identity + 0x10);
  if ((storedHandle & 0x7FFF) != entityIndex) return 0;

  return ReadMem<uintptr_t>(hProcess, identity);
}

int main() {
  SetConsoleTitleW(L"CS Capstone Experiment 1 - FREEZE MODE");
  std::cout << "--- CAPSTONE: SERVER RESISTANCE TEST ---\n";

  DWORD procId = GetProcessId(L"cs2.exe");
  if (!procId) {
    std::cout << "[ERROR] CS2 process not found.\n";
    return 0;
  }

  uintptr_t clientBase = GetModuleBaseAddress(procId, L"client.dll");
  HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId);

  if (!hProcess) {
    std::cout << "[ERROR] Failed to open process handle.\n";
    return 0;
  }

  int cheatAmmoValue = 1337;
  int cheatHealthValue = 1337;
  int cheatMoneyValue = 1337;
  int cheatFovValue = 150;
  float cheatFlashValue = 0.0f;

  bool bAmmoCheatActive = false;

  std::cout << "[OK] Connected to memory.\n";
  std::cout << ">> Press 'P' to TOGGLE Hacks (Ammo, FOV, NoFlash, Money).\n";
  std::cout << ">> Press 'INSERT' to Exit.\n\n";

  while (true) {
    if (GetAsyncKeyState('P') & 1) {
      bAmmoCheatActive = !bAmmoCheatActive;

      if (bAmmoCheatActive) {
        std::cout << "[ON] CHEATS ACTIVE! Injecting memory...\n";
      } else {
        std::cout << "[OFF] CHEATS DEACTIVATED.\n";
      }
    }

    if (GetAsyncKeyState(VK_INSERT) & 1)
      break;

    uintptr_t localPlayer =
        ReadMem<uintptr_t>(hProcess, clientBase + dwLocalPlayerPawn);
    uintptr_t playerController =
        ReadMem<uintptr_t>(hProcess, clientBase + dwLocalPlayerController);

    if (bAmmoCheatActive) {
      if (localPlayer) {
        WriteProcessMemory(hProcess, (LPVOID)(localPlayer + m_iHealth),
                           &cheatHealthValue, sizeof(cheatHealthValue), NULL);

        WriteProcessMemory(hProcess, (LPVOID)(localPlayer + m_flFlashDuration),
                           &cheatFlashValue, sizeof(cheatFlashValue), NULL);

        uintptr_t cameraServices =
            ReadMem<uintptr_t>(hProcess, localPlayer + m_pCameraServices);
        if (cameraServices) {
          WriteProcessMemory(hProcess, (LPVOID)(cameraServices + m_iFOV),
                             &cheatFovValue, sizeof(cheatFovValue), NULL);
        }

        uintptr_t weaponServices = ReadMem<uintptr_t>(hProcess, localPlayer + m_pWeaponServices);
        if (weaponServices) {
          uint32_t weaponHandle = ReadMem<uint32_t>(hProcess, weaponServices + m_hActiveWeapon);
          uintptr_t currentWeapon = ResolveEntityHandle(hProcess, clientBase, weaponHandle);
          if (currentWeapon) {
            WriteProcessMemory(hProcess, (LPVOID)(currentWeapon + m_iClip1),
                               &cheatAmmoValue, sizeof(cheatAmmoValue), NULL);
            WriteProcessMemory(hProcess, (LPVOID)(currentWeapon + m_pReserveAmmo),
                               &cheatAmmoValue, sizeof(cheatAmmoValue), NULL);
          }
        }
      }

      if (playerController) {
        uintptr_t moneyServices = ReadMem<uintptr_t>(
            hProcess, playerController + m_pInGameMoneyServices);
        if (moneyServices) {
          WriteProcessMemory(hProcess, (LPVOID)(moneyServices + m_iAccount),
                             &cheatMoneyValue, sizeof(cheatMoneyValue), NULL);
        }
      }
    }

    int ammoCount = 0, reserveCount = 0, healthCount = 0, moneyCount = 0;

    if (localPlayer) {
      healthCount = ReadMem<int>(hProcess, localPlayer + m_iHealth);
      uintptr_t weaponServices = ReadMem<uintptr_t>(hProcess, localPlayer + m_pWeaponServices);
      if (weaponServices) {
        uint32_t weaponHandle = ReadMem<uint32_t>(hProcess, weaponServices + m_hActiveWeapon);
        uintptr_t currentWeapon = ResolveEntityHandle(hProcess, clientBase, weaponHandle);
        if (currentWeapon) {
          ammoCount = ReadMem<int>(hProcess, currentWeapon + m_iClip1);
          reserveCount = ReadMem<int>(hProcess, currentWeapon + m_pReserveAmmo);
        }

      }
    }
    if (playerController) {
      uintptr_t moneyServices = ReadMem<uintptr_t>(
          hProcess, playerController + m_pInGameMoneyServices);
      if (moneyServices) {
        moneyCount = ReadMem<int>(hProcess, moneyServices + m_iAccount);
      }
    }

    std::cout << "Status: " << (bAmmoCheatActive ? "[ACTIVE] " : "[IDLE]   ")
              << "HP: " << healthCount << " | $: " << moneyCount
              << " | Ammo: " << ammoCount << "/" << reserveCount << "      \r";

    Sleep(0);
  }

  CloseHandle(hProcess);
  return 0;
}
