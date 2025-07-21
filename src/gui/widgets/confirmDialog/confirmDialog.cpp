#include "confirmDialog.hpp"

ConfirmDialog::ConfirmDialog(wxWindow *parent, const wxString &message, const wxString &title)
    : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
    wxStaticText *msg = new wxStaticText(this, wxID_ANY, message);
    m_checkBox = new wxCheckBox(this, wxID_ANY, "Don't ask me again");
    wxButton *btnConfirm = new wxButton(this, wxID_OK, "Confirm");
    wxButton *btnCancel = new wxButton(this, wxID_CANCEL, "Cancel");

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);

    mainSizer->Add(msg, 0, wxALL | wxEXPAND, 10);
    mainSizer->Add(m_checkBox, 0, wxLEFT | wxBOTTOM | wxRIGHT, 10);
    buttonSizer->Add(btnConfirm, 0, wxRIGHT, 5);
    buttonSizer->Add(btnCancel, 0, wxLEFT, 5);
    mainSizer->Add(buttonSizer, 0, wxALIGN_RIGHT | wxALL, 10);

    SetSizerAndFit(mainSizer);
    Center();
}

bool ConfirmDialog::DontAskAgain() const { return m_checkBox->IsChecked(); }