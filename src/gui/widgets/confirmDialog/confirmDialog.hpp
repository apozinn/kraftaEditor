/**
 * @file confirmDialog.hpp
 * @brief Declaration of the ConfirmDialog class, a dialog for user confirmation.
 *
 * This class represents a dialog box that displays a message and a checkbox for the user to confirm or not to be asked again in the future.
 * It is derived from the wxDialog class and provides a checkbox to allow the user to suppress future confirmation dialogs.
 *
 * @author Your Name
 * @version 1.0
 * @date 2022-01-01
 */

#pragma once

#include <wx/wx.h>

/**
 * @class ConfirmDialog
 * @brief A dialog for user confirmation.
 *
 * This class represents a dialog box that displays a message and a checkbox for the user to confirm or not to be asked again in the future.
 * It is derived from the wxDialog class and provides a checkbox to allow the user to suppress future confirmation dialogs.
 *
 * @author Your Name
 * @version 1.0
 * @date 2022-01-01
 */
class ConfirmDialog : public wxDialog
{
public:
    /**
     * @brief Constructor for the ConfirmDialog class.
     *
     * @param parent The parent window.
     * @param message The message to display in the dialog.
     * @param title The title of the dialog.
     */
    ConfirmDialog(wxWindow *parent, const wxString &message, const wxString &title);

    /**
     * @brief Checks if the user has opted not to be asked again in the future.
     *
     * @return True if the user has opted not to be asked again, false otherwise.
     */
    bool DontAskAgain() const;

private:
    /**
     * @brief The checkbox for the user to opt out of future confirmation dialogs.
     */
    wxCheckBox *m_checkBox;
};