#include "filesTree.hpp"
#include "ui/ids.hpp"

wxBEGIN_EVENT_TABLE(FilesTree, wxPanel)
	EVT_MENU(+Event::File::RenameFile, FilesTree::OnRenameFileRequested)
	EVT_MENU(+Event::File::DeleteFileEvent, FilesTree::OnDeleteFileRequested)
	EVT_MENU(+Event::File::RenameDir, FilesTree::OnRenameDirRequested)
	EVT_MENU(+Event::File::DeleteDir, FilesTree::OnDeleteDirRequested)
	EVT_PAINT(FilesTree::OnPaint)
wxEND_EVENT_TABLE()