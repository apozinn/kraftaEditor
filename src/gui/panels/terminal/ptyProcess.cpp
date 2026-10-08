#include "ptyProcess.hpp"

#include <cerrno>
#include <cstring>

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <mutex>

namespace {

std::mutex g_conptyMutex;

}  // namespace

PtyProcess::PtyProcess() = default;

PtyProcess::~PtyProcess() { Stop(); }

bool PtyProcess::Start(const Options& opts, DataCallback onData, ExitCallback onExit) {
    if (m_running.load())
        return false;

    m_onData = std::move(onData);
    m_onExit = std::move(onExit);
    m_cols = opts.cols > 0 ? opts.cols : 80;
    m_rows = opts.rows > 0 ? opts.rows : 24;

    HANDLE inRead = nullptr, inWrite = nullptr;
    HANDLE outRead = nullptr, outWrite = nullptr;
    if (!CreatePipe(&inRead, &inWrite, nullptr, 0) ||
        !CreatePipe(&outRead, &outWrite, nullptr, 0)) {
        return false;
    }

    COORD size{(SHORT)m_cols, (SHORT)m_rows};
    HPCON hpc = nullptr;
    HRESULT hr = CreatePseudoConsole(size, inRead, outWrite, 0, &hpc);
    if (FAILED(hr)) {
        CloseHandle(inRead);
        CloseHandle(inWrite);
        CloseHandle(outRead);
        CloseHandle(outWrite);
        return false;
    }

    CloseHandle(inRead);
    CloseHandle(outWrite);

    STARTUPINFOEXW si{};
    si.StartupInfo.cb = sizeof(si);
    si.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
    si.StartupInfo.hStdInput = nullptr;
    si.StartupInfo.hStdOutput = nullptr;
    si.StartupInfo.hStdError = nullptr;

    SIZE_T attrSize = 0;
    InitializeProcThreadAttributeList(nullptr, 1, 0, &attrSize);
    si.lpAttributeList = (LPPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(
        GetProcessHeap(), 0, attrSize);
    if (!si.lpAttributeList) {
        ClosePseudoConsole(hpc);
        CloseHandle(inWrite);
        CloseHandle(outRead);
        return false;
    }
    if (!InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &attrSize) ||
        !UpdateProcThreadAttribute(si.lpAttributeList, 0,
                                   PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE, hpc,
                                   sizeof(hpc), nullptr, nullptr)) {
        DeleteProcThreadAttributeList(si.lpAttributeList);
        HeapFree(GetProcessHeap(), 0, si.lpAttributeList);
        ClosePseudoConsole(hpc);
        CloseHandle(inWrite);
        CloseHandle(outRead);
        return false;
    }

    std::wstring cmdline;
    if (opts.program.empty()) {
        cmdline = L"cmd.exe";
    } else {
        cmdline.assign(opts.program.begin(), opts.program.end());
    }
    for (const auto& a : opts.args) {
        cmdline += L' ';
        cmdline.append(a.begin(), a.end());
    }

    std::wstring cwdW;
    if (!opts.cwd.empty())
        cwdW.assign(opts.cwd.begin(), opts.cwd.end());

    PROCESS_INFORMATION pi{};
    BOOL ok = CreateProcessW(
        nullptr, cmdline.data(), nullptr, nullptr, FALSE,
        EXTENDED_STARTUPINFO_PRESENT | CREATE_UNICODE_ENVIRONMENT, nullptr,
        cwdW.empty() ? nullptr : cwdW.c_str(), &si.StartupInfo, &pi);

    DeleteProcThreadAttributeList(si.lpAttributeList);
    HeapFree(GetProcessHeap(), 0, si.lpAttributeList);

    if (!ok) {
        ClosePseudoConsole(hpc);
        CloseHandle(inWrite);
        CloseHandle(outRead);
        return false;
    }

    CloseHandle(pi.hThread);

    m_hpc = hpc;
    m_hPipeIn = inWrite;
    m_hPipeOut = outRead;
    m_hProcess = pi.hProcess;
    m_processId = pi.dwProcessId;
    m_pid = (long)pi.dwProcessId;
    m_running.store(true);
    m_stopRequested.store(false);

    m_reader = std::thread(&PtyProcess::ReaderLoop, this);
    m_waiter = std::thread(&PtyProcess::WaiterLoop, this);
    return true;
}

void PtyProcess::ReaderLoop() {
    char buf[8192];
    DWORD n = 0;
    while (!m_stopRequested.load()) {
        BOOL ok = ReadFile((HANDLE)m_hPipeOut, buf, sizeof(buf), &n, nullptr);
        if (!ok || n == 0)
            break;
        if (m_onData)
            m_onData(buf, (size_t)n);
    }
    m_running.store(false);
}

void PtyProcess::WaiterLoop() {
    WaitForSingleObject((HANDLE)m_hProcess, INFINITE);
    DWORD code = 0;
    GetExitCodeProcess((HANDLE)m_hProcess, &code);
    m_running.store(false);
    if (m_onExit)
        m_onExit((int)code);
}

void PtyProcess::Stop() {
    if (!m_running.load() && !m_hProcess && !m_hpc)
        return;
    m_stopRequested.store(true);

    if (m_hProcess)
        TerminateProcess((HANDLE)m_hProcess, 0);
    if (m_hpc) {
        ClosePseudoConsole((HPCON)m_hpc);
        m_hpc = nullptr;
    }
    if (m_hPipeIn) {
        CloseHandle((HANDLE)m_hPipeIn);
        m_hPipeIn = nullptr;
    }
    if (m_hPipeOut) {
        CloseHandle((HANDLE)m_hPipeOut);
        m_hPipeOut = nullptr;
    }

    if (m_reader.joinable())
        m_reader.join();
    if (m_waiter.joinable())
        m_waiter.join();

    if (m_hProcess) {
        CloseHandle((HANDLE)m_hProcess);
        m_hProcess = nullptr;
    }
    m_running.store(false);
    m_pid = 0;
}

ssize_t PtyProcess::Write(const std::string& data) {
    if (!m_running.load() || !m_hPipeIn)
        return -1;
    DWORD written = 0;
    if (!WriteFile((HANDLE)m_hPipeIn, data.data(), (DWORD)data.size(),
                   &written, nullptr))
        return -1;
    return (ssize_t)written;
}

void PtyProcess::Resize(int cols, int rows) {
    if (!m_hpc)
        return;
    COORD size{(SHORT)cols, (SHORT)rows};
    ResizePseudoConsole((HPCON)m_hpc, size);
    m_cols = cols;
    m_rows = rows;
}

#else

#include <fcntl.h>
#include <pty.h>
#include <signal.h>
#include <spawn.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include <cstdlib>

extern char** environ;

PtyProcess::PtyProcess() = default;

PtyProcess::~PtyProcess() { Stop(); }

bool PtyProcess::Start(const Options& opts, DataCallback onData, ExitCallback onExit) {
    if (m_running.load())
        return false;

    m_onData = std::move(onData);
    m_onExit = std::move(onExit);
    m_cols = opts.cols > 0 ? opts.cols : 80;
    m_rows = opts.rows > 0 ? opts.rows : 24;

    struct winsize ws{};
    ws.ws_col = (unsigned short)m_cols;
    ws.ws_row = (unsigned short)m_rows;

    int master = -1;
    pid_t pid = forkpty(&master, nullptr, nullptr, &ws);
    if (pid < 0)
        return false;

    if (pid == 0) {
        if (!opts.cwd.empty())
            chdir(opts.cwd.c_str());

        std::string prog = opts.program;
        if (prog.empty()) {
            const char* shell = getenv("SHELL");
            prog = shell ? shell : "/bin/sh";
        }

        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(prog.c_str()));
        for (const auto& a : opts.args)
            argv.push_back(const_cast<char*>(a.c_str()));
        argv.push_back(nullptr);

        execvp(prog.c_str(), argv.data());
        _exit(127);
    }

    m_masterFd = master;
    m_childPid = pid;
    m_pid = pid;
    m_running.store(true);
    m_stopRequested.store(false);

    int flags = fcntl(m_masterFd, F_GETFL, 0);
    fcntl(m_masterFd, F_SETFL, flags | O_NONBLOCK);

    m_reader = std::thread(&PtyProcess::ReaderLoop, this);
    m_waiter = std::thread(&PtyProcess::WaiterLoop, this);
    return true;
}

void PtyProcess::ReaderLoop() {
    char buf[8192];
    while (!m_stopRequested.load()) {
        ssize_t n = read(m_masterFd, buf, sizeof(buf));
        if (n > 0) {
            if (m_onData)
                m_onData(buf, (size_t)n);
        } else if (n == 0) {
            break;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(2000);
                continue;
            }
            if (errno == EINTR)
                continue;
            break;
        }
    }
    m_running.store(false);
}

void PtyProcess::WaiterLoop() {
    int status = 0;
    while (waitpid(m_childPid, &status, 0) < 0 && errno == EINTR) {
    }
    int code = WIFEXITED(status) ? WEXITSTATUS(status)
                                 : (WIFSIGNALED(status) ? 128 + WTERMSIG(status) : -1);
    m_running.store(false);
    if (m_onExit)
        m_onExit(code);
}

void PtyProcess::Stop() {
    if (m_masterFd < 0 && m_childPid == 0)
        return;
    m_stopRequested.store(true);

    if (m_childPid > 0) {
        kill(m_childPid, SIGHUP);
        for (int i = 0; i < 20; ++i) {
            int status = 0;
            pid_t r = waitpid(m_childPid, &status, WNOHANG);
            if (r == m_childPid)
                break;
            usleep(5000);
        }
        kill(m_childPid, SIGKILL);
        int status = 0;
        waitpid(m_childPid, &status, 0);
        m_childPid = 0;
    }

    if (m_masterFd >= 0) {
        close(m_masterFd);
        m_masterFd = -1;
    }

    if (m_reader.joinable())
        m_reader.join();
    if (m_waiter.joinable())
        m_waiter.join();

    m_running.store(false);
    m_pid = 0;
}

ssize_t PtyProcess::Write(const std::string& data) {
    if (!m_running.load() || m_masterFd < 0)
        return -1;
    size_t written = 0;
    while (written < data.size()) {
        ssize_t n = write(m_masterFd, data.data() + written, data.size() - written);
        if (n > 0) {
            written += (size_t)n;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(1000);
                continue;
            }
            if (errno == EINTR)
                continue;
            return -1;
        }
    }
    return (ssize_t)written;
}

void PtyProcess::Resize(int cols, int rows) {
    if (m_masterFd < 0)
        return;
    struct winsize ws{};
    ws.ws_col = (unsigned short)cols;
    ws.ws_row = (unsigned short)rows;
    ioctl(m_masterFd, TIOCSWINSZ, &ws);
    m_cols = cols;
    m_rows = rows;
}

#endif  // _WIN32