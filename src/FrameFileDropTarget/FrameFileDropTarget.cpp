#include "FrameFileDropTarget.hpp"
#include <ui/ids.hpp>
#include <gui/panels/tabs/tabs.hpp>
#include <gui/panels/filesTree/filesTree.hpp>

FrameFileDropTarget::FrameFileDropTarget(wxFrame *frame)
    : m_frame(frame)
{
}

bool FrameFileDropTarget::IsWaylandSession() const
{
    return wxGetEnv("WAYLAND_DISPLAY", nullptr);
}

bool FrameFileDropTarget::OnDropFiles(wxCoord x, wxCoord y,
                                      const wxArrayString &filenames)
{
    if (filenames.empty())
    {
        if (IsWaylandSession())
        {
            FallbackOpenDialog();
        }

        return false;
    }

    for (const auto &file : filenames)
    {
        auto filesTree = static_cast<FilesTree *>(
            wxWindow::FindWindowById(+GUI::ControlID::FilesTree));

        if (filesTree)
            filesTree->OpenFile(file);

        wxCommandEvent evt(wxEVT_MENU, wxID_OPEN);
        evt.SetString(file);
        wxPostEvent(m_frame, evt);
    }

    return true;
}

void FrameFileDropTarget::FallbackOpenDialog()
{
    wxMessageBox(
        "Drag and drop is not fully supported on Wayland.\n"
        "Please use the file dialog to open files.",
        "Wayland Limitation",
        wxOK | wxICON_WARNING,
        m_frame);

    wxFileDialog dlg(
        m_frame,
        "Open file",
        "",
        "",
        "All files (*.*)|*.*",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (dlg.ShowModal() == wxID_OK)
    {
        wxCommandEvent evt(wxEVT_MENU, wxID_OPEN);
        evt.SetString(dlg.GetPath());
        wxPostEvent(m_frame, evt);
    }
}
