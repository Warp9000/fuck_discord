#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <string.h>
#include <psapi.h>
#include <time.h>

const char *priorityToString(DWORD p)
{
    switch (p)
    {
    case NORMAL_PRIORITY_CLASS:
        return "Normal";
    case IDLE_PRIORITY_CLASS:
        return "Idle";
    case HIGH_PRIORITY_CLASS:
        return "High";
    case REALTIME_PRIORITY_CLASS:
        return "Realtime";
    case BELOW_NORMAL_PRIORITY_CLASS:
        return "Below normal";
    case ABOVE_NORMAL_PRIORITY_CLASS:
        return "Above normal";

    default:
        return "unknown";
    }
}

const DWORD AboveNormalPriorities = ABOVE_NORMAL_PRIORITY_CLASS | HIGH_PRIORITY_CLASS | REALTIME_PRIORITY_CLASS;

int main()
{
    for (;;)
    {
        // Get the process ID of all processes
        DWORD processes[1024], cbNeeded, cProcesses;
        if (!EnumProcesses(processes, sizeof(processes), &cbNeeded))
        {
            printf("Failed to enumerate processes\n");
            return 1;
        }

        // Calculate the number of processes
        cProcesses = cbNeeded / sizeof(DWORD);

        // Iterate through all processes
        for (DWORD i = 0; i < cProcesses; i++)
        {
            // Get a handle to the process
            HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processes[i]);
            if (hProcess != NULL)
            {
                // Get the process name
                char szProcessName[MAX_PATH] = "";
                HMODULE hMod;
                DWORD cbNeeded;
                if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
                {
                    GetModuleBaseNameA(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(char));
                }

                // Check if the process name matches "Discord.exe"
                if (strcmp(szProcessName, "Discord.exe") == 0)
                {
                    DWORD priority = GetPriorityClass(hProcess);
                    if (!(priority & AboveNormalPriorities))
                    {
                        CloseHandle(hProcess);
                        continue;
                    }

                    time_t t = time(NULL);
                    struct tm tm = *localtime(&t);
                    printf("[%02d:%02d:%02d] ", tm.tm_hour, tm.tm_min, tm.tm_sec);

                    // Set the process priority to normal
                    if (SetPriorityClass(hProcess, NORMAL_PRIORITY_CLASS))
                    {
                        // printf("Successfully set priority of Discord.exe (%ld) to normal\n", processes[i]);
                        printf("Discord.exe (%ld) %s -> %s\n", processes[i], priorityToString(priority), priorityToString(NORMAL_PRIORITY_CLASS));
                    }
                    else
                    {
                        printf("Failed to set priority of Discord.exe (%ld) to normal\n", processes[i]);
                    }
                }

                // Close the process handle
                CloseHandle(hProcess);
            }
        }

        // Sleep for 5 seconds
        Sleep(5000);
    }
}