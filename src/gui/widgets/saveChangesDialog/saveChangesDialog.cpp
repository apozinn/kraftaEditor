#include "saveChangesDialog.hpp"
#include "ui/ids.hpp"

SaveChangesDialog::SaveChangesDialog(wxWindow *parent, const wxString &message, const wxString &title)
    : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
    wxStaticText *msg = new wxStaticText(this, wxID_ANY, message);
    wxButton *btnConfirm = new wxButton(this, wxID_OK, "Save");
    wxButton *btnDontSave = new wxButton(this, +Event::Frame::DontSaveChanges, "Don't save");
    btnDontSave->Bind(wxEVT_BUTTON, [this](wxCommandEvent &)
                      { EndModal(+Event::Frame::DontSaveChanges); });

    wxButton *btnCancel = new wxButton(this, wxID_CANCEL, "Cancel");

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);

    mainSizer->Add(msg, 0, wxALL | wxEXPAND, 10);
    buttonSizer->Add(btnConfirm, 0, wxRIGHT, 5);
    buttonSizer->Add(btnDontSave, 0, wxRIGHT, 5);
    buttonSizer->Add(btnCancel, 0, wxLEFT, 5);
    mainSizer->Add(buttonSizer, 0, wxALIGN_RIGHT | wxALL, 10);

    SetSizerAndFit(mainSizer);
    Center();
}