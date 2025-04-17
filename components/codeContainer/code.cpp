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

    if (UserConfigs["show_minimap"] == false)
        minimap->Hide();

    if (osName == "Windows")
    {
        font = wxFont(wxFontInfo(10).FaceName("Cascadia Code"));
    }
    else
    {
        font = wxFont(wxFontInfo(10).FaceName("Monospace"));
    }

    LoadPath(path);
    SetName(path + "_codeContainer");
    SetLabel(path);
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
        minimap->LoadFile(path);

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

void CodeContainer::OnSave(wxCommandEvent &event)
{
    wxWindow *currentCodeEditor = wxFindWindowByName(current_openned_path + "_codeContainer");
    if (currentCodeEditor)
    {
        auto currentEditor = ((Editor *)currentCodeEditor->GetChildren()[0]);
        if (currentEditor)
            currentEditor->Save();
    }
}

LanguageInfo const *CodeContainer::GetFilelanguage(wxString filename)
{
    // searching for the language with this file extension
    for (auto &language : languages_prefs)
    {
        wxString filepattern = language.filepattern;
        filepattern.Lower();
        while (!filepattern.empty())
        {
            wxString cur = filepattern.BeforeFirst(';');
            if ((cur == filename) ||
                (cur == (filename.BeforeLast('.') + ".*")) ||
                (cur == ("*." + filename.AfterLast('.'))))
            {
                currentLanguage = &language;
            }
            filepattern = filepattern.AfterFirst(';');
        }
    }

    if (!currentLanguage)
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

void CodeContainer::ToggleCommentLine(wxCommandEvent &WXUNUSED(event))
{
    wxWindow *currentCodeEditor = wxFindWindowByName(current_openned_path + "_codeContainer");
    if (currentCodeEditor)
    {
        auto currentEditor = ((wxStyledTextCtrl *)currentCodeEditor->GetChildren()[0]);
        auto currentMinimap = ((wxStyledTextCtrl *)currentCodeEditor->GetChildren()[1]);

        if (currentEditor && currentMinimap)
        {
            int lineStart = currentEditor->PositionFromLine(currentEditor->GetCurrentLine());
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

void CodeContainer::ToggleMiniMapView(wxCommandEvent &event)
{
    auto mainCodeComponent = FindWindowById(ID_MAIN_CODE);
    wxWindow *currentCodeEditor = wxFindWindowByName(current_openned_path + "_codeContainer");
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