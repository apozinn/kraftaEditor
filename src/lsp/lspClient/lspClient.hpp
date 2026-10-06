#pragma once

#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include <wx/string.h>

class LspProcess;

using LspResponseCallback = std::function<void(const std::string &)>;
using LspNotificationCallback =
	std::function<void(const std::string &method, const std::string &params)>;

struct LspTextChange {
	int startLine = 0, startChar = 0;
	int endLine = 0, endChar = 0;
	std::string text; 
};

class LspClient : public std::enable_shared_from_this<LspClient> {
  public:
	LspClient();
	~LspClient();

	bool Start(const wxString &serverPath, const wxString &extraArgs);
	void Stop();
	bool IsRunning() const { return m_running.load(); }
	bool IsInitialized() const { return m_initialized.load(); }

	void SetOnConnectionLost(std::function<void()> cb) {
		m_onConnectionLost = std::move(cb);
	}

	void Initialize(const wxString &rootUri, std::function<void()> onReady);

	void DidOpen(const wxString &fileUri, const wxString &languageId,
				 const wxString &content, int version = 1);
	void DidChange(const wxString &fileUri, const wxString &newContent,
				   int version);
	void DidChange(const wxString &fileUri,
				   const std::vector<LspTextChange> &changes, int version);
	void DidClose(const wxString &fileUri);
	void DidSave(const wxString &fileUri);

	void RequestCompletion(const wxString &fileUri, int line, int col,
						   LspResponseCallback cb, int triggerKind = 1,
						   const std::string &triggerChar = {});
	void RequestHover(const wxString &fileUri, int line, int col,
					  LspResponseCallback cb);
	void RequestDefinition(const wxString &fileUri, int line, int col,
						   LspResponseCallback cb);
	void RequestReferences(const wxString &fileUri, int line, int col,
						   LspResponseCallback cb);
	void RequestDocumentSymbols(const wxString &fileUri, LspResponseCallback cb);
	void RequestFormatting(const wxString &fileUri, LspResponseCallback cb);
	void RequestRename(const wxString &fileUri, int line, int col,
					   const wxString &newName, LspResponseCallback cb);

	void OnNotification(const wxString &method, LspNotificationCallback cb);
	void RemoveNotificationHandler(const wxString &method);
	void OnDiagnosticsReady(const wxString &fileUri, std::function<void()> cb);

  private:
	friend class LspProcess;

	void ReaderLoop();
	void WriterLoop();
	void OnConnectionLost();
	void OnDataReceived(const std::string &raw);

	void Send(const std::string &body);
	int SendRequest(const char *method, const nlohmann::json &params,
					LspResponseCallback cb);
	void SendNotification(const char *method, const nlohmann::json &params);
	void SendInitialized();
	void RequestPositional(const char *method, const wxString &uri, int line,
						   int col, LspResponseCallback cb);
	bool NotReady(const LspResponseCallback &cb);
	void CancelRequest(int id);

	static std::string Frame(const std::string &body);
	static int NextId();

	LspProcess *m_process = nullptr;
	long m_pid = 0;
	std::thread m_reader;
	std::thread m_writer;
	std::weak_ptr<LspClient> m_self;

	std::atomic<bool> m_running{false};
	std::atomic<bool> m_initialized{false};
	bool m_stopWriter = false;

	std::mutex m_outMutex;
	std::condition_variable m_outCv;
	std::deque<std::string> m_out;

	std::mutex m_pendingMutex;
	std::unordered_map<int, LspResponseCallback> m_pending;

	std::mutex m_handlerMutex;
	std::unordered_map<std::string, LspNotificationCallback>
		m_notificationHandlers;
	std::unordered_map<std::string, std::vector<std::function<void()>>>
		m_onDiagnosticsReady;

	std::function<void()> m_onConnectionLost;
	int m_lastCompletionId = 0;
};