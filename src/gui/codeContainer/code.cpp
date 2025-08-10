#include "platformInfos/platformInfos.hpp"
#include "appConstants/appConstants.hpp"
#include "core/lexerStyles/lexerStyle.hpp"
#include "languagesPreferences/languagesPreferences.hpp"

#include "./code.hpp"
#include <wx/filename.h>

CodeContainer::CodeContainer(wxWindow *parent, wxString path) : wxScrolled<wxPanel>(parent)
{
    // setting components
    editor = new Editor(this);
    minimap = new MiniMap(this);

    sizer->Add(editor, 1, wxEXPAND);
    sizer->Add(minimap, 0, wxEXPAND);

    SetSizerAndFit(sizer);

    if (UserSettings["show_minimap"] == false)
        minimap->Hide();

    if (PlatformInfos::IsWindows())
    {
        font = wxFont(wxFontInfo(10).FaceName("Cascadia Code"));
    }
    else
    {
        font = wxFont(wxFontInfo(10).FaceName("Monospace"));
    }

    SetName(path);
    SetLabel(path + "_codeContainer");
    LoadPath(path);
    currentPath = path;

    // keyboard shortcuts
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
    if (file_props.IsOk() && file_props.FileExists())
    {

        // editor setup
        editor->SetLabel(path + "_codeEditor");
        editor->SetName(path);
        editor->LoadFile(path);
        wxString text = editor->GetText().Upper();
        editor->HighlightSyntax(0, editor->GetTextLength(), text);
        editor->UpdateFoldLevels(0, 0, text);

        // minimap setup
        minimap->SetLabel(path + "_codeMap");
        minimap->SetName(path);
        minimap->LoadFile(path);
        statusBar->UpdateCodeLocale(editor);

        languagePreferences = LanguagesPreferences::Get().SetupLanguagesPreferences(this);
    }
    else
    {
        wxLogError("There was an error opening the file");
    }
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
                ((wxStaticBitmap *)tab->GetChildren()[0]->GetChildren()[2])
                    ->SetBitmap(wxBitmapBundle::FromBitmap(wxBitmap(ApplicationPaths::AssetsPath("icons") + "close.png", wxBITMAP_TYPE_PNG)));
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


void CodeContainer::OnToggleMinimapView(wxCommandEvent &WXUNUSED(event))
{
    auto mainCodeComponent = FindWindowById(+GUI::ControlID::MainCode);
    wxWindow *currentCodeEditor = wxFindWindowByLabel(ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeContainer");
    if (currentCodeEditor)
    {
        auto currentMinimap = ((wxStyledTextCtrl *)currentCodeEditor->GetChildren()[1]);
        if (currentMinimap)
        {
            if (currentMinimap->IsShown())
                currentMinimap->Hide();
            else
                currentMinimap->Show();

            // updating gui components
            mainCodeComponent->GetSizer()->Layout();

            // updating user settings
            UserSettings["show_minimap"] = currentMinimap->IsShown();
            UserSettings.update(UserSettings);
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
    wxWindow *currentCodeEditor = wxFindWindowByLabel(ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeContainer");
    if (currentCodeEditor)
    {
        auto currentEditor = ((wxStyledTextCtrl *)currentCodeEditor->GetChildren()[0]);
        auto currentMinimap = ((wxStyledTextCtrl *)currentCodeEditor->GetChildren()[1]);

        if (currentEditor && currentMinimap)
        {

            int lineStart = 0;
            int lineEnd = 0;
            if (currentEditor->GetSelectionEnd() - currentEditor->GetSelectionStart() <= 0)
            {
                lineStart = currentEditor->PositionFromLine(currentEditor->GetCurrentLine());
                lineEnd = currentEditor->GetLineEndPosition(currentEditor->GetCurrentLine()) + 2;
            }
            else
            {
                lineStart = currentEditor->GetSelectionStart();
                lineEnd = currentEditor->GetSelectionEnd() + 2;
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

            if (chr == '/' && (char)currentEditor->GetCharAt(lineStart + 1) == '*')
            {
                currentEditor->DeleteRange(lineStart, 2);
                currentMinimap->DeleteRange(lineStart, 2);
            }
            else
            {
                currentEditor->InsertText(lineStart, "/*");
                currentEditor->InsertText(lineEnd, "*/");

                currentMinimap->InsertText(lineStart, "/*");
                currentMinimap->InsertText(lineEnd, "*/");
            }
        }
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