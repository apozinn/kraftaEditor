#include <wx/wx.h>

class ConfirmDialog : public wxDialog
{
public:
    ConfirmDialog(wxWindow *parent, const wxString &message, const wxString &title);
    bool DontAskAgain() const;

private:
    wxCheckBox *m_checkBox;
};