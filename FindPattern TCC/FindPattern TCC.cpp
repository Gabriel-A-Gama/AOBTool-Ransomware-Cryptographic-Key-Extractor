#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>


DWORD FindPatternInProcess(HANDLE hProcess, DWORD base, DWORD size, const char* pattern, const char* mask) {
    SIZE_T patternLength = strlen(mask);
    std::vector<char> buffer(size);
    SIZE_T bytesRead;

    if (!ReadProcessMemory(hProcess, (LPCVOID)base, buffer.data(), size, &bytesRead)) {
        return 0xBEEE;
    }

    for (DWORD i = 0; i < size - patternLength; i++) {
        bool found = true;
        for (DWORD j = 0; j < patternLength; j++) {
            if (mask[j] != '?' && pattern[j] != buffer[i + j]) {
                found = false;
                break;
            }
        }
        if (found) return base + i;
    }

    return 0xBEEE;
}

DWORD GetModuleBaseAddress(DWORD pid, const std::wstring& moduleName) {
    DWORD baseAddress = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (hSnap != INVALID_HANDLE_VALUE) {
        MODULEENTRY32W me32 = { sizeof(me32) };
        if (Module32FirstW(hSnap, &me32)) {
            do {
                if (moduleName == me32.szModule) {
                    baseAddress = (DWORD)me32.modBaseAddr;
                    break;
                }
            } while (Module32NextW(hSnap, &me32));
        }
        CloseHandle(hSnap);
    }
    return baseAddress;
}

DWORD GetProcessIdByName(const std::wstring& processName) {
    DWORD pid = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe32 = { sizeof(pe32) };
        if (Process32FirstW(hSnap, &pe32)) {
            do {
                if (processName == pe32.szExeFile) {
                    pid = pe32.th32ProcessID;
                    break;
                }
            } while (Process32NextW(hSnap, &pe32));
        }
        CloseHandle(hSnap);
    }
    return pid;
}

void SuspendProcess(DWORD pid) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        THREADENTRY32 te32 = { sizeof(te32) };
        if (Thread32First(hSnap, &te32)) {
            do {
                if (te32.th32OwnerProcessID == pid) {
                    HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
                    if (hThread) {
                        SuspendThread(hThread);
                        CloseHandle(hThread);
                    }
                }
            } while (Thread32Next(hSnap, &te32));
        }
        CloseHandle(hSnap);
    }
}
void ResumeProcess(DWORD pid) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return;

    THREADENTRY32 te = { 0 };
    te.dwSize = sizeof(te);

    if (Thread32First(hSnapshot, &te)) {
        do {
            if (te.th32OwnerProcessID == pid) {
                HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
                if (hThread) {
                    ResumeThread(hThread); 
                    CloseHandle(hThread);
                }
            }
        } while (Thread32Next(hSnapshot, &te));
    }

    CloseHandle(hSnapshot);
}



int main() {
    std::wstring targetExe = L"Ransomware TCC.exe";
    std::cout << "Esperando processo: " << std::string(targetExe.begin(), targetExe.end()) << "...\n";


    auto inicio_total = std::chrono::high_resolution_clock::now();
    int tentativas = 0;

    while (true) {
        if (GetAsyncKeyState(VK_F8) & 0x8000) {
            std::cout << "Encerrando...\n";
            break;
        }

        DWORD pid = GetProcessIdByName(targetExe);
        if (pid != 0) {
            tentativas++;


            auto inicio_varredura = std::chrono::high_resolution_clock::now();

            SuspendProcess(pid);

            HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, pid);
            if (hProcess) {
                DWORD base = GetModuleBaseAddress(pid, targetExe);
                if (base) {
                    MODULEINFO modInfo = { 0 };
                    GetModuleInformation(hProcess, (HMODULE)base, &modInfo, sizeof(modInfo));
                    DWORD size = (DWORD)modInfo.SizeOfImage;

                    const char* pattern = "\xBA\x00\x00\x00\x00\x8B\xC8\xE8\x00\x00\x00\x00\xBA";
                    const char* mask = "x????xxx????x";

                    auto inicio_pattern = std::chrono::high_resolution_clock::now();
                    DWORD instr_addr = FindPatternInProcess(hProcess, base, size, pattern, mask);
                    auto fim_pattern = std::chrono::high_resolution_clock::now();

                    if (instr_addr != 0xBEEE) {
                        DWORD key_addr;
                        if (ReadProcessMemory(hProcess, (LPCVOID)(instr_addr + 1), &key_addr, sizeof(DWORD), nullptr)) {
                            char key[33] = { 0 };

                            auto inicio_leitura = std::chrono::high_resolution_clock::now();


                            if (ReadProcessMemory(hProcess, (LPCVOID)key_addr, key, 32, nullptr) &&
                                key[0] != 0 && key[1] != 0 && key[2] != 0) {

                                auto fim_leitura = std::chrono::high_resolution_clock::now();
                                auto fim_varredura = std::chrono::high_resolution_clock::now();
                                auto fim_total = std::chrono::high_resolution_clock::now();

                                auto tempo_pattern =
                                    std::chrono::duration<double, std::milli>(fim_pattern - inicio_pattern);

                                auto tempo_leitura =
                                    std::chrono::duration<double, std::milli>(fim_leitura - inicio_leitura);

                                auto tempo_total =
                                    std::chrono::duration<double, std::milli>(fim_total - inicio_total);



                                std::cout << "\n===============================\n";
                                std::cout << "CHAVE ENCONTRADA: " << key << std::endl;

                                std::ofstream ofs("C:\\Users\\Gama\\Desktop\\Dump\\chave_lida.bin", std::ios::binary);
                                ofs.write(key, 32);
                                ofs.close();

                                std::cout << "Chave salva em: C:\\Users\\Gama\\Desktop\\Dump\\chave_lida.bin\n";
                                std::cout << "METRICAS DE TEMPO:\n";
                                std::cout << "Busca do pattern: " << tempo_pattern.count() << " ms\n";
                                std::cout << "Leitura da chave: " << tempo_leitura.count() << " ms\n";   
                                std::cout << "===============================\n";

                                CloseHandle(hProcess);
                                ResumeProcess(pid);
                                break;
                            }
                        }
                    }
                    else {
                        auto fim_varredura = std::chrono::high_resolution_clock::now();
                        std::chrono::duration<double, std::milli> tempo_varredura = fim_varredura - inicio_varredura;
                        std::cout << "Varredura " << tentativas << ": Pattern não encontrado ("
                            << tempo_varredura.count() << " ms)\n";
                    }
                }
                CloseHandle(hProcess);
            }
            ResumeProcess(pid);
        }
    }

    std::cout << "Pressione qualquer tecla para sair...";
    std::cin.get();
    return 0;
}
