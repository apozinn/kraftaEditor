#include "platformInfos/platformInfos.hpp"
#include "appConstants/appConstants.hpp"
#include "core/lexerStyles/lexerStyle.hpp"

#include "./code.hpp"
#include <wx/filename.h>

#define STRINGIFY(x) #x

wxBEGIN_EVENT_TABLE(CodeContainer, wxScrolled<wxPanel>)
    wxEND_EVENT_TABLE()

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

    LoadPath(path);
    SetName(path);
    SetLabel(path + "_codeContainer");
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
        editor->LoadFile(path);
        wxString text = editor->GetText().Upper();
        editor->highlightSTCsyntax(0, editor->GetTextLength(), text);
        editor->setfoldlevels(0, 0, text);
        editor->SetLabel(path + "_codeEditor");

        minimap->LoadFile(path);
        minimap->SetLabel(path + "_codeMap");

        statusBar->UpdateCodeLocale(editor);
        GetFilelanguage(path);
        InitializeLanguagePrefs();
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

LanguageInfo const *CodeContainer::GetFilelanguage(wxString filename)
{
    // searching for the language with this file extension
    LanguageInfo const *currentInfo;
    int languageNr;

    bool found = false;
    for (languageNr = 0; languageNr < languages_prefs_size; languageNr++)
    {
        currentInfo = &languages_prefs[languageNr];
        wxString filepattern = currentInfo->filepattern;
        filepattern.Lower();

        while (!filepattern.empty() && !found)
        {
            wxString cur = filepattern.BeforeFirst(';');
            if ((cur == filename) ||
                (cur == (filename.BeforeLast('.') + ".*")) ||
                (cur == ("*." + filename.AfterLast('.'))))
            {
                found = true;
                currentLanguage = currentInfo;
            }
            filepattern = filepattern.AfterFirst(';');
        }
    }

    if (!found)
        currentLanguage = &languages_prefs[0];
    return currentLanguage;
}

void CodeContainer::InitializeLanguagePrefs()
{
    if (!editor || !minimap)
        return;

    // setting the lexer
    editor->SetLexer(currentLanguage->lexer);
    minimap->SetLexer(currentLanguage->lexer);

    // setting the same font for all styles
    int Nr;
    for (Nr = 0; Nr < wxSTC_STYLE_LASTPREDEFINED; Nr++)
    {
        editor->StyleSetFont(Nr, font);
        minimap->StyleSetFont(Nr, font);
    }

    if (g_CommonPrefs.syntaxEnable)
    {
        int keywordnr = 0;
        for (Nr = 0; Nr < StyleConstants::TYPES_COUNT; Nr++)
        {
            if (currentLanguage->styles[Nr].type == -1)
                continue;

            const StyleInfo &curType = global_lexer_styles[currentLanguage->styles[Nr].type];
            auto setStyles = [&](wxStyledTextCtrl *component)
            {
                component->StyleSetFont(Nr, font);
                component->StyleSetForeground(Nr, curType.foreground);
                component->StyleSetBold(Nr, (curType.fontstyle & mySTC_STYLE_BOLD) > 0);
                component->StyleSetItalic(Nr, (curType.fontstyle & mySTC_STYLE_ITALIC) > 0);
                component->StyleSetUnderline(Nr, (curType.fontstyle & mySTC_STYLE_UNDERL) > 0);
                component->StyleSetVisible(Nr, (curType.fontstyle & mySTC_STYLE_HIDDEN) == 0);
                component->StyleSetCase(Nr, curType.lettercase);
            };

            const char *pwords = currentLanguage->styles[Nr].words;
            if (pwords)
            {
                editor->SetKeyWords(keywordnr, pwords);
                minimap->SetKeyWords(keywordnr, pwords);
                keywordnr += 1;
            }

            setStyles(editor);
            setStyles(minimap);
        }
    }
}

// void CodeContainer::SetupSyntaxeStyles(const wxStyledTextCtrl &component)
// {
// }

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