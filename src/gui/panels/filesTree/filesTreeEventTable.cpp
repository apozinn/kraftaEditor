#include "filesTree.hpp"

wxBEGIN_EVENT_TABLE(FilesTree, wxPanel)
	EVT_MENU(+Event::File::RenameFile, FilesTree::OnFileRename)
	EVT_MENU(+Event::File::DeleteFileEvent, FilesTree::OnDeleteFile)
	EVT_MENU(+Event::File::RenameDir, FilesTree::OnDirRename)
	EVT_MENU(+Event::File::DeleteDir, FilesTree::OnDeleteDir)
wxEND_EVENT_TABLE()