#include "./code.hpp"
#include <wx/filename.h>
#include "path/path.hpp"
#include "platform/platform.hpp"

CodeContainer::CodeContainer(wxWindow *parent, wxString path) : wxScrolled<wxPanel>(parent)
{
    // setting components
    editor = new Editor(this);
    minimap = new MiniMap(this);

    sizer->Add(editor, 1, wxEXPAND);
    sizer->Add(minimap, 0, wxEXPAND);

    SetSizerAndFit(sizer);

    if (UserConfigs["show_minimap"] == false)
        minimap->Hide();

    if (Platform::OsName() == "Windows")
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
    wxAcceleratorEntry entries[1];
    entries[0].Set(wxACCEL_CTRL, WXK_CONTROL_S, wxID_SAVE);
    entries[0].FromString("Ctrl+S");
    wxAcceleratorTable accel(1, entries);
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

        if (mainFrame)
        {
            mainFrame->SetTitle(wxFileNameFromPath(path) + " (" + project_name + ") - Krafta Editor");
        }

        GetFilelanguage(path);
        InitializeLanguagePrefs();
    }
    else
    {
        wxLogError("There was an error opening the file");
    }
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
                    ->SetBitmap(wxBitmapBundle::FromBitmap(wxBitmap(ApplicationPaths::IconsPath() + "close.png", wxBITMAP_TYPE_PNG)));
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

void CodeContainer::OnSave(wxCommandEvent &event)
{
    Save(current_openned_path);
}

void CodeContainer::OnSaveAs(wxCommandEvent &event)
{
    wxString filename;
    wxFileDialog dlg(this, "Save file", wxEmptyString, wxFileNameFromPath(current_openned_path), "Any file (*)|*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() != wxID_OK)
        return;
    filename = dlg.GetPath();
    Save(filename);
}

void CodeContainer::OnSaveAll(wxCommandEvent &event)
{
    auto mainCode = FindWindowById(ID_MAIN_CODE);
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

void CodeContainer::OnCloseFile(wxCommandEvent &event)
{
    auto currentEditor = ((Editor *)wxFindWindowByLabel(current_openned_path + "_codeEditor"));
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

        auto tabsContainer = ((Tabs *)FindWindowById(ID_TABS));
        if (tabsContainer)
        {
            if (auto tab = FindWindowByLabel(current_openned_path + "_tab"))
            {
                tabsContainer->Close(tab, current_openned_path);
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

    if (global_commonPrefs.syntaxEnable)
    {
        int keywordnr = 0;
        for (Nr = 0; Nr < STYLE_TYPES_COUNT; Nr++)
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

void CodeContainer::OnToggleMinimapView(wxCommandEvent &event)
{
    auto mainCodeComponent = FindWindowById(ID_MAIN_CODE);
    wxWindow *currentCodeEditor = wxFindWindowByLabel(current_openned_path + "_codeContainer");
    if (currentCodeEditor)
    {
        auto currentMinimap = ((wxStyledTextCtrl *)currentCodeEditor->GetChildren()[1]);
        if (currentMinimap)
        {
            if (currentMinimap->IsShown())
                currentMinimap->Hide();
            else
                currentMinimap->Show();

            // updating the main code component to resize layout
            mainCodeComponent->GetSizer()->Layout();

            // updating the user configs file
            json user_config = UserConfig().Get();
            user_config["show_minimap"] = currentMinimap->IsShown();
            UserConfig().Update(user_config);
        }
    }
}

void CodeContainer::OnRedo(wxCommandEvent &WXUNUSED(event))
{
    auto currentEditor = ((Editor *)wxFindWindowByLabel(current_openned_path + "_codeEditor"));
    if (currentEditor)
    {
        if (!currentEditor->CanRedo())
            return;
        currentEditor->Redo();
    }
}

void CodeContainer::OnUndo(wxCommandEvent &WXUNUSED(event))
{
    auto currentEditor = ((Editor *)wxFindWindowByLabel(current_openned_path + "_codeEditor"));
    if (currentEditor)
    {
        if (!currentEditor->CanUndo())
            return;
        currentEditor->Undo();
    }
}

void CodeContainer::OnClear(wxCommandEvent &WXUNUSED(event))
{
    auto currentEditor = ((Editor *)wxFindWindowByLabel(current_openned_path + "_codeEditor"));
    if (currentEditor)
    {
        if (!currentEditor->CanUndo())
            return;
        currentEditor->Undo();
    }
}

void CodeContainer::OnCut(wxCommandEvent &WXUNUSED(event))
{
    auto currentEditor = ((Editor *)wxFindWindowByLabel(current_openned_path + "_codeEditor"));
    if (currentEditor)
    {
        if (currentEditor->GetReadOnly() || (currentEditor->GetSelectionEnd() - currentEditor->GetSelectionStart() <= 0))
            return;
        currentEditor->Cut();
    }
}

void CodeContainer::OnCopy(wxCommandEvent &WXUNUSED(event))
{
    auto currentEditor = ((Editor *)wxFindWindowByLabel(current_openned_path + "_codeEditor"));
    if (currentEditor)
    {
        if (currentEditor->GetSelectionEnd() - currentEditor->GetSelectionStart() <= 0)
            return;
        currentEditor->Copy();
    }
}

void CodeContainer::OnPaste(wxCommandEvent &WXUNUSED(event))
{
    auto currentEditor = ((Editor *)wxFindWindowByLabel(current_openned_path + "_codeEditor"));
    if (currentEditor)
    {
        if (!currentEditor->CanPaste())
            return;
        currentEditor->Paste();
    }
}

void CodeContainer::ToggleCommentLine(wxCommandEvent &WXUNUSED(event))
{
    wxWindow *currentCodeEditor = wxFindWindowByLabel(current_openned_path + "_codeContainer");
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
    wxWindow *currentCodeEditor = wxFindWindowByLabel(current_openned_path + "_codeContainer");
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
    auto currentEditor = ((Editor *)wxFindWindowByLabel(current_openned_path + "_codeEditor"));
    if (currentEditor)
    {
        currentEditor->SetSelection(0, currentEditor->GetTextLength());
    }
}

void CodeContainer::OnSelectLine(wxCommandEvent &WXUNUSED(event))
{
    auto currentEditor = ((Editor *)wxFindWindowByLabel(current_openned_path + "_codeEditor"));
    if (currentEditor)
    {
        int lineStart = currentEditor->PositionFromLine(currentEditor->GetCurrentLine());
        int lineEnd = currentEditor->PositionFromLine(currentEditor->GetCurrentLine() + 1);
        currentEditor->SetSelection(lineStart, lineEnd);
    }
}