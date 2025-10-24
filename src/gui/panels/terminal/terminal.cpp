#include "terminal.hpp"
#include <wx/txtstrm.h>
#include <wx/event.h>

#include "projectSettings/projectSettings.hpp"
#include "ui/ids.hpp"

Terminal::Terminal(wxWindow *parent, wxWindowID ID) : wxPanel(parent, ID)
{
    SetBackgroundColour(ThemesManager::Get().GetColor("main"));
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

    m_commandInput = new wxTextCtrl(
        this,
        +GUI::ControlID::TerminalCommandInput,
        "", wxDefaultPosition,
        wxDefaultSize,
        wxTE_MULTILINE | wxTE_PROCESS_ENTER | wxTE_RICH2 | wxBORDER_NONE);

    m_commandInput->SetBackgroundColour(ThemesManager::Get().GetColor("main"));
    m_commandInput->SetForegroundColour(ThemesManager::Get().GetColor("text"));

    m_commandInput->SetFocus();

    m_commandInput->Bind(wxEVT_TEXT_ENTER, &Terminal::OnCommand, this);
    m_commandInput->Bind(wxEVT_LEFT_DOWN, &Terminal::OnCommandInputClick, this);

    sizer->Add(m_commandInput, 1, wxEXPAND);
    SetSizer(sizer);

    Bind(wxEVT_SHOW,
         [this](wxShowEvent &(event))
         {
             if (!event.IsShown())
                 return;

             if (!m_commandInput)
                 return;

             m_commandInput->SetFocus();

             if (m_commandInput->GetValue().IsEmpty() && !ProjectSettings::Get().GetProjectPath().IsEmpty())
             {
                 m_commandInput->AppendText(ProjectSettings::Get().GetProjectPath() + ">");
             }
         });
}

void Terminal::OnCommand(wxCommandEvent & WXUNUSED(event))
{
    wxString prompt = ProjectSettings::Get().GetProjectPath() + ">";

    int lastPromptPos = m_commandInput->GetValue().find_last_of(prompt);
    if (static_cast<size_t>(lastPromptPos) == wxString::npos)
    {
        lastPromptPos = 0;
    }
    else
    {
        lastPromptPos += prompt.Length();
    }

    wxString fullText = m_commandInput->GetValue();
    wxString command = fullText.Mid(lastPromptPos);
    command.Trim(true).Trim(false);

    m_process = new wxProcess(this);
    m_process->Redirect();

#ifdef __WINDOWS__
    long pid = wxExecute("cmd /C " + command, wxEXEC_ASYNC, m_process);
#else
    long pid = wxExecute(command, wxEXEC_ASYNC, m_process);
#endif

    if (!pid)
    {
        m_commandInput->AppendText("\nErro ao executar comando\n");
        m_commandInput->AppendText(prompt);
        return;
    }

    wxInputStream *inputStream = m_process->GetInputStream();
    if (inputStream)
    {
        wxTextInputStream textStream(*inputStream);
        m_commandInput->AppendText("\n");
        while (!inputStream->Eof())
        {
            wxString line = textStream.ReadLine();
            m_commandInput->AppendText(line + "\n");
        }
    }

    wxInputStream *errorStream = m_process->GetErrorStream();
    if (errorStream)
    {
        wxTextInputStream textStream(*errorStream);
        m_commandInput->AppendText("\n");
        while (!errorStream->Eof())
        {
            wxString line = textStream.ReadLine();
            m_commandInput->AppendText(line + "\n");
        }
    }

    m_commandInput->AppendText(prompt);
}

void Terminal::OnCommandInputClick(wxMouseEvent & WXUNUSED(event))
{
    m_commandInput->SetInsertionPoint(m_commandInput->GetValue().Len());
}