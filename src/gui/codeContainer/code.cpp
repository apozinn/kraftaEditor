#include "platformInfos/platformInfos.hpp"
#include "appConstants/appConstants.hpp"
#include "languagesPreferences/languagesPreferences.hpp"

#include "./code.hpp"
#include <wx/filename.h>

#include <wx/stc/stc.h>

CodeContainer::CodeContainer(wxWindow *parent, wxString path) : wxPanel(parent, wxID_ANY, wxDefaultPosition)
{
    Hide();

    sizer = new wxBoxSizer(wxHORIZONTAL);

    editor = new Editor(this);
    editor->SetMinSize(wxSize(parent->GetSize().x - 100, parent->GetSize().y));
    minimap = new MiniMap(this, editor);

    editor->m_linked_minimap = minimap;

    sizer->Add(editor, 1, wxEXPAND);
    sizer->Add(minimap, 0, wxEXPAND);

    SetSizerAndFit(sizer);
    LoadPath(path);
    Layout();

    if (UserSettings["show_minimap"] == false)
        minimap->Hide();

    if (PlatformInfos::IsWindows())
        font = wxFont(wxFontInfo(10).FaceName("Cascadia Code"));
    else
        font = wxFont(wxFontInfo(10).FaceName("Monospace"));

    wxAcceleratorEntry entries[2];
    entries[0].Set(wxACCEL_CTRL, WXK_CONTROL_S, +Event::File::Save);
    entries[0].FromString("Ctrl+S");
    entries[1].Set(wxACCEL_CTRL, WXK_SHIFT, +Event::File::SaveAs);
    entries[1].FromString("Ctrl+Shift+S");
    wxAcceleratorTable accel(2, entries);
    SetAcceleratorTable(accel);
}

void CodeContainer::LoadPath(wxString path)
{
    wxFileName file_props(path);
    if (file_props.IsOk() && file_props.FileExists() && editor)
    {
        SetName(path);
        SetLabel(path + "_codeContainer");
        currentPath = path;

        editor->SetLabel(path + "_codeEditor");
        editor->SetName(path);
        editor->LoadFile(path);

        minimap->SetLabel(path + "_codeMap");
        minimap->SetName(path);
        statusBar->UpdateComponents(path);

        languagePreferences = LanguagesPreferences::Get().SetupLanguagesPreferences(this);

        editor->SetAutoCompleteWordsList(LanguagesPreferences::Get().GetAutoCompleteWordsList(languagePreferences));
        editor->SetLanguagesPreferences(languagePreferences);

        Save(path);
        editor->SendMsg(4003, 0, -1);

        minimap->languagePreferences = languagePreferences;
        minimap->ExtractStyledText();
    }
    else
    {
        wxLogError("There was an error opening the file");
    }

    GetParent()->Layout();
    Layout();
}

void CodeContainer::OnSave(wxCommandEvent &WXUNUSED(event))
{
    Save(ProjectSettings::Get().GetCurrentlyFileOpen());
}

bool CodeContainer::Save(wxString path)
{
    auto currentEditor = ((Editor *)wxFindWindowByLabel(path + "_codeEditor"));
    if (currentEditor)
    {
        if (currentEditor->SaveFile(path) && !currentEditor->Modified())
        {
            if (auto tab = FindWindowByLabel(path + "_tab"))
            {
                auto icon = ((wxStaticBitmap *)tab->GetChildren()[0]->GetChildren()[2]);
                if (icon)
                {
                    icon->SetBitmap(wxBitmapBundle::FromBitmap(wxBitmap(ApplicationPaths::AssetsPath("icons") + "close.png", wxBITMAP_TYPE_PNG)));
                    icon->SetLabel("saved_icon");
                    tab->Layout();
                }
            }
            return true;
        }
        else
        {
            wxMessageBox(_("File could not be saved!"), _("Close abort"),
                         wxOK | wxICON_EXCLAMATION);
        }
    }
    return false;
}

void CodeContainer::OnSaveAs(wxCommandEvent &WXUNUSED(event))
{
    wxString filename;
    wxFileDialog dlg(this, "Save file", wxEmptyString, wxFileNameFromPath(ProjectSettings::Get().GetCurrentlyFileOpen()), "Any file (*)|*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() != wxID_OK)
        return;
    filename = dlg.GetPath();
    Save(filename);
}

void CodeContainer::OnSaveAll(wxCommandEvent &WXUNUSED(event))
{
    auto mainCode = FindWindowById(+GUI::ControlID::MainCode);
    if (mainCode)
    {
        for (auto &&children : mainCode->GetChildren())
        {
            if (children->GetLabel().ToStdString().find("_codeContainer") != std::string::npos)
            {
                Save(children->GetName());
            }
        }
    }
}

void CodeContainer::OnCloseFile(wxCommandEvent &WXUNUSED(event))
{
    auto currentEditor = ((Editor *)wxFindWindowByLabel(ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeEditor"));
    if (currentEditor)
    {
        if (currentEditor->Modified())
        {
            if (wxMessageBox(_("Text is not saved, save before closing?"), _("Close"),
                             wxYES_NO | wxICON_QUESTION) == wxYES)
            {
                currentEditor->SaveFile();
                if (currentEditor->Modified())
                {
                    wxMessageBox(_("Text could not be saved!"), _("Close abort"),
                                 wxOK | wxICON_EXCLAMATION);
                    return;
                }
            }
        }

        auto tabsContainer = ((Tabs *)FindWindowById(+GUI::ControlID::Tabs));
        if (tabsContainer)
        {
            if (auto tab = FindWindowByLabel(ProjectSettings::Get().GetCurrentlyFileOpen() + "_tab"))
            {
                tabsContainer->Close(tab, ProjectSettings::Get().GetCurrentlyFileOpen());
            }
        }
    }
}

void CodeContainer::OnRedo(wxCommandEvent &WXUNUSED(event))
{
    auto currentEditor = ((Editor *)wxFindWindowByLabel(ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeEditor"));
    if (currentEditor)
    {
        if (!currentEditor->CanRedo())
            return;
        currentEditor->Redo();
    }
}

void CodeContainer::OnUndo(wxCommandEvent &WXUNUSED(event))
{
    auto currentEditor = ((Editor *)wxFindWindowByLabel(ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeEditor"));
    if (currentEditor)
    {
        if (!currentEditor->CanUndo())
            return;
        currentEditor->Undo();
    }
}

void CodeContainer::OnCut(wxCommandEvent &WXUNUSED(event))
{
    auto currentEditor = ((Editor *)wxFindWindowByLabel(ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeEditor"));
    if (currentEditor)
    {
        if (currentEditor->GetReadOnly() || (currentEditor->GetSelectionEnd() - currentEditor->GetSelectionStart() <= 0))
            return;
        currentEditor->Cut();
    }
}

void CodeContainer::OnCopy(wxCommandEvent &WXUNUSED(event))
{
    auto currentEditor = ((Editor *)wxFindWindowByLabel(ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeEditor"));
    if (currentEditor)
    {
        if (currentEditor->GetSelectionEnd() - currentEditor->GetSelectionStart() <= 0)
            return;
        currentEditor->Copy();
    }
}

void CodeContainer::OnPaste(wxCommandEvent &WXUNUSED(event))
{
    auto currentEditor = ((Editor *)wxFindWindowByLabel(ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeEditor"));
    if (currentEditor)
    {
        if (!currentEditor->CanPaste())
            return;
        currentEditor->Paste();
    }
}

void CodeContainer::ToggleCommentLine(wxCommandEvent &WXUNUSED(event))
{
    wxWindow *currentCodeEditor = wxFindWindowByLabel(ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeContainer");
    if (currentCodeEditor)
    {
        auto currentEditor = ((wxStyledTextCtrl *)currentCodeEditor->GetChildren()[0]);
        auto currentMinimap = ((wxStyledTextCtrl *)currentCodeEditor->GetChildren()[1]);

        if (currentEditor && currentMinimap)
        {
            int lineStart = 0;
            if (currentEditor->GetSelectionEnd() - currentEditor->GetSelectionStart() <= 0)
            {
                lineStart = currentEditor->PositionFromLine(currentEditor->GetCurrentLine());
            }
            else
            {
                lineStart = currentEditor->GetSelectionStart();
            }

            char chr = (char)currentEditor->GetCharAt(lineStart);

            if (chr == ' ')
            {
                while (chr == ' ')
                {
                    lineStart++;
                    chr = (char)currentEditor->GetCharAt(lineStart);
                }
            }

            if (chr == '/' && (char)currentEditor->GetCharAt(lineStart + 1) == '/')
            {
                currentEditor->DeleteRange(lineStart, 2);
                currentMinimap->DeleteRange(lineStart, 2);
            }
            else
            {
                currentEditor->InsertText(lineStart, "//");
                currentMinimap->InsertText(lineStart, "//");
            }
        }
    }
}

void CodeContainer::ToggleCommentBlock(wxCommandEvent &WXUNUSED(event))
{
    wxWindow *currentCodeEditor =
        wxFindWindowByLabel(ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeContainer");
    if (!currentCodeEditor)
        return;

    if (currentCodeEditor->GetChildren().GetCount() < 2)
        return;
    auto *currentEditor = dynamic_cast<wxStyledTextCtrl *>(currentCodeEditor->GetChildren()[0]);
    auto *currentMinimap = dynamic_cast<wxStyledTextCtrl *>(currentCodeEditor->GetChildren()[1]);
    if (!currentEditor || !currentMinimap)
        return;

    int selStart = currentEditor->GetSelectionStart();
    int selEnd = currentEditor->GetSelectionEnd();
    if (selStart > selEnd)
        std::swap(selStart, selEnd);

    if (selStart == selEnd)
    {
        const int line = currentEditor->GetCurrentLine();
        selStart = currentEditor->PositionFromLine(line);
        selEnd = currentEditor->GetLineEndPosition(line);
    }

    auto isSpace = [&](int ch)
    {
        return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
    };

    int s = selStart;
    int e = selEnd;
    while (s < e && isSpace(currentEditor->GetCharAt(s)))
        s++;
    while (e > s && isSpace(currentEditor->GetCharAt(e - 1)))
        e--;

    const bool hasOpen = (e - s >= 2) &&
                         currentEditor->GetCharAt(s) == '/' &&
                         currentEditor->GetCharAt(s + 1) == '*';
    const bool hasClose = (e - s >= 4) &&
                          currentEditor->GetCharAt(e - 2) == '*' &&
                          currentEditor->GetCharAt(e - 1) == '/';

    auto unwrap = [&](wxStyledTextCtrl *ctrl)
    {
        ctrl->BeginUndoAction();
        ctrl->DeleteRange(e - 2, 2);
        ctrl->DeleteRange(s, 2);
        ctrl->EndUndoAction();
    };

    auto wrap = [&](wxStyledTextCtrl *ctrl)
    {
        ctrl->BeginUndoAction();
        ctrl->InsertText(e, "*/");
        ctrl->InsertText(s, "/*");
        ctrl->EndUndoAction();
    };

    if (hasOpen && hasClose)
    {
        unwrap(currentEditor);
        unwrap(currentMinimap);
        currentEditor->SetSelection(selStart, wxMax(selStart, selEnd - 4));
        currentMinimap->SetSelection(selStart, wxMax(selStart, selEnd - 4));
    }
    else
    {
        wrap(currentEditor);
        wrap(currentMinimap);
        currentEditor->SetSelection(selStart, selEnd + 4);
        currentMinimap->SetSelection(selStart, selEnd + 4);
    }
}

void CodeContainer::OnSelectAll(wxCommandEvent &WXUNUSED(event))
{
    auto currentEditor = ((Editor *)wxFindWindowByLabel(ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeEditor"));
    if (currentEditor)
    {
        currentEditor->SetSelection(0, currentEditor->GetTextLength());
    }
}

void CodeContainer::OnSelectLine(wxCommandEvent &WXUNUSED(event))
{
    auto currentEditor = ((Editor *)wxFindWindowByLabel(ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeEditor"));
    if (currentEditor)
    {
        int lineStart = currentEditor->PositionFromLine(currentEditor->GetCurrentLine());
        int lineEnd = currentEditor->PositionFromLine(currentEditor->GetCurrentLine() + 1);
        currentEditor->SetSelection(lineStart, lineEnd);
    }
}

void CodeContainer::OnMoveLineUp(wxCommandEvent &WXUNUSED(event))
{
    auto currentEditor = ((Editor *)wxFindWindowByLabel(ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeEditor"));
    if (currentEditor)
    {
        currentEditor->MoveSelectedLinesUp();
    }
}

void CodeContainer::OnMoveLineDown(wxCommandEvent &WXUNUSED(event))
{
    auto currentEditor = ((Editor *)wxFindWindowByLabel(ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeEditor"));
    if (currentEditor)
    {
        currentEditor->MoveSelectedLinesDown();
    }
}

void CodeContainer::OnRemoveCurrentLine(wxCommandEvent &WXUNUSED(event))
{
    auto currentEditor = ((Editor *)wxFindWindowByLabel(ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeEditor"));
    if (currentEditor)
    {
        currentEditor->RemoveCurrentLine();
    }
}