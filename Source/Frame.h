#pragma once

#include <wx/frame.h>
#include <wx/dataview.h>
#include "JsonDataViewModel.h"

/**
 * 
 */
class Frame : public wxFrame
{
public:
	Frame(const wxPoint& position, const wxSize& size);
	virtual ~Frame();

private:
	void OnLoadButtonPushed(wxCommandEvent& event);
	void OnSaveButtonPushed(wxCommandEvent& event);
	void OnLoadFromClipboardButtonPushed(wxCommandEvent& event);
	void OnSaveToClipboardButtonPushed(wxCommandEvent& event);

	wxDataViewCtrl* dataViewControl;
	JsonDataViewModel* dataViewModel;
	wxString filePath;
};