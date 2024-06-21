#pragma once


#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#include <windows.h>
#elif defined(__linux__)
#include <sched.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
typedef void* HANDLE;
#endif

#include <stdlib.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <sstream>

namespace tcx{

struct process
{
#ifdef _WIN32
    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};
#elif defined(__linux__)
    pid_t pid;
#endif
    bool suspended = false;
};
typedef const char* arg_t;

struct startinfo {
    char entry[1024]{ 0 };
    char cwd[1024]{ 0 };

    bool open_console = false;
    bool std_out = true;
    bool suspend = false;

};

inline char* __str_to_c_str(const std::string& src) {
    size_t size = src.size();
    char* buff = (char*)malloc(size + 1);
    if (!buff) {
        throw 0;
        return nullptr;
    }
    memcpy_s(buff, size + 1, src.data(), size);
    buff[size] = 0;
    return buff;
}

inline bool __get_file_folder_path(const std::string& file_full_path,std::string& out_path){
#ifdef _WIN32
    char slash = '\\';
#elif defined(__linux__)
    char slash = '/';
#endif
    if(file_full_path[file_full_path.size()-1] == slash) return false;
    auto pos = file_full_path.find_last_of(slash);
    if(pos == std::string::npos) return false;
    out_path = file_full_path.substr(0,pos);
    return true;
}

inline process* creaet_process_v(const startinfo& info, const std::vector<std::string>& args) {
    process* p = new process();
#ifdef _WIN32
    p->si.cb = sizeof(STARTUPINFO);
    p->si.dwFlags = info.std_out ? 0 : STARTF_USESTDHANDLES;
    DWORD create_flag = 0;
    if (info.suspend) create_flag |= CREATE_SUSPENDED;
    if (info.open_console) create_flag |= CREATE_NEW_CONSOLE;

    std::string cwd_s = "";

    if (!strlen(info.cwd)) __get_file_folder_path(info.entry, cwd_s);
    else cwd_s = info.cwd;

    std::vector<std::string> args_vec = args;

    std::stringstream ss("");
    ss << info.entry << " ";
    for (auto& i : args_vec) {
        ss << i << " ";
    }
    char* cmd_buff = __str_to_c_str(ss.str());
    char* cwd_buff = __str_to_c_str(cwd_s);

    auto ret = CreateProcessA(
        NULL,
        cmd_buff,
        NULL,
        NULL,
        TRUE,
        create_flag,
        NULL,
        strlen(cwd_buff) ? cwd_buff : NULL,
        &p->si,
        &p->pi
    );
    free(cmd_buff);
    free(cwd_buff);
    if (ret) {
        p->suspended = info.suspend;
        return p;
    }
    else {
        delete p;
        return nullptr;
    }


#elif defined(__linux__)

    p->pid = fork();
    if (p->pid < 0) {
        // error
        return p;
    }
    else if (p->pid == 0) {
        if (!strlen(info.entry)) return nullptr;
        if (strlen(info.cwd)) {
            int ret = chdir(info.cwd);
        }
        else {
            std::string out_path = "";
            file::get_file_folder_path(info.entry, out_path);
            int ret = chdir(out_path.c_str());
        }
        // char cwd[MAX_CMD_LEN]{0};
        std::string name = "";
        file::get_file_name(info.entry, name);
        // getcwd(cwd,MAX_CMD_LEN);
        // printf("cmd: %s\ncwd: %s\n",info.entry,cwd);

        const char** __argp = new const char* [args.size() + 2];
        __argp[0] = name.c_str();
        for (size_t i = 0; i < args.size(); i++) {
            __argp[i + 1] = args[i].c_str();
        }
        // std::cout << info.entry << "\n";
        __argp[args.size() + 1] = NULL;

        for (int i = 0; i < args.size() + 2; i++) {
            std::cout << __argp[i] << "\n";
        }

        execvp(info.entry, (char* const*)__argp);
        // execlp(info.entry,name.c_str(),args...,NULL);


        return nullptr;
    }
    else {
        // ok
        return p;
    }
#endif
}

inline bool command(std::string& in_out_cmd_res) {
    FILE* fp = popen(in_out_cmd_res.c_str(), "r");
    if (!fp)return false;
    in_out_cmd_res = "";

    while (!::feof(fp)) {
        uint8_t temp;
        if (::fread(&temp, 1, 1, fp)) {
            in_out_cmd_res += temp;
        }
    }
    pclose(fp);
    return true;
}

inline void stop_process(process* p) {
    if (!p) return;
#ifdef _WIN32
    TerminateProcess(p->pi.hProcess, 0);
#elif defined(__linux__)
    kill(p->pid, 0);
#endif
    delete p;
    p = nullptr;
}

inline void resume_process(process* p) {
#ifdef _WIN32
    if (p->suspended) {
        ::ResumeThread(p->pi.hThread);
    }
#elif defined(__linux__)

#endif
}

bool inject(process* p,std::string const& _dll_path) {
#ifdef _WIN32
	HANDLE hProcess = (HANDLE)p->pi.hProcess;
	if (!hProcess) return false;
	size_t dll_path_size = _dll_path.size();

	// alloc remote space for process
	LPVOID alloc_start_addr = VirtualAllocEx(
		hProcess,
		NULL,
		dll_path_size,
		MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE
	);
	if (!alloc_start_addr) return false;
	
	// copy the dll path into the remote space
	size_t sizeNumberOfBitWrote = 0;
	if (!WriteProcessMemory(
		hProcess,
		alloc_start_addr,
		_dll_path.c_str(),
		dll_path_size,
		&sizeNumberOfBitWrote)) {
		return false;
	}

	// get kernel module
	HMODULE kernel = GetModuleHandleA("kernel32.dll");
	if (!kernel) return false;

	// get LoadLibraryA func pointer
	auto pLoadLibraryA = (PTHREAD_START_ROUTINE)GetProcAddress(kernel, "LoadLibraryA");
	if (!pLoadLibraryA) return false;

	// run LoadLibraryA in remote thread
	HANDLE hRemoteThread = CreateRemoteThreadEx(
		hProcess, NULL, NULL, pLoadLibraryA, alloc_start_addr, NULL, NULL, NULL);
	if(!hRemoteThread) return false;

	//std::this_thread::sleep_for(std::chrono::seconds(10));
	WaitForSingleObject(hRemoteThread, INFINITE);
	CloseHandle(hRemoteThread);
	return true;
#elif defined(__linux__)
    return false;
#endif
}

}