#include "terminal.hpp"
#include <wx/txtstrm.h>
#include <wx/process.h>

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

    Bind(wxEVT_SHOW, [this](wxShowEvent &WXUNUSED(event))
         {
        if(m_commandInput->GetValue().IsEmpty() && !ProjectSettings::Get().GetProjectPath().IsEmpty()) {
            m_commandInput->AppendText(ProjectSettings::Get().GetProjectPath()+">");
        } });
}

void Terminal::OnCommand(wxCommandEvent &WXUNUSED(event))
{
    wxString command = m_commandInput->GetValue().substr(
        m_commandInput->GetValue().find_last_of(ProjectSettings::Get().GetProjectPath() + ">") - 1,
        m_commandInput->GetValue().Len());

    wxProcess *process = new wxProcess(this);
    process->Redirect();

#ifdef __WINDOWS__
    long pid = wxExecute("cmd /C " + command, wxEXEC_ASYNC, process);
#else
    long pid = wxExecute(command, wxEXEC_ASYNC, process);
#endif

    if (!pid)
    {
        m_commandInput->AppendText("Erro ao executar comando\n");
        return;
    }

    wxInputStream *inputStream = process->GetInputStream();
    if (inputStream)
    {
        wxTextInputStream textStream(*inputStream);
        while (!inputStream->Eof())
        {
            wxString line = textStream.ReadLine();
            m_commandInput->AppendText(line + "\n");
        }
    }

    wxInputStream *errorStream = process->GetErrorStream();
    if (errorStream)
    {
        wxTextInputStream textStream(*errorStream);
        while (!errorStream->Eof())
        {
            wxString line = textStream.ReadLine();
            m_commandInput->AppendText(line + "\n");
        }
    }

    m_commandInput->AppendText(ProjectSettings::Get().GetProjectPath() + ">");
}

void Terminal::OnCommandInputClick(wxMouseEvent &WXUNUSED(event))
{
    m_commandInput->SetInsertionPoint(m_commandInput->GetValue().Len());
}