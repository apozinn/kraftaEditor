/**
 * @file lspClient.hpp
 * @brief Cross-platform Language Server Protocol (LSP) client implementation using wxWidgets.
 * 
 * This module provides a complete LSP client that communicates with language servers
 * via standard input/output streams, supporting various LSP features such as
 * completion, hover, diagnostics, and document synchronization.
 * 
 * @author Apozin
 * @date 2026
 */

#pragma once

#include <atomic>
#include <functional>
#include <map>
#include <string>
#include <vector>
#include <wx/string.h>
#include <wx/thread.h>

// Forward declarations
class wxProcess;      /**< Forward declaration for wxWidgets process class */
class wxInputStream;  /**< Forward declaration for wxWidgets input stream class */

/**
 * @class LspClient
 * @brief Main client class for communicating with Language Server Protocol servers.
 * 
 * LspClient manages the lifecycle of an LSP server process and provides an
 * asynchronous interface for sending requests and receiving responses/notifications.
 * It uses wxWidgets for cross-platform process management and threading.
 * 
 * The client handles:
 * - Server process spawning and termination
 * - JSON-RPC message framing according to LSP specification
 * - Asynchronous request/response handling
 * - Notification dispatching
 * - Document synchronization (open, change, close, save)
 * 
 * @note All public methods are thread-safe and can be called from any thread.
 *       Callbacks are always executed on the main thread via wxEvtHandler::CallAfter.
 * 
 * @warning This client implements LSP protocol version 3.0.
 *          The server process is automatically terminated on destruction if still running.
 * 
 * @code
 * LspClient client;
 * client.Start("/usr/bin/clangd", "--log=error");
 * client.Initialize("file:///workspace", []() {
 *     // Server is ready
 * });
 * client.DidOpen("file:///workspace/main.cpp", "cpp", fileContent);
 * client.RequestCompletion("file:///workspace/main.cpp", 10, 5, [](const std::string& json) {
 *     // Handle completion results
 * });
 * @endcode
 */
class LspClient
{
public:
    /**
     * @brief Type alias for response callbacks.
     * 
     * Callback function receiving the raw JSON response string from the server.
     * The callback is always invoked on the main application thread.
     * 
     * @param response The raw JSON response from the LSP server
     */
    using LspResponseCallback = std::function<void(const std::string& response)>;

    /**
     * @brief Type alias for notification callbacks.
     * 
     * Callback function for handling server notifications (messages without an ID).
     * The callback is always invoked on the main application thread.
     * 
     * @param method The LSP method name of the notification
     * @param params The JSON parameters of the notification
     */
    using LspNotificationCallback = std::function<void(const std::string& method, 
                                                         const std::string& params)>;

    /**
     * @brief Default constructor.
     * 
     * Creates an uninitialized LSP client. Call Start() to launch the server.
     */
    LspClient();

    /**
     * @brief Destructor.
     * 
     * Automatically calls Stop() to cleanly shut down the LSP server process
     * and free all resources. Blocks until the server terminates or timeout.
     */
    ~LspClient();

    /**
     * @brief Starts the LSP server process.
     * 
     * Launches the specified language server executable with optional arguments.
     * Creates pipes for stdin/stdout communication and starts a reader thread
     * for processing server responses.
     * 
     * @param serverPath Full path to the language server executable
     * @param extraArgs Additional command-line arguments for the server (optional)
     * 
     * @return true if the server was successfully launched, false on failure
     * 
     * @note The function returns after spawning the process, but the server
     *       may not be ready to accept requests yet. Use Initialize() with
     *       an onReady callback to ensure the server is initialized.
     * 
     * @warning Only one server instance can be running at a time.
     *          Returns false if a server is already running.
     * 
     * @code
     * LspClient client;
     * if (client.Start("/usr/bin/clangd", "--background-index")) {
     *     // Server started successfully
     * }
     * @endcode
     */
    bool Start(const wxString& serverPath, const wxString& extraArgs = "");

    /**
     * @brief Checks whether a completion request is currently awaiting a response.
     *
     * @return true if a completion request has been sent and no response
     *         (or timeout release) has been recorded for it yet.
     */
    bool HasPendingCompletion() const;

    /**
     * @brief Releases a completion request that has been pending too long,
     *        without terminating the LSP connection.
     *
     * If the most recent completion request has been waiting longer than
     * the given threshold, its pending callback is discarded locally and
     * HasPendingCompletion() becomes false again, allowing new completion
     * requests to be sent. The server process and connection are left
     * untouched; this only prevents a single slow or lost response from
     * permanently blocking future completions.
     *
     * @param thresholdMs Time in milliseconds to wait before releasing
     *                     the pending completion (default: 8000).
     * @return true if a stale completion was released, false if there was
     *         no pending completion or it has not exceeded the threshold.
     */
    bool ReleaseStaleCompletion(long long thresholdMs = 8000);

    /**
     * @brief Stops the LSP server process gracefully.
     * 
     * Sends 'shutdown' and 'exit' notifications to the server, waits for
     * the reader thread to finish (with timeout), and terminates the process.
     * Cleans up all pending callbacks and notification handlers.
     * 
     * @note This method blocks until the server is fully stopped or a
     *       3-second timeout is reached, after which SIGTERM/SIGKILL is sent.
     * 
     * @warning All pending callbacks will be cleared without being called.
     *          After calling Stop(), the client must be restarted with Start()
     *          before it can be used again.
     */
    void Stop();

    /**
     * @brief Checks if the LSP server is currently running.
     * 
     * @return true if the server process is active, false otherwise
     */
    bool IsRunning() const { return m_running.load(); }

    /**
     * @brief Initializes the LSP server with workspace information.
     * 
     * Sends the 'initialize' request according to LSP specification with
     * the provided root URI and client capabilities. After receiving a
     * successful response, automatically sends the 'initialized' notification.
     * 
     * @param rootUri The root URI of the workspace (e.g., "file:///home/user/project")
     * @param onReady Optional callback invoked when initialization is complete.
     *                Called even if initialization fails.
     * 
     * @note The onReady callback is guaranteed to be called exactly once.
     *       Check m_initialized flag or use other requests' callback to verify success.
     * 
     * @code
     * client.Initialize("file:///workspace", []() {
     *     wxLogMessage("Server initialized and ready");
     * });
     * @endcode
     */
    void Initialize(const wxString& rootUri, std::function<void()> onReady = nullptr);

    /**
     * @brief Notifies the server that a document was opened.
     * 
     * Sends 'textDocument/didOpen' notification with the document's URI,
     * language identifier, and full text content. Required before any other
     * document-specific operations can be performed.
     * 
     * @param fileUri The URI of the opened document (e.g., "file:///workspace/main.cpp")
     * @param languageId The language identifier (e.g., "cpp", "python", "javascript")
     * @param content The complete text content of the document
     * 
     * @note This is a notification (no response expected). The server must be
     *       initialized before calling this method.
     */
    void DidOpen(const wxString& fileUri, const wxString& languageId, 
                 const wxString& content);

    /**
     * @brief Notifies the server about document changes.
     * 
     * Sends 'textDocument/didChange' notification with the new content
     * and a version identifier for synchronization.
     * 
     * @param fileUri The URI of the changed document
     * @param newContent The complete new text content of the document
     * @param version Monotonically increasing version number for the document
     * 
     * @note Sends the full document content (not just the diff) for simplicity.
     *       The version number should increment with each change.
     */
    void DidChange(const wxString& fileUri, const wxString& newContent, int version);

    /**
     * @brief Notifies the server that a document was closed.
     * 
     * Sends 'textDocument/didClose' notification. The server should release
     * resources associated with this document.
     * 
     * @param fileUri The URI of the closed document
     */
    void DidClose(const wxString& fileUri);

    /**
     * @brief Notifies the server that a document was saved.
     * 
     * Sends 'textDocument/didSave' notification. May trigger server-side
     * operations like linting or formatting.
     * 
     * @param fileUri The URI of the saved document
     */
    void DidSave(const wxString& fileUri);

    /**
     * @brief Handles loss of connection to the LSP server process.
     *
     * Called when the reader thread detects that the server's output
     * stream has reached EOF (the process closed its pipe or exited).
     * Marks the client as not running/initialized, clears all pending
     * request callbacks, and invokes the connection-lost handler set
     * via SetOnConnectionLost(), if any.
     *
     * @note This method is invoked on the main thread via CallAfter()
     *       and should not be called directly by user code.
     */
    void OnConnectionLost();

    /**
     * @brief Registers a callback invoked when the connection to the
     *        LSP server is lost.
     *
     * Typical usage is to restart the LSP client from this callback,
     * since after a connection loss the client must be started again
     * with Start() before it can be used.
     *
     * @param cb Callback invoked with no arguments when OnConnectionLost()
     *           runs.
     */
    void SetOnConnectionLost(std::function<void()> cb) { m_onConnectionLost = cb; }

    /**
     * @brief Requests document symbols (outline/structure) from the server.
     * 
     * Sends 'textDocument/documentSymbol' request to retrieve the document's
     * symbol hierarchy (classes, functions, variables, etc.).
     * 
     * @param fileUri The URI of the document to analyze
     * @param cb Callback receiving the JSON response with symbol information
     * 
     * @note Returns "{}" immediately if the server is not running or initialized.
     */
    void RequestDocumentSymbols(const wxString& fileUri, LspResponseCallback cb);

    /**
     * @brief Requests code completion suggestions at a specific position.
     * 
     * Sends 'textDocument/completion' request with the cursor position.
     * The server returns completion items based on the current context.
     * 
     * @param fileUri The URI of the document
     * @param line Zero-based line number
     * @param col Zero-based character offset in the line
     * @param cb Callback receiving the JSON response with completion items
     * 
     * @code
     * client.RequestCompletion("file:///main.cpp", 10, 5, 
     *     [](const std::string& result) {
     *         // Parse and display completion items
     *         auto json = nlohmann::json::parse(result);
     *         for (auto& item : json["result"]["items"]) {
     *             // Process each completion item
     *         }
     *     });
     * @endcode
     */
    void RequestCompletion(const wxString& fileUri, int line, int col, 
                           LspResponseCallback cb);

    /**
     * @brief Requests hover information at a specific position.
     * 
     * Sends 'textDocument/hover' request to get type information,
     * documentation, or other details about the symbol at the cursor.
     * 
     * @param fileUri The URI of the document
     * @param line Zero-based line number
     * @param col Zero-based character offset in the line
     * @param cb Callback receiving the JSON response with hover information
     */
    void RequestHover(const wxString& fileUri, int line, int col, 
                      LspResponseCallback cb);

    /**
     * @brief Requests the definition location of a symbol.
     * 
     * Sends 'textDocument/definition' request to find where a symbol
     * is defined (go-to-definition functionality).
     * 
     * @param fileUri The URI of the document
     * @param line Zero-based line number
     * @param col Zero-based character offset in the line
     * @param cb Callback receiving the JSON response with definition location(s)
     */
    void RequestDefinition(const wxString& fileUri, int line, int col, 
                           LspResponseCallback cb);

    /**
     * @brief Requests all references to a symbol.
     * 
     * Sends 'textDocument/references' request to find all usages of
     * the symbol at the specified position.
     * 
     * @param fileUri The URI of the document
     * @param line Zero-based line number
     * @param col Zero-based character offset in the line
     * @param cb Callback receiving the JSON response with reference locations
     * 
     * @note The request includes the declaration by default.
     */
    void RequestReferences(const wxString& fileUri, int line, int col, 
                           LspResponseCallback cb);

    /**
     * @brief Requests document formatting.
     * 
     * Sends 'textDocument/formatting' request to format the entire document
     * according to the server's formatting rules.
     * 
     * @param fileUri The URI of the document to format
     * @param cb Callback receiving the JSON response with text edits
     * 
     * @note Formatting options: 4 spaces per tab, using spaces (not tabs).
     */
    void RequestFormatting(const wxString& fileUri, LspResponseCallback cb);

    /**
     * @brief Requests symbol renaming across the workspace.
     * 
     * Sends 'textDocument/rename' request to rename all occurrences
     * of the symbol at the specified position.
     * 
     * @param fileUri The URI of the document
     * @param line Zero-based line number
     * @param col Zero-based character offset in the line
     * @param newName The new name for the symbol
     * @param cb Callback receiving the JSON response with workspace edits
     */
    void RequestRename(const wxString& fileUri, int line, int col, 
                       const wxString& newName, LspResponseCallback cb);

    /**
     * @brief Registers a handler for server notifications.
     * 
     * Adds a callback that will be invoked whenever the server sends
     * a notification with the specified method name.
     * 
     * @param method The LSP method name to listen for (e.g., "textDocument/publishDiagnostics")
     * @param cb Callback invoked with the method name and parameters
     * 
     * @note Only one handler per method is supported. Registering a new
     *       handler for the same method will replace the previous one.
     * 
     * @code
     * client.OnNotification("textDocument/publishDiagnostics",
     *     [](const std::string& method, const std::string& params) {
     *         wxLogMessage("Received diagnostics: %s", params);
     *     });
     * @endcode
     */
    void OnNotification(const wxString& method, LspNotificationCallback cb);

    /**
     * @brief Removes a previously registered notification handler.
     * 
     * @param method The LSP method name to stop listening for
     */
    void RemoveNotificationHandler(const wxString& method);

    /**
     * @brief Registers a callback for when diagnostics are ready.
     * 
     * The callback is invoked when the server sends the first
     * 'textDocument/publishDiagnostics' notification for the specified URI.
     * This is useful for waiting until initial analysis is complete.
     * 
     * @param fileUri The document URI to wait for diagnostics
     * @param cb Callback invoked when diagnostics become available
     * 
     * @note Multiple callbacks can be registered for the same URI.
     *       All are called when diagnostics arrive.
     */
    void OnDiagnosticsReady(const wxString& fileUri, std::function<void()> cb);

    /**
     * @brief Internal method called when data is received from the server.
     * 
     * Parses the JSON response/notification and dispatches to appropriate
     * handlers: pending request callbacks, notification handlers, or
     * diagnostic ready callbacks.
     * 
     * @param json The raw JSON string received from the server
     * 
     * @note This method is called from the main thread via CallAfter().
     *       It should not be called directly by user code.
     */
    void OnDataReceived(const std::string& json);

private:
    /**
     * @brief Sends raw data to the server's stdin.
     * 
     * Writes the message directly to the server process input stream.
     * Handles partial writes and stream errors.
     * 
     * @param msg The raw message string to send
     * 
     * @note If the write fails, the server is marked as not running.
     */
    void SendRaw(const std::string& msg);

    /**
     * @brief Frames a JSON message according to LSP specification.
     * 
     * Prepends HTTP-style headers (Content-Length) required by the
     * Language Server Protocol for message framing.
     * 
     * @param json The JSON message body
     * @return The complete framed message with headers
     * 
     * @see https://microsoft.github.io/language-server-protocol/specifications/specification-current/#headerPart
     */
    std::string Frame(const std::string& json);

    /**
     * @brief Generates the next unique request ID.
     * 
     * Uses an atomic counter to generate monotonically increasing
     * integer IDs for JSON-RPC requests.
     * 
     * @return The next request ID (starting from 1)
     */
    int NextId();

    /**
     * @brief Escapes special characters in a string for JSON.
     * 
     * Handles escaping of quotes, backslashes, newlines, carriage returns,
     * and tabs. Does not use external JSON libraries.
     * 
     * @param s The raw string to escape
     * @return The JSON-safe escaped string
     * 
     * @note This is a simplified implementation. For full JSON compliance,
     *       consider using a dedicated JSON library.
     */
    std::string EscapeJson(const std::string& s);

    /**
     * @brief Extracts a string value for a given key from JSON.
     * 
     * Simple JSON parser that finds a key and returns its string value.
     * Handles escaped characters within the string.
     * 
     * @param json The JSON string to search in
     * @param key The key to look for (including quotes and colon, e.g., "\"method\":")
     * @return The extracted string value, or empty string if not found
     */
    std::string ExtractString(const std::string& json, const std::string& key);

    /**
     * @brief Sends the 'initialized' notification to the server.
     * 
     * Part of the LSP initialization handshake. Called automatically
     * after a successful initialize response.
     */
    void SendInitialized();


    /** @brief Atomic flag indicating if the server is running */
    std::atomic<bool> m_running{false};
    
    /** @brief Atomic flag indicating if the server has been successfully initialized */
    std::atomic<bool> m_initialized{false};
    
    /** @brief wxProcess object managing the server process lifecycle */
    wxProcess* m_process = nullptr;
    
    /** @brief Reader thread for processing server output asynchronously */
    wxThread* m_readerThread = nullptr;
    
    /** @brief Process ID of the running server (0 if not running) */
    long m_pid = 0;

    /** @brief Map of pending request callbacks, keyed by request ID */
    std::map<int, LspResponseCallback> m_pending;
    
    /** @brief Map of diagnostic ready callbacks, keyed by document URI */
    std::map<std::string, std::vector<std::function<void()>>> m_onDiagnosticsReady;
    
    /** @brief Map of notification handlers, keyed by LSP method name */
    std::map<std::string, LspNotificationCallback> m_notificationHandlers;

    /** @brief Critical section protecting m_pending access */
    wxCriticalSection m_pendingLock;
    
    /** @brief Critical section protecting m_onDiagnosticsReady access */
    wxCriticalSection m_readyLock;
    
    /** @brief Critical section protecting m_notificationHandlers access */
    wxCriticalSection m_notificationLock;
    
    std::atomic<int> m_lastCompletionId{-1};

    /** @brief Callback invoked when the connection to the server is lost */
    std::function<void()> m_onConnectionLost;

    /** @brief Timestamp (ms) of the last completion request sent, or 0 if none is pending */
    std::atomic<long long> m_lastCompletionSentAt{0};
};