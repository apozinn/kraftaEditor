#include "textCtrl.hpp"

void TextCtrl::CharAdd(wxStyledTextEvent &event) {
	if (m_isDestroyed)
		return;
        
        changedFile = true;
	const char chr = static_cast<char>(event.GetKey());

	if (chr == '\n') {
		OnEnterKey(event);
	}

	HandleAutoPairing(chr);
	event.Skip();
}

void TextCtrl::OnChange(wxStyledTextEvent &event) {
	if (m_isDestroyed)
		return;
	if (!GetModify() || !changedFile) {
		event.Skip();
		return;
	}

	if (UserSettingsManager::Get().GetSetting<bool>("editor/autoSave").value &&
		GetName() != UserSettingsManager::Get().SettingsPath) {
		DoSave(GetName());
	} else {
		UpdateUnsavedIndicator();
	}

	ClearIndicators();
	event.Skip();
}

void TextCtrl::OnEnterKey(wxStyledTextEvent &event) {
	wxStyledTextCtrl *stc = this;
	const int curPos = stc->GetCurrentPos();
	const int curLine = stc->GetCurrentLine();
	const int indentSize = stc->GetIndent();

	int baseIndent = 0;
	if (curLine > 0) {
		const int prevLine = curLine - 1;
		baseIndent = stc->GetLineIndentation(prevLine);

		wxString prevText = stc->GetLine(prevLine);
		prevText.Trim(true).Trim(false);

		if (prevText.EndsWith("{")) {
			baseIndent += indentSize;
		}
	}

	if (curPos > 0 && curPos < stc->GetTextLength()) {
		const char prevChar = stc->GetCharAt(curPos - 1);
		const char nextChar = stc->GetCharAt(curPos);

		if (prevChar == '{' || nextChar == '}') {
			stc->BeginUndoAction();

			stc->AddText("\n");
			stc->SetLineIndentation(curLine, baseIndent);
			stc->SetLineIndentation(curLine + 1, baseIndent - indentSize);
			stc->GotoPos(stc->GetLineIndentPosition(curLine));

			stc->EndUndoAction();
			return;
		}
	}

	event.Skip();

	wxString curText = stc->GetLine(curLine);
	curText.Trim(true).Trim(false);

	int finalIndent = baseIndent;
	if (curText.StartsWith("}")) {
		finalIndent = std::max(0, baseIndent - indentSize);
	}

	stc->SetLineIndentation(curLine, finalIndent);
	stc->GotoPos(stc->GetLineIndentPosition(curLine));
}

void TextCtrl::DoCopy(wxCommandEvent &e) {
	TextCtrl *focusedTextCtrl = GetFocusedTextCtrl();
	if (!focusedTextCtrl)
		return;

	if (!focusedTextCtrl->CanCopy())
		return;
	focusedTextCtrl->Copy();
}

void TextCtrl::DoCut(wxCommandEvent &e) {
	TextCtrl *focusedTextCtrl = GetFocusedTextCtrl();
	if (!focusedTextCtrl)
		return;

	if (focusedTextCtrl->GetSelectionStart() ==
		focusedTextCtrl->GetSelectionEnd()) {
		int line = focusedTextCtrl->GetCurrentLine();

		if (line < focusedTextCtrl->GetLineCount() - 1) {
			int start = focusedTextCtrl->PositionFromLine(line);
			int end = focusedTextCtrl->GetLineEndPosition(line) + 1;
			focusedTextCtrl->SetSelection(start, end);
		} else {
			int start = focusedTextCtrl->PositionFromLine(line);
			int end = focusedTextCtrl->GetLineEndPosition(line);

			if (line > 0) {
				start--;
			}

			focusedTextCtrl->SetSelection(start, end);
		}
	}

	focusedTextCtrl->Cut();
}

void TextCtrl::DoPaste(wxCommandEvent &e) {
	TextCtrl *focusedTextCtrl = GetFocusedTextCtrl();
	if (!focusedTextCtrl)
		return;

	if (!focusedTextCtrl->CanPaste())
		return;
	focusedTextCtrl->Paste();
}

void TextCtrl::DoUndo(wxCommandEvent &e) {
	TextCtrl *focusedTextCtrl = GetFocusedTextCtrl();
	if (!focusedTextCtrl)
		return;

	if (!focusedTextCtrl->CanUndo())
		return;
	focusedTextCtrl->Undo();
}

void TextCtrl::DoRedo(wxCommandEvent &e) {
	TextCtrl *focusedTextCtrl = GetFocusedTextCtrl();
	if (!focusedTextCtrl)
		return;

	if (!focusedTextCtrl->CanRedo())
		return;
	focusedTextCtrl->Redo();
}

void TextCtrl::SelectLine(wxCommandEvent &e) {
	TextCtrl *focusedTextCtrl = GetFocusedTextCtrl();
	if (!focusedTextCtrl)
		return;

	int lineStart =
		focusedTextCtrl->PositionFromLine(focusedTextCtrl->GetCurrentLine());
	int lineEnd = focusedTextCtrl->PositionFromLine(
		focusedTextCtrl->GetCurrentLine() + 1);
	focusedTextCtrl->SetSelection(lineStart, lineEnd);
}

void TextCtrl::DoSelectAll(wxCommandEvent &e) {
	TextCtrl *focusedTextCtrl = GetFocusedTextCtrl();
	if (!focusedTextCtrl)
		return;

	focusedTextCtrl->SelectAll();
}

void TextCtrl::MoveCursorUp(wxCommandEvent &WXUNUSED(event)) {
	const int line = GetCurrentLine();
	const int lineStart = PositionFromLine(line);

	BeginUndoAction();

	InsertText(lineStart, "\n");

	EndUndoAction();

	if (line > 0)
		GotoLine(line);
	else
		GotoPos(0);

	SetEmptySelection(GetCurrentPos());
	EnsureCaretVisible();
}

void TextCtrl::MoveCursorDown(wxCommandEvent &WXUNUSED(event)) {
	const int line = GetCurrentLine();
	const int lineEnd = GetLineEndPosition(line);

	BeginUndoAction();

	GotoPos(lineEnd);
	InsertText(lineEnd, "\n");

	EndUndoAction();

	GotoLine(line + 1);
	SetEmptySelection(GetCurrentPos());
	EnsureCaretVisible();
}

void TextCtrl::DuplicateLine(wxCommandEvent &e) {
	TextCtrl *focusedTextCtrl = GetFocusedTextCtrl();
	if (!focusedTextCtrl)
		return;

	const int selStart = focusedTextCtrl->GetSelectionStart();
	const int selEnd = focusedTextCtrl->GetSelectionEnd();

	if (selStart == selEnd) {
		const int line = focusedTextCtrl->GetCurrentLine();
		const int lineStart = focusedTextCtrl->PositionFromLine(line);
		const int lineEnd = focusedTextCtrl->GetLineEndPosition(line);
		const wxString text = focusedTextCtrl->GetTextRange(lineStart, lineEnd);

		focusedTextCtrl->BeginUndoAction();
		focusedTextCtrl->InsertText(lineEnd, "\n" + text);
		focusedTextCtrl->EndUndoAction();

		focusedTextCtrl->GotoLine(line + 1);
		focusedTextCtrl->SetEmptySelection(focusedTextCtrl->GetCurrentPos());
	} else {
		const wxString text = focusedTextCtrl->GetTextRange(selStart, selEnd);

		focusedTextCtrl->BeginUndoAction();
		focusedTextCtrl->InsertText(selEnd, "\n" + text);
		focusedTextCtrl->EndUndoAction();

		focusedTextCtrl->SetSelection(selEnd, selEnd + text.Length());
	}

	focusedTextCtrl->EnsureCaretVisible();
}

void TextCtrl::MoveLineUp(wxCommandEvent &e) {
	TextCtrl *focusedTextCtrl = GetFocusedTextCtrl();
	if (!focusedTextCtrl)
		return;

	int selStart = focusedTextCtrl->GetSelectionStart();
	int selEnd = focusedTextCtrl->GetSelectionEnd();

	bool hasSelection = (selStart != selEnd);

	int startLine;
	int endLine;

	if (!hasSelection) {
		startLine = endLine =
			focusedTextCtrl->LineFromPosition(focusedTextCtrl->GetCurrentPos());
	} else {
		startLine = focusedTextCtrl->LineFromPosition(selStart);
		endLine = focusedTextCtrl->LineFromPosition(selEnd);

		if (selEnd == focusedTextCtrl->PositionFromLine(endLine))
			endLine--;
	}

	if (startLine <= 0)
		return;

	focusedTextCtrl->BeginUndoAction();

	wxString textAbove = focusedTextCtrl->GetLine(startLine - 1);
	wxString blockText;

	for (int i = startLine; i <= endLine; ++i)
		blockText += focusedTextCtrl->GetLine(i);

	int removeStart = focusedTextCtrl->PositionFromLine(startLine - 1);
	int removeEnd = focusedTextCtrl->PositionFromLine(endLine + 1);

	focusedTextCtrl->SetTargetStart(removeStart);
	focusedTextCtrl->SetTargetEnd(removeEnd);
	focusedTextCtrl->ReplaceTarget(blockText + textAbove);

	int newStartPos = focusedTextCtrl->PositionFromLine(startLine - 1);
	int newEndPos = newStartPos + blockText.Length();

	if (hasSelection)
		focusedTextCtrl->SetSelection(newStartPos, newEndPos);
	else
		focusedTextCtrl->GotoPos(newStartPos);

	focusedTextCtrl->EndUndoAction();
}

void TextCtrl::MoveLineDown(wxCommandEvent &e) {
	TextCtrl *focusedTextCtrl = GetFocusedTextCtrl();
	if (!focusedTextCtrl)
		return;

	int selStart = focusedTextCtrl->GetSelectionStart();
	int selEnd = focusedTextCtrl->GetSelectionEnd();

	bool hasSelection = (selStart != selEnd);

	int startLine;
	int endLine;

	if (!hasSelection) {
		startLine = endLine =
			focusedTextCtrl->LineFromPosition(focusedTextCtrl->GetCurrentPos());
	} else {
		startLine = focusedTextCtrl->LineFromPosition(selStart);
		endLine = focusedTextCtrl->LineFromPosition(selEnd);

		if (selEnd == focusedTextCtrl->PositionFromLine(endLine))
			endLine--;
	}

	if (endLine >= focusedTextCtrl->GetLineCount() - 1)
		return;

	focusedTextCtrl->BeginUndoAction();

	wxString textBelow = focusedTextCtrl->GetLine(endLine + 1);
	wxString blockText;

	for (int i = startLine; i <= endLine; ++i)
		blockText += focusedTextCtrl->GetLine(i);

	int removeStart = focusedTextCtrl->PositionFromLine(startLine);
	int removeEnd = focusedTextCtrl->PositionFromLine(endLine + 2);

	focusedTextCtrl->SetTargetStart(removeStart);
	focusedTextCtrl->SetTargetEnd(removeEnd);
	focusedTextCtrl->ReplaceTarget(textBelow + blockText);

	int newStartPos = focusedTextCtrl->PositionFromLine(startLine + 1);
	int newEndPos = newStartPos + blockText.Length();

	if (hasSelection)
		focusedTextCtrl->SetSelection(newStartPos, newEndPos);
	else
		focusedTextCtrl->GotoPos(newStartPos);

	focusedTextCtrl->EndUndoAction();
}

void TextCtrl::RemoveCurrentLine(wxCommandEvent &e) {
	TextCtrl *focusedTextCtrl = GetFocusedTextCtrl();
	if (!focusedTextCtrl)
		return;

	if (focusedTextCtrl->GetSelectionStart() !=
		focusedTextCtrl->GetSelectionEnd()) {
		focusedTextCtrl->ReplaceSelection("");
		return;
	}

	const int currentLine = focusedTextCtrl->GetCurrentLine();
	const int lineStart = focusedTextCtrl->PositionFromLine(currentLine);
	int lineEnd = focusedTextCtrl->GetLineEndPosition(currentLine);

	const int lastLine = focusedTextCtrl->GetLineCount() - 1;
	if (currentLine < lastLine) {
		lineEnd++;
	}

	focusedTextCtrl->BeginUndoAction();
	focusedTextCtrl->SetTargetStart(lineStart);
	focusedTextCtrl->SetTargetEnd(lineEnd);
	focusedTextCtrl->ReplaceTarget("");
	focusedTextCtrl->EndUndoAction();
}

void TextCtrl::ToggleLineComment(wxCommandEvent &e) {
	TextCtrl *focusedTextCtrl = GetFocusedTextCtrl();
	if (!focusedTextCtrl)
		return;

	int lineStart = 0;
	if (focusedTextCtrl->GetSelectionEnd() -
			focusedTextCtrl->GetSelectionStart() <=
		0) {
		lineStart = focusedTextCtrl->PositionFromLine(
			focusedTextCtrl->GetCurrentLine());
	} else {
		lineStart = focusedTextCtrl->GetSelectionStart();
	}

	char chr = (char)focusedTextCtrl->GetCharAt(lineStart);

	if (chr == ' ') {
		while (chr == ' ') {
			lineStart++;
			chr = (char)focusedTextCtrl->GetCharAt(lineStart);
		}
	}

	if (chr == '/' && (char)focusedTextCtrl->GetCharAt(lineStart + 1) == '/') {
		focusedTextCtrl->DeleteRange(lineStart, 2);
	} else {
		focusedTextCtrl->InsertText(lineStart, "//");
	}
}

void TextCtrl::ToggleBlockComment(wxCommandEvent &event) {
	TextCtrl *focusedTextCtrl = GetFocusedTextCtrl();
	if (!focusedTextCtrl)
		return;

	int selStart = focusedTextCtrl->GetSelectionStart();
	int selEnd = focusedTextCtrl->GetSelectionEnd();
	if (selStart > selEnd)
		std::swap(selStart, selEnd);

	if (selStart == selEnd) {
		const int line = focusedTextCtrl->GetCurrentLine();
		selStart = focusedTextCtrl->PositionFromLine(line);
		selEnd = focusedTextCtrl->GetLineEndPosition(line);
	}

	auto isSpace = [&](int ch) {
		return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
	};

	int s = selStart;
	int e = selEnd;
	while (s < e && isSpace(focusedTextCtrl->GetCharAt(s)))
		s++;
	while (e > s && isSpace(focusedTextCtrl->GetCharAt(e - 1)))
		e--;

	const bool hasOpen = (e - s >= 2) && focusedTextCtrl->GetCharAt(s) == '/' &&
						 focusedTextCtrl->GetCharAt(s + 1) == '*';
	const bool hasClose = (e - s >= 4) &&
						  focusedTextCtrl->GetCharAt(e - 2) == '*' &&
						  focusedTextCtrl->GetCharAt(e - 1) == '/';

	auto unwrap = [&](wxStyledTextCtrl *ctrl) {
		ctrl->BeginUndoAction();
		ctrl->DeleteRange(e - 2, 2);
		ctrl->DeleteRange(s, 2);
		ctrl->EndUndoAction();
	};

	auto wrap = [&](wxStyledTextCtrl *ctrl) {
		ctrl->BeginUndoAction();
		ctrl->InsertText(e, "*/");
		ctrl->InsertText(s, "/*");
		ctrl->EndUndoAction();
	};

	if (hasOpen && hasClose) {
		unwrap(focusedTextCtrl);
		focusedTextCtrl->SetSelection(selStart, wxMax(selStart, selEnd - 4));
	} else {
		wrap(focusedTextCtrl);
		focusedTextCtrl->SetSelection(selStart, selEnd + 4);
	}
}