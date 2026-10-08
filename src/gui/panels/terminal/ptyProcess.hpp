#pragma once

/**
 * @file ptyProcess.hpp
 * @brief Runs a child process inside a pseudo-terminal (PTY).
 *
 * Encapsulates platform differences:
 *  - Linux/macOS: forkpty() + reader thread on the master fd.
 *  - Windows: ConPTY (CreatePseudoConsole) + pipes.
 *
 * The API is asynchronous: PTY data arrives through a callback on a
 * separate thread. The caller is responsible for serializing UI access
 * (for example, via wxTheApp->CallAfter).
 *
 * The object is not copyable and is not thread-safe for concurrent
 * Start/Stop and Write. Use an external mutex if you need to write from
 * multiple threads.
 */

#include <atomic>
#include <cstddef>
#include <functional>
#include <string>
#include <thread>
#include <vector>

class PtyProcess {
public:
    /** @brief Process startup parameters. */
    struct Options {
        std::string program;             /**< Executable (empty = default shell). */
        std::vector<std::string> args;   /**< Arguments (without argv[0]). */
        std::string cwd;                 /**< Initial working directory. */
        int cols = 80;                   /**< PTY columns. */
        int rows = 24;                   /**< PTY rows. */
    };

    /** @brief Callback invoked on the reader thread when PTY data arrives. */
    using DataCallback = std::function<void(const char*, size_t)>;

    /** @brief Callback invoked (via wx CallAfter) when the process exits. */
    using ExitCallback = std::function<void(int exitCode)>;

    PtyProcess();
    ~PtyProcess();

    PtyProcess(const PtyProcess&) = delete;
    PtyProcess& operator=(const PtyProcess&) = delete;

    /**
     * @brief Starts the process inside a PTY.
     * @param opts Execution options.
     * @param onData Data callback (called on the reader thread).
     * @param onExit Exit callback (called on the waiter thread).
     * @return true on success.
     */
    bool Start(const Options& opts, DataCallback onData, ExitCallback onExit);

    /**
     * @brief Stops the process and joins the threads.
     *
     * Idempotent: calling it multiple times is harmless.
     */
    void Stop();

    /**
     * @brief Writes bytes to the PTY (program input).
     * @return Number of bytes written, or -1 on error.
     */
    ssize_t Write(const std::string& data);

    /**
     * @brief Adjusts the PTY size (sends SIGWINCH to the process).
     */
    void Resize(int cols, int rows);

    /** @brief Returns true if the process is running. */
    bool IsRunning() const { return m_running.load(); }

    /** @brief PID of the child process (0 if not started). */
    long Pid() const { return m_pid; }

private:
    void ReaderLoop();
    void WaiterLoop();

    DataCallback m_onData;
    ExitCallback m_onExit;

    std::atomic<bool> m_running{false};
    std::atomic<bool> m_stopRequested{false};

    std::thread m_reader;
    std::thread m_waiter;

    long m_pid = 0;

    int m_cols = 80;
    int m_rows = 24;

#ifdef _WIN32
    void* m_hpc = nullptr;
    void* m_hPipeIn = nullptr;
    void* m_hPipeOut = nullptr;
    void* m_hProcess = nullptr;
    unsigned long m_processId = 0;
#else
    int m_masterFd = -1;
    int m_childPid = 0;
#endif
};