#include "lspClient.hpp"
#include <algorithm>
#include <sstream>
#include <thread>
#include <wx/process.h>
#include <wx/utils.h>

static const int MAX_BUFFER_SIZE = 1024 * 1024 * 10;
static const int READ_TIMEOUT_MS = 100;

class LspReaderThread : public wxThread {
  public:
	LspReaderThread(LspClient *client, wxInputStream *stream)
		: wxThread(wxTHREAD_JOINABLE), m_client(client), m_stream(stream) {}

  protected:
	virtual ExitCode Entry() wxOVERRIDE {
		if (!m_client || !m_stream)
			return (ExitCode) nullptr;

		std::vector<char> buffer(8192);
		std::string accumulator;

		while (!TestDestroy() && m_client->IsRunning()) {
			if (!m_stream->CanRead()) {
				wxMilliSleep(READ_TIMEOUT_MS);
				continue;
			}

			m_stream->Read(buffer.data(), buffer.size());
			size_t bytesRead = m_stream->LastRead();

			if (bytesRead > 0) {
				accumulator.append(buffer.data(), bytesRead);
			} else if (m_stream->Eof()) {
				if (m_client) {
					wxTheApp->CallAfter(
						[client = m_client]() { client->OnConnectionLost(); });
				}
				break;
			} else {
				wxMilliSleep(READ_TIMEOUT_MS);
				continue;
			}

			while (true) {
				size_t headerEnd = accumulator.find("\r\n\r\n");
				if (headerEnd == std::string::npos) {
					if (accumulator.size() > (size_t)MAX_BUFFER_SIZE) {
						accumulator.clear();
					}
					break;
				}

				int contentLength = -1;
				std::string headers = accumulator.substr(0, headerEnd);
				size_t clPos = headers.find("Content-Length: ");
				if (clPos == std::string::npos)
					clPos = headers.find("content-length: ");

				if (clPos != std::string::npos) {
					std::string clStr = headers.substr(clPos + 16);
					size_t endCL = clStr.find('\r');
					if (endCL != std::string::npos)
						clStr = clStr.substr(0, endCL);
					try {
						contentLength = std::stoi(clStr);
					} catch (...) {
						contentLength = -1;
					}
				}

				if (contentLength <= 0 || contentLength > MAX_BUFFER_SIZE) {
					accumulator.erase(0, headerEnd + 4);
					continue;
				}

				size_t bodyStart = headerEnd + 4;
				if (accumulator.size() < bodyStart + (size_t)contentLength)
					break;

				std::string body = accumulator.substr(bodyStart, contentLength);
				accumulator.erase(0, bodyStart + contentLength);

				if (m_client) {
					std::string captured = std::move(body);
					wxTheApp->CallAfter(
						[client = m_client, captured = std::move(captured)]() {
							if (client && client->IsRunning())
								client->OnDataReceived(captured);
						});
				}
			}
		}
		return (ExitCode) nullptr;
	}

  private:
	LspClient *m_client;
	wxInputStream *m_stream;
};

LspClient::LspClient() = default;
LspClient::~LspClient() { Stop(); }

void LspClient::OnConnectionLost() {
	m_running.store(false);
	m_initialized.store(false);
	{
		wxCriticalSectionLocker lock(m_pendingLock);
		m_pending.clear();
	}
	if (m_onConnectionLost)
		m_onConnectionLost();
}

bool LspClient::Start(const wxString &serverPath, const wxString &extraArgs) {
	if (m_running.load())
		return false;
	if (!wxFileExists(serverPath))
		return false;

	m_process = new wxProcess(wxPROCESS_REDIRECT);
	m_process->Redirect();

	wxString command = serverPath;
	if (!extraArgs.IsEmpty())
		command += " " + extraArgs;

	m_pid = wxExecute(command, wxEXEC_ASYNC, m_process);
	if (m_pid == 0) {
		delete m_process;
		m_process = nullptr;
		return false;
	}

	m_running.store(true);
	wxMilliSleep(200);

	wxInputStream *stream = m_process->GetInputStream();
	if (!stream) {
		m_running.store(false);
		delete m_process;
		m_process = nullptr;
		return false;
	}

	m_readerThread = new LspReaderThread(this, stream);
	if (m_readerThread->Create() != wxTHREAD_NO_ERROR ||
		m_readerThread->Run() != wxTHREAD_NO_ERROR) {
		delete m_readerThread;
		m_readerThread = nullptr;
		m_running.store(false);
		delete m_process;
		m_process = nullptr;
		return false;
	}

	return true;
}

void LspClient::Stop() {
	if (!m_running.load())
		return;
	m_running.store(false);

	{
		wxCriticalSectionLocker lock(m_pendingLock);
		m_pending.clear();
	}
	{
		wxCriticalSectionLocker lock(m_readyLock);
		m_onDiagnosticsReady.clear();
	}
	{
		wxCriticalSectionLocker lock(m_notificationLock);
		m_notificationHandlers.clear();
	}

	if (m_readerThread) {
		m_readerThread->Delete();
		wxStopWatch sw;
		while (m_readerThread->IsRunning() && sw.Time() < 3000)
			wxMilliSleep(10);
		if (m_readerThread->IsRunning())
			m_readerThread->Wait();
		delete m_readerThread;
		m_readerThread = nullptr;
	}

	if (m_pid > 0) {
		wxProcess::Kill(m_pid, wxSIGKILL);
		wxMilliSleep(100);
		m_pid = 0;
	}

	m_process = nullptr;
	m_initialized.store(false);
}

void LspClient::Initialize(const wxString &rootUri,
						   std::function<void()> onReady) {
	if (!m_running.load()) {
		if (onReady)
			onReady();
		return;
	}

	int id = NextId();
	std::string rootUriEscaped = EscapeJson(rootUri.ToStdString());

	std::ostringstream ss;
	ss << "{"
	   << "\"jsonrpc\":\"2.0\","
	   << "\"id\":" << id << ","
	   << "\"method\":\"initialize\","
	   << "\"params\":{"
	   << "\"processId\":" << wxGetProcessId() << ","
	   << "\"rootUri\":\"" << rootUriEscaped << "\","
	   << "\"workspaceFolders\":[{"
	   << "\"uri\":\"" << rootUriEscaped << "\","
	   << "\"name\":\"workspace\""
	   << "}],"
	   << "\"capabilities\":{"
	   << "\"textDocument\":{"
	   << "\"synchronization\":{\"didSave\":true},"
	   << "\"completion\":{"
	   << "\"completionItem\":{\"snippetSupport\":true},"
	   << "\"contextSupport\":true"
	   << "}"
	   << "}"
	   << "}"
	   << "}"
	   << "}";

	{
		wxCriticalSectionLocker lock(m_pendingLock);
		m_pending[id] = [this, onReady](const std::string &response) {
			if (response.find("\"error\"") != std::string::npos) {
				if (onReady)
					onReady();
				return;
			}
			SendInitialized();
			m_initialized.store(true);
			if (onReady)
				onReady();
		};
	}

	SendRaw(Frame(ss.str()));
}

void LspClient::SendInitialized() {
	SendRaw(Frame(R"({"jsonrpc":"2.0","method":"initialized","params":{}})"));
}

void LspClient::DidOpen(const wxString &fileUri, const wxString &languageId,
						const wxString &content) {
	if (!m_running.load() || !m_initialized.load())
		return;

	std::string uri = EscapeJson(fileUri.ToStdString());
	std::string lang = EscapeJson(languageId.ToStdString());
	std::string text = EscapeJson(std::string(content.utf8_str()));

	std::ostringstream ss;
	ss << "{"
	   << "\"jsonrpc\":\"2.0\","
	   << "\"method\":\"textDocument/didOpen\","
	   << "\"params\":{"
	   << "\"textDocument\":{"
	   << "\"uri\":\"" << uri << "\","
	   << "\"languageId\":\"" << lang << "\","
	   << "\"version\":1,"
	   << "\"text\":\"" << text << "\""
	   << "}}}";

	SendRaw(Frame(ss.str()));
}

void LspClient::DidChange(const wxString &fileUri, const wxString &newContent,
						  int version) {
	if (!m_running.load() || !m_initialized.load())
		return;

	std::string uri = EscapeJson(fileUri.ToStdString());
	std::string text = EscapeJson(std::string(newContent.utf8_str()));

	std::ostringstream ss;
	ss << "{\"jsonrpc\":\"2.0\","
	   << "\"method\":\"textDocument/didChange\","
	   << "\"params\":{"
	   << "\"textDocument\":{"
	   << "\"uri\":\"" << uri << "\","
	   << "\"version\":" << version << "},"
	   << "\"contentChanges\":[{\"text\":\"" << text << "\"}]"
	   << "}}";

	SendRaw(Frame(ss.str()));
}

void LspClient::DidClose(const wxString &fileUri) {
	if (!m_running.load() || !m_initialized.load())
		return;
	std::ostringstream ss;
	ss << R"({"jsonrpc":"2.0","method":"textDocument/didClose","params":{)"
	   << R"("textDocument":{"uri":")" << EscapeJson(fileUri.ToStdString())
	   << R"("}}})";
	SendRaw(Frame(ss.str()));
}

void LspClient::DidSave(const wxString &fileUri) {
	if (!m_running.load() || !m_initialized.load())
		return;
	std::ostringstream ss;
	ss << R"({"jsonrpc":"2.0","method":"textDocument/didSave","params":{)"
	   << R"("textDocument":{"uri":")" << EscapeJson(fileUri.ToStdString())
	   << R"("}}})";
	SendRaw(Frame(ss.str()));
}

void LspClient::RequestDocumentSymbols(const wxString &fileUri,
									   LspResponseCallback cb) {
	if (!m_running.load() || !m_initialized.load()) {
		if (cb)
			cb("{}");
		return;
	}
	int id = NextId();
	std::ostringstream ss;
	ss << R"({"jsonrpc":"2.0","id":)" << id
	   << R"(,"method":"textDocument/documentSymbol","params":{)"
	   << R"("textDocument":{"uri":")" << EscapeJson(fileUri.ToStdString())
	   << R"("}}})";
	{
		wxCriticalSectionLocker lock(m_pendingLock);
		m_pending[id] = std::move(cb);
	}
	SendRaw(Frame(ss.str()));
}

void LspClient::RequestCompletion(const wxString &fileUri, int line, int col,
								  LspResponseCallback cb) {
	if (!m_running.load() || !m_initialized.load()) {
		if (cb)
			cb("{}");
		return;
	}

	int id = NextId();
	m_lastCompletionId = id;
	m_lastCompletionSentAt.store(wxGetLocalTimeMillis().GetValue());

	std::ostringstream ss;
	ss << R"({"jsonrpc":"2.0","id":)" << id
	   << R"(,"method":"textDocument/completion","params":{)"
	   << R"("textDocument":{"uri":")" << EscapeJson(fileUri.ToStdString())
	   << R"("},)"
	   << R"("position":{"line":)" << line << R"(,"character":)" << col
	   << R"(}}})";

	LspResponseCallback wrapped =
		[this, id, cb = std::move(cb)](const std::string &json) {
			m_lastCompletionSentAt.store(0);
			if (m_lastCompletionId.load() != id)
				return;
			if (cb)
				cb(json);
		};

	{
		wxCriticalSectionLocker lock(m_pendingLock);
		m_pending[id] = std::move(wrapped);
	}

	SendRaw(Frame(ss.str()));
}

bool LspClient::HasPendingCompletion() const {
	return m_lastCompletionSentAt.load() != 0;
}

bool LspClient::ReleaseStaleCompletion(long long thresholdMs) {
	long long sentAt = m_lastCompletionSentAt.load();
	if (sentAt == 0)
		return false;

	long long elapsed = wxGetLocalTimeMillis().GetValue() - sentAt;
	if (elapsed <= thresholdMs)
		return false;

	int staleId = m_lastCompletionId.load();

	{
		wxCriticalSectionLocker lock(m_pendingLock);
		auto it = m_pending.find(staleId);
		if (it != m_pending.end())
			m_pending.erase(it);
	}

	m_lastCompletionSentAt.store(0);
	return true;
}

void LspClient::RequestHover(const wxString &fileUri, int line, int col,
							 LspResponseCallback cb) {
	if (!m_running.load() || !m_initialized.load()) {
		if (cb)
			cb("{}");
		return;
	}
	int id = NextId();
	std::ostringstream ss;
	ss << R"({"jsonrpc":"2.0","id":)" << id
	   << R"(,"method":"textDocument/hover","params":{)"
	   << R"("textDocument":{"uri":")" << EscapeJson(fileUri.ToStdString())
	   << R"("},)"
	   << R"("position":{"line":)" << line << R"(,"character":)" << col
	   << R"(}}})";
	{
		wxCriticalSectionLocker lock(m_pendingLock);
		m_pending[id] = std::move(cb);
	}
	SendRaw(Frame(ss.str()));
}

void LspClient::RequestDefinition(const wxString &fileUri, int line, int col,
								  LspResponseCallback cb) {
	if (!m_running.load() || !m_initialized.load()) {
		if (cb)
			cb("{}");
		return;
	}
	int id = NextId();
	std::ostringstream ss;
	ss << R"({"jsonrpc":"2.0","id":)" << id
	   << R"(,"method":"textDocument/definition","params":{)"
	   << R"("textDocument":{"uri":")" << EscapeJson(fileUri.ToStdString())
	   << R"("},)"
	   << R"("position":{"line":)" << line << R"(,"character":)" << col
	   << R"(}}})";
	{
		wxCriticalSectionLocker lock(m_pendingLock);
		m_pending[id] = std::move(cb);
	}
	SendRaw(Frame(ss.str()));
}

void LspClient::RequestReferences(const wxString &fileUri, int line, int col,
								  LspResponseCallback cb) {
	if (!m_running.load() || !m_initialized.load()) {
		if (cb)
			cb("{}");
		return;
	}
	int id = NextId();
	std::ostringstream ss;
	ss << R"({"jsonrpc":"2.0","id":)" << id
	   << R"(,"method":"textDocument/references","params":{)"
	   << R"("textDocument":{"uri":")" << EscapeJson(fileUri.ToStdString())
	   << R"("},)"
	   << R"("position":{"line":)" << line << R"(,"character":)" << col
	   << R"(},)"
	   << R"("context":{"includeDeclaration":true})"
	   << R"(}})";
	{
		wxCriticalSectionLocker lock(m_pendingLock);
		m_pending[id] = std::move(cb);
	}
	SendRaw(Frame(ss.str()));
}

void LspClient::RequestFormatting(const wxString &fileUri,
								  LspResponseCallback cb) {
	if (!m_running.load() || !m_initialized.load()) {
		if (cb)
			cb("{}");
		return;
	}
	int id = NextId();
	std::ostringstream ss;
	ss << R"({"jsonrpc":"2.0","id":)" << id
	   << R"(,"method":"textDocument/formatting","params":{)"
	   << R"("textDocument":{"uri":")" << EscapeJson(fileUri.ToStdString())
	   << R"("},)"
	   << R"("options":{"tabSize":4,"insertSpaces":true})"
	   << R"(}})";
	{
		wxCriticalSectionLocker lock(m_pendingLock);
		m_pending[id] = std::move(cb);
	}
	SendRaw(Frame(ss.str()));
}

void LspClient::RequestRename(const wxString &fileUri, int line, int col,
							  const wxString &newName, LspResponseCallback cb) {
	if (!m_running.load() || !m_initialized.load()) {
		if (cb)
			cb("{}");
		return;
	}
	int id = NextId();
	std::ostringstream ss;
	ss << R"({"jsonrpc":"2.0","id":)" << id
	   << R"(,"method":"textDocument/rename","params":{)"
	   << R"("textDocument":{"uri":")" << EscapeJson(fileUri.ToStdString())
	   << R"("},)"
	   << R"("position":{"line":)" << line << R"(,"character":)" << col
	   << R"(},)"
	   << R"("newName":")" << EscapeJson(newName.ToStdString()) << R"(")"
	   << R"(}})";
	{
		wxCriticalSectionLocker lock(m_pendingLock);
		m_pending[id] = std::move(cb);
	}
	SendRaw(Frame(ss.str()));
}

void LspClient::OnNotification(const wxString &method,
							   LspNotificationCallback cb) {
	wxCriticalSectionLocker lock(m_notificationLock);
	m_notificationHandlers[method.ToStdString()] = std::move(cb);
}

void LspClient::RemoveNotificationHandler(const wxString &method) {
	wxCriticalSectionLocker lock(m_notificationLock);
	auto it = m_notificationHandlers.find(method.ToStdString());
	if (it != m_notificationHandlers.end())
		m_notificationHandlers.erase(it);
}

void LspClient::OnDiagnosticsReady(const wxString &fileUri,
								   std::function<void()> cb) {
	wxCriticalSectionLocker lock(m_readyLock);
	m_onDiagnosticsReady[fileUri.ToStdString()].push_back(std::move(cb));
}

void LspClient::OnDataReceived(const std::string &json) {
	if (!m_running.load())
		return;

	int id = -1;
	bool hasId = false;
	size_t idPos = json.find("\"id\"");
	if (idPos != std::string::npos) {
		size_t colonPos = json.find(':', idPos);
		if (colonPos != std::string::npos) {
			std::string idPart = json.substr(colonPos + 1);
			while (!idPart.empty() && (idPart[0] == ' ' || idPart[0] == '\t'))
				idPart.erase(0, 1);

			if (!idPart.empty()) {
				if (idPart[0] == '"') {
					size_t endQuote = idPart.find('"', 1);
					if (endQuote != std::string::npos) {
						try {
							id = std::stoi(idPart.substr(1, endQuote - 1));
							hasId = true;
						} catch (...) {
						}
					}
				} else if (isdigit(idPart[0]) || idPart[0] == '-') {
					try {
						size_t len;
						id = std::stoi(idPart, &len);
						hasId = true;
					} catch (...) {
					}
				}
			}
		}
	}

	bool hasMethod = json.find("\"method\"") != std::string::npos;

	if (hasMethod && !hasId) {
		std::string method = ExtractString(json, "\"method\":");

		if (method == "textDocument/publishDiagnostics") {
			std::string uri = ExtractString(json, "\"uri\":");
			std::vector<std::function<void()>> callbacks;
			{
				wxCriticalSectionLocker lock(m_readyLock);
				auto it = m_onDiagnosticsReady.find(uri);
				if (it != m_onDiagnosticsReady.end()) {
					callbacks = std::move(it->second);
					m_onDiagnosticsReady.erase(it);
				}
			}
			for (auto &cb : callbacks)
				if (cb)
					cb();
		}

		{
			wxCriticalSectionLocker lock(m_notificationLock);
			auto it = m_notificationHandlers.find(method);
			if (it != m_notificationHandlers.end() && it->second) {
				std::string params;
				size_t paramsPos = json.find("\"params\":");
				if (paramsPos != std::string::npos) {
					params = json.substr(paramsPos + 9);
					if (!params.empty() && params.back() == '}')
						params.pop_back();
				}
				it->second(method, params);
			}
		}
		return;
	}

	if (!hasId || id < 0)
		return;

	LspResponseCallback cb;
	{
		wxCriticalSectionLocker lock(m_pendingLock);
		auto it = m_pending.find(id);
		if (it == m_pending.end())
			return;
		cb = std::move(it->second);
		m_pending.erase(it);
	}

	if (cb)
		cb(json);
}

void LspClient::SendRaw(const std::string &msg) {
	if (!m_process || !m_process->IsInputOpened() || !m_running.load())
		return;

	wxOutputStream *stream = m_process->GetOutputStream();
	if (!stream)
		return;

	stream->Write(msg.c_str(), msg.size());
}

std::string LspClient::Frame(const std::string &json) {
	std::ostringstream ss;
	ss << "Content-Length: " << json.size() << "\r\n";
	ss << "\r\n";
	ss << json;
	return ss.str();
}

int LspClient::NextId() {
	static std::atomic<int> g_nextId{1};
	return g_nextId.fetch_add(1);
}

std::string LspClient::EscapeJson(const std::string &s) {
	std::string result;
	result.reserve(s.size());
	for (char c : s) {
		switch (c) {
		case '"':
			result += "\\\"";
			break;
		case '\\':
			result += "\\\\";
			break;
		case '\n':
			result += "\\n";
			break;
		case '\r':
			result += "\\r";
			break;
		case '\t':
			result += "\\t";
			break;
		default:
			result += c;
		}
	}
	return result;
}

std::string LspClient::ExtractString(const std::string &json,
									 const std::string &key) {
	auto pos = json.find(key);
	if (pos == std::string::npos)
		return {};
	pos = json.find('"', pos + key.size());
	if (pos == std::string::npos)
		return {};
	++pos;
	auto end = pos;
	while (end < json.size()) {
		end = json.find('"', end);
		if (end == std::string::npos)
			return {};
		if (end > 0 && json[end - 1] == '\\') {
			++end;
			continue;
		}
		break;
	}
	if (end == std::string::npos)
		return {};
	return json.substr(pos, end - pos);
}