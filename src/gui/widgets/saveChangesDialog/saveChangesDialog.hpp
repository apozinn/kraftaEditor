#include <wx/wx.h>

class SaveChangesDialog : public wxDialog
{
public:
    SaveChangesDialog(wxWindow *parent, const wxString &message, const wxString &title);
};