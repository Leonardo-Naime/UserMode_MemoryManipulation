# User Mode Memory Manipulation — CS2

External C++ application that manipulates Counter-Strike 2 memory at runtime using the Win32 API (Ring 3). Developed as part of an undergraduate thesis on game security (Computer Science, UTFPR-CM).

## How it works

1. Locates the `cs2.exe` process via `CreateToolhelp32Snapshot`
2. Resolves the `client.dll` base address (ASLR bypass)
3. Opens a `HANDLE` with `OpenProcess(PROCESS_ALL_ACCESS)`
4. Traverses Source 2 pointer chains with `ReadProcessMemory`
5. Overwrites target variables in a continuous loop with `WriteProcessMemory`

## Target Variables

| Variable | Offset (build 14160) | Authority |
|---|---|---|
| `m_iHealth` | `Pawn + 0x34C` | Server |
| `m_iClip1` | `Weapon + 0x16D8` | Client (partial) |
| `m_pReserveAmmo` | `Weapon + 0x16E0` | Server |
| `m_iAccount` | `MoneyServices + 0x40` | Server |
| `m_iFOV` | `CameraServices + 0x290` | Client |
| `m_flFlashDuration` | `Pawn + 0x1400` | Client |

## Prerequisites

- Windows 10/11 (x64)
- Visual Studio 2022 with "Desktop development with C++" workload
- Counter-Strike 2 running with the `-insecure` launch parameter

## Build and Run

1. Open `UserMode.sln` in Visual Studio
2. Set configuration to `Release | x64`
3. Build (`Ctrl+B`)
4. Launch CS2 with `-insecure` and start a local match with bots
5. Run `UserMode.exe`
6. Press `P` to toggle cheats on/off
7. Press `INSERT` to exit

## Updating Offsets

Offsets change with every CS2 update. Check the [cs2-dumper](https://github.com/a2x/cs2-dumper) project for updated values and replace the global variables at the top of `UserMode.cpp`.

## Disclaimer

This project was developed exclusively for academic purposes and offensive security research. Using it in online matches violates the game's terms of service and may result in a permanent ban.

---

# Versão em Português

Aplicação externa em C++ que manipula a memória do Counter-Strike 2 em tempo de execução utilizando a API Win32 (Ring 3). Desenvolvido como parte do TCC em Ciência da Computação na UTFPR-CM.

## Como funciona

1. Localiza o processo `cs2.exe` via `CreateToolhelp32Snapshot`
2. Resolve o endereço base da `client.dll` (contorno do ASLR)
3. Abre um `HANDLE` com `OpenProcess(PROCESS_ALL_ACCESS)`
4. Navega pelas cadeias de ponteiros da Source 2 com `ReadProcessMemory`
5. Sobrescreve as variáveis-alvo em laço contínuo com `WriteProcessMemory`

## Variáveis-alvo

| Variável | Offset (build 14160) | Autoridade |
|---|---|---|
| `m_iHealth` | `Pawn + 0x34C` | Servidor |
| `m_iClip1` | `Weapon + 0x16D8` | Cliente (parcial) |
| `m_pReserveAmmo` | `Weapon + 0x16E0` | Servidor |
| `m_iAccount` | `MoneyServices + 0x40` | Servidor |
| `m_iFOV` | `CameraServices + 0x290` | Cliente |
| `m_flFlashDuration` | `Pawn + 0x1400` | Cliente |

## Pré-requisitos

- Windows 10/11 (x64)
- Visual Studio 2022 com workload "Desktop development with C++"
- Counter-Strike 2 em execução com o parâmetro `-insecure`

## Compilação e execução

1. Abra `UserMode.sln` no Visual Studio
2. Configure para `Release | x64`
3. Compile (`Ctrl+B`)
4. Inicie o CS2 com `-insecure` e entre em uma partida local com bots
5. Execute `UserMode.exe`
6. Pressione `P` para ativar/desativar as trapaças
7. Pressione `INSERT` para encerrar

## Atualização de offsets

Os offsets mudam a cada atualização do CS2. Consulte o projeto [cs2-dumper](https://github.com/a2x/cs2-dumper) para obter os valores atualizados e substitua as variáveis globais no início de `UserMode.cpp`.

## Aviso

Este projeto foi desenvolvido exclusivamente para fins acadêmicos e de pesquisa em segurança ofensiva. O uso em partidas online viola os termos de serviço do jogo e pode resultar em banimento permanente.
