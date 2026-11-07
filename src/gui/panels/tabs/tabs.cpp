#include "tabs.hpp"
#include "gui/editor/controls/textCtrl/textCtrl.hpp"

#include <wx/wupdlock.h>

namespace {
void SetBackgroundRecursive(wxWindow *window, const wxColour &bg) {
    window->SetBackgroundColour(bg);
    for (wxWindow *child : window->GetChildren())
        SetBackgroundRecursive(child, bg);
}

void UpdateTabsColors(wxWindow *tabsContainer) {
    if (!tabsContainer)
        return;

    const wxString current = ProjectSettings::Get().GetCurrentlyFileOpen();
    for (wxWindow *tab : tabsContainer->GetChildren()) {
        const bool active = (tab->GetName() == current);
        const wxColour bg =
            ThemesManager::Get().GetColor(active ? "secondary" : "main");
        SetBackgroundRecursive(tab, bg);
        tab->Refresh(false);
    }
}

wxStaticBitmap *FindTabStateIcon(wxWindow *tabInfos) {
    if (!tabInfos)
        return nullptr;
    return dynamic_cast<wxStaticBitmap *>(
               tabInfos->FindWindow(wxString("tab_icon_close_or_unsaved")));
}

}

Tabs::Tabs(wxPanel *parent, wxWindowID ID) : wxPanel(parent, ID) {
    auto background_color = Theme["main"].template get<std::string>();
    SetBackgroundColour(wxColor(background_color));
    sizer = new wxBoxSizer(wxHORIZONTAL);

    tabsContainer =
        new wxScrolled<wxPanel>(this, +GUI::ControlID::TabsContainer);
    tabsContainer->SetBackgroundColour(wxColor(background_color));
    tabsContainerSizer = new wxBoxSizer(wxHORIZONTAL);
    tabsContainer->SetSizer(tabsContainerSizer);

    sizer->Add(tabsContainer, 1, wxEXPAND);

    if (ID != (+GUI::ControlID::SplitEditorTabs)) {
        wxPanel *rightMenus = new wxPanel(this);
        rightMenus->SetBackgroundColour(wxColor(background_color));
        auto rightMenusSizer = new wxBoxSizer(wxHORIZONTAL);

        auto splittEditor = new wxStaticBitmap(
            rightMenus, wxID_ANY,
            UiUtils::CreateIcon(
                ApplicationPaths::GetIconPath("split_editor.svg")));
        splittEditor->Bind(wxEVT_LEFT_UP, &Tabs::OnSplitEditorMenuClick, this);
        rightMenusSizer->Add(splittEditor, 0, wxEXPAND | wxRIGHT, 10);

        auto menuDown = new wxStaticBitmap(
            rightMenus, wxID_ANY,
            UiUtils::CreateIcon(
                ApplicationPaths::GetIconPath("menu_down.svg")));

        menuDown->Bind(wxEVT_LEFT_UP, &Tabs::OnDownMenuClick, this);
        rightMenusSizer->Add(menuDown, 0);

        rightMenus->SetSizer(rightMenusSizer);
        sizer->Add(rightMenus, 0, wxALIGN_CENTER | wxRIGHT, 10);
    }

    SetSizer(sizer);

    tabsContainer->SetScrollRate(20, 20);
    tabsContainer->EnableScrolling(true, false);

    SetLabel("tabsContainer");

#ifdef __WXMSW__
    // WS_EX_COMPOSITED: o Windows pinta o Tabs e todos os filhos num buffer
    // só, então não piscam separadamente. Só aqui, nunca no wxStyledTextCtrl.
    SetDoubleBuffered(true);
#endif

    Hide();
}

void Tabs::Add(wxString tab_name, wxString path) {
    wxWindow *parent = GetParent();
    wxWindowUpdateLocker freeze(parent ? parent : this);

    projectSettings.SetCurrentlyFileOpen(path);

    for (auto &a_tab : tabsContainer->GetChildren()) {
        if (a_tab->GetName() == path) {
            if (!IsShown())
                Show();
            UpdateTabsColors(tabsContainer);
            return;
        }
    }

    const wxColour activeBg = ThemesManager::Get().GetColor("secondary");

    wxPanel *new_tab = new wxPanel(tabsContainer);
    new_tab->Hide();
    new_tab->SetBackgroundStyle(wxBG_STYLE_PAINT);
    new_tab->SetBackgroundColour(activeBg);

    new_tab->SetName(path);
    new_tab->SetLabel(path + "_tab");
    new_tab->Bind(wxEVT_LEFT_UP, &Tabs::OnTabClick, this);
    wxBoxSizer *new_tab_sizer = new wxBoxSizer(wxVERTICAL);

    wxPanel *tab_infos = new wxPanel(new_tab);
    tab_infos->SetName(path);
    wxBoxSizer *tab_infos_sizer = new wxBoxSizer(wxHORIZONTAL);

    wxString iconPath;
    if (FileOperations::IsImageFile(path))
        iconPath = ApplicationPaths::GetLanguageIcon("image");
    else
        iconPath = LanguagesPreferences::Get().GetLanguageIconPath(path);

    if (!iconPath.IsEmpty()) {
        wxBitmapBundle iconBundle;

        if (iconPath.Lower().EndsWith(".svg")) {
            iconBundle = wxBitmapBundle::FromSVGFile(iconPath, wxSize(16, 16));
        } else {
            wxBitmap bmp(iconPath, wxBITMAP_TYPE_ANY);
            if (bmp.IsOk())
                iconBundle = wxBitmapBundle::FromBitmap(bmp);
        }

        if (iconBundle.IsOk()) {
            auto *icon = new wxStaticBitmap(tab_infos, wxID_ANY, iconBundle);
            icon->SetName("tab_language_icon");
            tab_infos_sizer->Add(icon, 0, wxALIGN_CENTER | wxLEFT, 10);
        }
    }

    wxStaticText *name = new wxStaticText(tab_infos, wxID_ANY, tab_name);
    name->SetToolTip(path);
    name->SetName(path);
    name->Bind(wxEVT_LEFT_UP, &Tabs::OnTabClick, this);
    tab_infos_sizer->Add(name, 1, wxEXPAND | wxLEFT | wxRIGHT, 15);

    wxStaticBitmap *close_icon = new wxStaticBitmap(
        tab_infos, wxID_ANY,
        UiUtils::CreateIcon(ApplicationPaths::GetIconPath("close.svg")));
    close_icon->SetName("tab_icon_close_or_unsaved");
    close_icon->Bind(wxEVT_LEFT_UP, &Tabs::OnCloseTab, this);
    tab_infos_sizer->Add(close_icon, 0, wxALIGN_CENTER | wxRIGHT, 10);

    tab_infos->Bind(wxEVT_ENTER_WINDOW, &Tabs::OnEnterComp, this);
    tab_infos->Bind(wxEVT_LEAVE_WINDOW, &Tabs::OnLeaveComp, this);

    tab_infos->SetSizerAndFit(tab_infos_sizer);
    new_tab_sizer->Add(tab_infos, 1, wxTOP | wxBOTTOM, 12);

    wxPanel *active_bar = new wxPanel(new_tab);
    new_tab_sizer->Add(active_bar, 0, wxEXPAND);

    new_tab->SetSizerAndFit(new_tab_sizer);
    tabsContainerSizer->Add(new_tab, 0, wxEXPAND);
    new_tab->Bind(wxEVT_PAINT, &Tabs::OnTabPaint, this);

    auto scrollWheelHandler = [this](wxMouseEvent &event) {
        int rotation = event.GetWheelRotation();
        int currentX, currentY;
        this->tabsContainer->GetViewStart(&currentX, &currentY);

        if (rotation > 0) {
            this->tabsContainer->Scroll(currentX - 20, currentY);
        } else if (rotation < 0) {
            this->tabsContainer->Scroll(currentX + 20, currentY);
        }
    };

    new_tab->Bind(wxEVT_MOUSEWHEEL, scrollWheelHandler);
    new_tab->CallForEachChild([scrollWheelHandler](auto child) {
        child->Bind(wxEVT_MOUSEWHEEL, scrollWheelHandler);
    });

    SetMinSize(wxSize(GetSize().GetWidth(), new_tab->GetSize().GetHeight()));

    UpdateTabsColors(tabsContainer);

    int endX = 0;
    for (wxWindow *t : tabsContainer->GetChildren())
        if (t != new_tab && t->IsShown())
            endX += t->GetSize().GetWidth();
    int px = 0, py = 0;
    tabsContainer->CalcScrolledPosition(endX, 0, &px, &py);
    new_tab->SetSize(px, py, new_tab->GetSize().GetWidth(),
                     tabsContainer->GetClientSize().GetHeight());

    new_tab->Show();
    if (!IsShown())
        Show();

    if (parent)
        parent->Layout();
    sizer->Layout();
    tabsContainerSizer->Layout();
    tabsContainer->FitInside();
    tabsContainer->Scroll(1000, 0);
}

void Tabs::Close(wxString tab_path) {
    auto editor = ((Editor *)FindWindowByName(tab_path + "_editor"));
    auto textCtrl = ((TextCtrl *)FindWindowByName(tab_path + "_textCtrl"));
    auto imgContainer =
        ((wxPanel *)FindWindowByLabel(tab_path + "_imageContainer"));
    auto fileContainer =
        ((FilesTree *)FindWindowById(+GUI::ControlID::FilesTree));
    auto mainCode = FindWindowById(+GUI::ControlID::MainCode);

    auto tab = FindWindowByLabel(tab_path + "_tab");
    if (!tab || !mainCode)
        return;

    const bool hasEditor = (editor && textCtrl);

    if (hasEditor && textCtrl->Modified()) {
        SaveChangesDialog dlg(
            NULL,
            wxString::Format(
                _("Do you want to save the changes you made to: %s?"),
                wxFileNameFromPath(tab_path)),
            "Krafta Editor");
        int result = dlg.ShowModal();

        if (result == wxID_OK) {
            editor->Save(tab_path);
        }
        if (result == +Event::Frame::DontSaveChanges) { }
        if (result == wxID_CANCEL) {
            return;
        }
    }

    wxWindowUpdateLocker freeze(mainCode);

    if (hasEditor)
        editor->Destroy();

    if (imgContainer)
        imgContainer->Destroy();

    wxWindow *descendantTab = NULL;

    if (ProjectSettings::Get().GetSecondSplittedEditorPath() != tab_path) {
        if (auto prevTab = tab->GetPrevSibling()) {
            descendantTab = prevTab;
        } else if (auto nextTab = tab->GetNextSibling()) {
            descendantTab = nextTab;
        }

        if (descendantTab) {
            projectSettings.SetCurrentlyFileOpen(descendantTab->GetName());

            auto linkedFile = FindWindowByLabel(
                                  ProjectSettings::Get().GetCurrentlyFileOpen() +
                                  "_file_container");
            if (linkedFile && fileContainer) {
                fileContainer->SetFileHighlight(linkedFile->GetName());
            }

            auto other_codeContainer = ((Editor *)FindWindowByName(
                                            ProjectSettings::Get().GetCurrentlyFileOpen() + "_editor"));
            if (other_codeContainer)
                other_codeContainer->Show();

            auto new_imageContainer = FindWindowByLabel(
                                          ProjectSettings::Get().GetCurrentlyFileOpen() +
                                          "_imageContainer");
            if (new_imageContainer)
                new_imageContainer->Show();
        } else {
            if (fileContainer)
                fileContainer->SetFileHighlight(wxEmptyString);

            Hide();

            auto emptyWindow = FindWindowById(+GUI::ControlID::EmptyWindow);
            if (emptyWindow) {
                emptyWindow->Show();
            }

            auto statusBar =
                ((StatusBar *)FindWindowById(+GUI::ControlID::StatusBar));
            if (statusBar) {
                statusBar->ClearLabels();
            }
        }
    }

    tab->Destroy();

    auto CodeContainerBlockRight =
        wxWindow::FindWindowById(+GUI::ControlID::CodeContainerBlockRight);
    wxSplitterWindow *mainCodeParent =
        CodeContainerBlockRight
        ? dynamic_cast<wxSplitterWindow *>(CodeContainerBlockRight->GetParent())
        : nullptr;

    if (GetId() == (+GUI::ControlID::SplitEditorTabs)) {
        if (mainCodeParent)
            mainCodeParent->Unsplit(CodeContainerBlockRight);

        auto mainTabs = ((Tabs *)FindWindowById(+GUI::ControlID::Tabs));
        if (mainTabs) {
            ProjectSettings::Get().SetCurrentlyFileOpen(
                ProjectSettings::Get().GetFirstSplittedEditorPath());
            ProjectSettings::Get().SetSplitEditorPaths("", "");

            mainTabs->Select();
        }
    } else {
        if (tab_path == ProjectSettings::Get().GetFirstSplittedEditorPath()) {

            auto splitEditorTab =
                ((Tabs *)FindWindowById(+GUI::ControlID::SplitEditorTabs));
            if (splitEditorTab) {

                if (mainCodeParent)
                    mainCodeParent->Unsplit(CodeContainerBlockRight);

                splitEditorTab->CloseAllFiles();
            }

            if (fileContainer) {
                fileContainer->OpenFile(
                    ProjectSettings::Get().GetSecondSplittedEditorPath(), 0);
            }

            ProjectSettings::Get().SetSplitEditorPaths("", "");
        }
    }

    UpdateTabsColors(tabsContainer);

    tabsContainer->GetSizer()->Layout();
    tabsContainer->FitInside();
    if (mainCode->GetSizer())
        mainCode->GetSizer()->Layout();

    if (GetId() == (+GUI::ControlID::SplitEditorTabs))
        Destroy();
}

void Tabs::CloseAllFiles() {
    wxWindow *parent = GetParent();
    if (!parent)
        return;

    wxWindowUpdateLocker freeze(parent);

    const wxWindowList childrenCopy = parent->GetChildren();

    std::vector<wxWindow *> toDestroy;
    toDestroy.reserve(childrenCopy.size());

    for (wxWindow *child : childrenCopy) {
        if (!child || child->IsBeingDeleted())
            continue;

        if (child->GetLabel().Contains("_editor")) {
            toDestroy.push_back(child);
        } else {
            child->Hide();
        }
    }

    for (wxWindow *child : toDestroy) {
        if (child && !child->IsBeingDeleted()) {
            child->Destroy();
        }
    }

    if (tabsContainer && !tabsContainer->IsBeingDeleted()) {
        tabsContainer->DestroyChildren();
    }

    Hide();

    if (auto *emptyWindow = FindWindowById(+GUI::ControlID::EmptyWindow)) {
        if (!emptyWindow->IsBeingDeleted()) {
            emptyWindow->Show();
        }
    }

    ProjectSettings::Get().SetCurrentlyFileOpen(wxEmptyString);
    parent->Layout();

    if (!statusBar || statusBar->IsBeingDeleted()) {
        statusBar = dynamic_cast<StatusBar *>(
                        FindWindowById(+GUI::ControlID::StatusBar));
    }

    if (statusBar && !statusBar->IsBeingDeleted()) {
        statusBar->ClearLabels();
    }
}

void Tabs::Select() {
    auto mainCode = FindWindowById(+GUI::ControlID::MainCode);
    if (!mainCode)
        return;

    wxWindowUpdateLocker freeze(mainCode);

    UpdateTabsColors(tabsContainer);

    for (auto &&other_ct : mainCode->GetChildren()) {
        if (other_ct->GetId() != +GUI::ControlID::Tabs)
            other_ct->Hide();
    }

    auto editor = ((Editor *)FindWindowByName(
                       ProjectSettings::Get().GetCurrentlyFileOpen() + "_editor"));
    if (editor)
        editor->Show();
    auto imageContainer = ((wxStaticBitmap *)FindWindowByLabel(
                               ProjectSettings::Get().GetCurrentlyFileOpen() + "_imageContainer"));
    if (imageContainer)
        imageContainer->Show();

    if (statusBar)
        statusBar->UpdateComponents(
            ProjectSettings::Get().GetCurrentlyFileOpen());

    if (mainCode->GetSizer())
        mainCode->GetSizer()->Layout();
    mainCode->Update();
}

void Tabs::OnTabClick(wxMouseEvent &event) {
    auto target = ((wxWindow *)event.GetEventObject());
    if (!target)
        return;

    wxString path = target->GetName();
    if (path == ProjectSettings::Get().GetCurrentlyFileOpen())
        return;

    ProjectSettings::Get().SetCurrentlyFileOpen(path);
    Select();
}

void Tabs::OnCloseTab(wxMouseEvent &event) {
    wxObject *obj = event.GetEventObject();
    auto this_tab = ((wxWindow *)obj)->GetGrandParent();

    auto *currentStatusBar = statusBar;

    if (this_tab)
        Tabs::Close(this_tab->GetName());

    if (currentStatusBar)
        currentStatusBar->ClearLabels();
}

void Tabs::OnDownMenuClick(wxMouseEvent &WXUNUSED(event)) {
    auto tabsContainerMenu = TabsContainerMenu::Get();
    if (!tabsContainerMenu) {
        wxMessageBox(ErrorMessages::CreateMenuContextError, "Error",
                     wxOK | wxICON_ERROR);
        return;
    }

    PopupMenu(tabsContainerMenu);
}

void Tabs::OnSplitEditorMenuClick(wxMouseEvent &WXUNUSED(event)) {
    int TabsLength = tabsContainer->GetChildren().size();

    if (TabsLength > 1) {
        auto GetOtherTabPath = []() {
            auto currentlyOpenTab = wxFindWindowByLabel(
                                        ProjectSettings::Get().GetCurrentlyFileOpen() + "_tab");
            if (currentlyOpenTab) {
                if (auto next = currentlyOpenTab->GetNextSibling()) {
                    return next->GetName();
                } else if (auto prev = currentlyOpenTab->GetPrevSibling()) {
                    return prev->GetName();
                } else
                    return wxString("");
            } else
                return wxString("");

            return wxString("");
        };

        SplitEditorManager::CreateSplitedEditor(GetOtherTabPath());
    }
}

void Tabs::OnEnterComp(wxMouseEvent &event) {
    auto target = ((wxWindow *)event.GetEventObject());
    if (!target)
        return;

    auto textCtrl = ((wxStyledTextCtrl *)FindWindowByName(
                         target->GetParent()->GetName() + "_textCtrl"));
    if (!textCtrl)
        return;

    auto icon = FindTabStateIcon(target);
    if (!icon)
        return;

    if (icon->GetLabel() == "saved_icon")
        return;

    auto closeIcon =
        UiUtils::CreateIcon(ApplicationPaths::GetIconPath("close.svg"));
    if (!closeIcon.IsOk())
        return;

    icon->SetBitmap(closeIcon);
    icon->SetLabel("saved_icon");

    Layout();
}

void Tabs::OnLeaveComp(wxMouseEvent &event) {
    auto target = ((wxWindow *)event.GetEventObject());
    if (!target)
        return;

    auto textCtrl = ((wxStyledTextCtrl *)FindWindowByName(
                         target->GetParent()->GetName() + "_textCtrl"));
    if (!textCtrl)
        return;

    auto icon = FindTabStateIcon(target);
    if (!icon)
        return;

    if (textCtrl->GetModify()) {
        if (icon->GetLabel() == "unsaved_icon")
            return;

        auto unsavedIcon = UiUtils::CreateIcon(
                               ApplicationPaths::GetIconPath("unsaved.svg"), wxSize(9, 9),
                               ThemesManager::Get().GetColor("unsavedIcon"));
        if (!unsavedIcon.IsOk())
            return;
        icon->SetBitmap(unsavedIcon);
        icon->SetLabel("unsaved_icon");
    } else {
        if (icon->GetLabel() == "saved_icon")
            return;

        auto closeIcon =
            UiUtils::CreateIcon(ApplicationPaths::GetIconPath("close.svg"));
        if (!closeIcon.IsOk())
            return;
        icon->SetBitmap(closeIcon);
        icon->SetLabel("saved_icon");
    }
    Layout();
}

void Tabs::OnTabPaint(wxPaintEvent &event) {
    auto *target = static_cast<wxPanel *>(event.GetEventObject());
    wxPaintDC dc(target);

    const bool isActive =
        (ProjectSettings::Get().GetCurrentlyFileOpen() == target->GetName());
    const wxColour bg =
        ThemesManager::Get().GetColor(isActive ? "secondary" : "main");
    const wxSize size = target->GetClientSize();

    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(bg));
    dc.DrawRectangle(wxPoint(0, 0), size);

    if (isActive) {
        dc.SetBrush(wxBrush(ThemesManager::Get().GetColor("highlight")));
        dc.DrawRectangle(wxPoint(0, 0), wxSize(size.GetWidth(), 1));
    }
}

void Tabs::OnPreviousTab() {
    const wxString current = ProjectSettings::Get().GetCurrentlyFileOpen();

    for (auto &&tab : tabsContainer->GetChildren()) {
        if (tab->GetName() != current)
            continue;

        if (auto prevTab = tab->GetPrevSibling()) {
            projectSettings.SetCurrentlyFileOpen(prevTab->GetName());
            Select();
        }
        return;
    }
}

void Tabs::OnNextTab() {
    const wxString current = ProjectSettings::Get().GetCurrentlyFileOpen();

    for (auto &&tab : tabsContainer->GetChildren()) {
        if (tab->GetName() != current)
            continue;

        if (auto nextTab = tab->GetNextSibling()) {
            projectSettings.SetCurrentlyFileOpen(nextTab->GetName());
            Select();
        }
        return;
    }
}