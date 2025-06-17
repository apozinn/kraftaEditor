#include "terminal.hpp"
#include <wx/txtstrm.h>
#include <wx/process.h>

Terminal::Terminal(wxWindow *parent, wxWindowID ID) : wxPanel(parent, ID)
{
    SetBackgroundColour(ThemesManager::Get().GetColor("main"));
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    m_output = new wxTextCtrl(this, wxID_ANY, "",
                              wxDefaultPosition, wxDefaultSize,
                              wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2 | wxBORDER_NONE);
    m_output->SetBackgroundColour(ThemesManager::Get().GetColor("main"));
    m_output->SetForegroundColour(ThemesManager::Get().GetColor("text"));

    m_input = new wxTextCtrl(this, wxID_ANY, "",
                             wxDefaultPosition, wxDefaultSize,
                             wxTE_PROCESS_ENTER | wxBORDER_NONE);
    m_input->SetBackgroundColour(ThemesManager::Get().GetColor("main"));
    m_input->SetForegroundColour(ThemesManager::Get().GetColor("text"));

    m_input->SetFocus();

    m_input->Bind(wxEVT_TEXT_ENTER, &Terminal::OnCommand, this);

    sizer->Add(m_output, 1, wxEXPAND);
    sizer->Add(m_input, 0, wxEXPAND);
    SetSizer(sizer);
}

void Terminal::OnCommand(wxCommandEvent &WXUNUSED(event))
{
    wxString cmd = m_input->GetValue();
    m_output->AppendText("> " + cmd + "\n");
    m_input->Clear();

    // Cria um processo para capturar a saída
    wxProcess *process = new wxProcess(this);
    process->Redirect();

#ifdef __WINDOWS__
    long pid = wxExecute("cmd /C " + cmd, wxEXEC_ASYNC, process);
#else
    long pid = wxExecute(cmd, wxEXEC_ASYNC, process);
#endif

    if (!pid)
    {
        m_output->AppendText("Erro ao executar comando\n");
        return;
    }

    // Lê a saída do processo
    wxInputStream *inputStream = process->GetInputStream();
    if (inputStream)
    {
        wxTextInputStream textStream(*inputStream);
        while (!inputStream->Eof())
        {
            wxString line = textStream.ReadLine();
            m_output->AppendText(line + "\n");
        }
    }

    // Lê os erros do processo
    wxInputStream *errorStream = process->GetErrorStream();
    if (errorStream)
    {
        wxTextInputStream textStream(*errorStream);
        while (!errorStream->Eof())
        {
            wxString line = textStream.ReadLine();
            m_output->AppendText(line + "\n");
        }
    }
}