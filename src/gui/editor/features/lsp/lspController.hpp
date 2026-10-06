#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <wx/stc/stc.h>
#include <wx/timer.h>
#include <wx/weakref.h>
#include <wx/wx.h>

#include "lsp/lspClient/lspClient.hpp"
#include "languagesPreferences/languagesPreferences.hpp"

class TextCtrl;
class AutoCompleteController;

class LspController : public wxEvtHandler {
  public:
	static LspController *Create(TextCtrl *textCtrl,
								 languagePreferencesStruct languagePreferences);
	~LspController() override;

	void SetAutocompleteController(AutoCompleteController *controller);

	void TriggerCompletion();
	void NotifySaved();
	void Restart();
	void Stop();

  private:
	LspController(TextCtrl *textCtrl,
				  languagePreferencesStruct languagePreferences);

	enum {
		ID_SYNC_TIMER = wxID_HIGHEST + 701,
		ID_COMPLETION_TIMER,
		ID_RESTART_TIMER
	};

	bool Alive() const;
	void CreateLspClientInstance();

	void OnTextModified(wxStyledTextEvent &event);
	void OnSyncTimer(wxTimerEvent &);
	void ScheduleSync();
	void FlushSync();

	void OnCharAdd(wxStyledTextEvent &event);
	void OnCompletionTimer(wxTimerEvent &);
	void OnRestartTimer(wxTimerEvent &);
	void RequestCompletionNow(int triggerKind, const std::string &triggerChar);
	void OnCompletionResponse(uint64_t serial, int wordStart, int line,
							  const std::string &raw);
	void ShowFallback(const std::string &prefix, int curLen);

	void LspPosition(int pos, int &line, int &utf16Col) const;

	wxWeakRef<TextCtrl> m_textCtrl;
	wxWeakRef<wxStyledTextCtrl> m_stc;
	languagePreferencesStruct m_languagePreferences;
	AutoCompleteController *m_autocompleteController = nullptr;

	std::shared_ptr<LspClient> m_lsp;
	wxTimer m_syncTimer;
	wxTimer m_completionTimer;
	wxTimer m_restartTimer;

	wxString m_uri;
	wxString m_languageId;

	bool m_starting = false;
	bool m_ready = false;
	int m_version = 1;
	int m_restartCount = 0;

	std::vector<LspTextChange> m_pending;
	bool m_fullSyncPending = false;

	uint64_t m_completionSerial = 0;
	bool m_lastIncomplete = false;
	int m_trigKind = 1;
	std::string m_trigChar;
};