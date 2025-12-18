#include "Frame.h"
#include "JsonValue.h"
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/file.h>
#include <wx/clipbrd.h>

using namespace ParseParty;

Frame::Frame(const wxPoint& position, const wxSize& size) : wxFrame(nullptr, wxID_ANY, wxT("Json Editor"), position, size)
{
	wxButton* loadButton = new wxButton(this, wxID_ANY, wxT("Load"));
	loadButton->Bind(wxEVT_BUTTON, &Frame::OnLoadButtonPushed, this);

	wxButton* saveButton = new wxButton(this, wxID_ANY, wxT("Save"));
	saveButton->Bind(wxEVT_BUTTON, &Frame::OnSaveButtonPushed, this);
	saveButton->SetToolTip("CTRL-click this button to save as some other file name.");

	wxButton* loadFromClipboardButton = new wxButton(this, wxID_ANY, wxT("Load From Clipboard"));
	loadFromClipboardButton->Bind(wxEVT_BUTTON, &Frame::OnLoadFromClipboardButtonPushed, this);

	wxButton* saveToClipboardButton = new wxButton(this, wxID_ANY, wxT("Save To Clipboard"));
	saveToClipboardButton->Bind(wxEVT_BUTTON, &Frame::OnSaveToClipboardButtonPushed, this);

	auto nameRenderer = new wxDataViewTextRenderer(wxDataViewTextRenderer::GetDefaultType(), wxDATAVIEW_CELL_EDITABLE, wxALIGN_LEFT);
	auto typeRenderer = new wxDataViewTextRenderer(wxDataViewTextRenderer::GetDefaultType(), wxDATAVIEW_CELL_INERT, wxALIGN_LEFT);
	auto valueRenderer = new wxDataViewTextRenderer(wxDataViewTextRenderer::GetDefaultType(), wxDATAVIEW_CELL_EDITABLE, wxALIGN_LEFT);

	auto nameColumn = new wxDataViewColumn(wxT("Name"), nameRenderer, COL_NAME, 250);
	auto typeColumn = new wxDataViewColumn(wxT("Type"), typeRenderer, COL_TYPE, 150);
	auto valueColumn = new wxDataViewColumn(wxT("Value"), valueRenderer, COL_VALUE, 250);

	// STPTODO: Add context menu that can be used to insert, delete, move, rename, etc., etc., etc.
	this->dataViewControl = new wxDataViewCtrl(this, wxID_ANY);
	this->dataViewControl->AppendColumn(nameColumn);
	this->dataViewControl->AppendColumn(typeColumn);
	this->dataViewControl->AppendColumn(valueColumn);

	this->dataViewModel = new JsonDataViewModel();
	this->dataViewControl->AssociateModel(this->dataViewModel);

	wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	buttonSizer->Add(loadButton, 0, wxGROW);
	buttonSizer->Add(saveButton, 0, wxGROW | wxLEFT, 5);
	buttonSizer->Add(loadFromClipboardButton, 0, wxGROW | wxLEFT, 5);
	buttonSizer->Add(saveToClipboardButton, 0, wxGROW | wxLEFT, 5);

	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(buttonSizer, 0, wxGROW | wxALL, 5);
	mainSizer->Add(this->dataViewControl, 1, wxGROW | wxLEFT | wxRIGHT | wxBOTTOM, 5);

	this->SetSizer(mainSizer);
}

/*virtual*/ Frame::~Frame()
{
	this->dataViewModel->DecRef();
}

void Frame::OnLoadButtonPushed(wxCommandEvent& event)
{
	wxFileDialog fileDialog(this, wxT("Select JSON file to open."), wxEmptyString, wxEmptyString, wxT("JSON Files (*.json)|*.json"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (fileDialog.ShowModal() != wxID_OK)
		return;

	wxString tentativeFilePath = fileDialog.GetPath();

	wxFile file;
	if (!file.Open(tentativeFilePath, wxFile::read))
	{
		wxMessageBox(wxT("Failed to open file: ") + tentativeFilePath, wxT("Error!"), wxOK | wxICON_ERROR, this);
		return;
	}

	wxString jsonText;
	if (!file.ReadAll(&jsonText))
	{
		wxMessageBox(wxT("Failed to read file: ") + tentativeFilePath, wxT("Error!"), wxOK | wxICON_ERROR, this);
		return;
	}

	file.Close();

	std::string parseError;
	std::shared_ptr<JsonValue> jsonValue = JsonValue::ParseJson(jsonText.ToStdString(), parseError);
	if (!jsonValue)
	{
		wxMessageBox(wxString::Format(wxT("Failed to parse json in file: %s\n\nError: %s"), tentativeFilePath.c_str(), parseError.c_str()), wxT("Error!"), wxOK | wxICON_ERROR, this);
		return;
	}

	this->dataViewModel->SetJsonRootValue(jsonValue);
	this->dataViewModel->Cleared();
	this->filePath = tentativeFilePath;
}

void Frame::OnSaveButtonPushed(wxCommandEvent& event)
{
	JsonValue* jsonValue = this->dataViewModel->GetJsonRootValue().get();
	if (!jsonValue)
	{
		wxMessageBox(wxT("No JSON loaded that can be saved."), wxT("Error!"), wxOK | wxICON_ERROR, this);
		return;
	}

	if (this->filePath.length() == 0 || wxGetKeyState(WXK_CONTROL))
	{
		wxFileDialog fileDialog(this, wxT("Select JSON file save location."), wxEmptyString, wxEmptyString, wxT("JSON Files (*.json)|*.json"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		if (fileDialog.ShowModal() != wxID_OK)
			return;

		this->filePath = fileDialog.GetPath();
	}

	std::string jsonText;
	if (!jsonValue->PrintJson(jsonText))
	{
		wxMessageBox(wxT("Json print failed!?"), wxT("Error!"), wxOK | wxICON_ERROR, this);
		return;
	}

	wxFile file;
	if (!file.Open(this->filePath, wxFile::write))
	{
		wxMessageBox(wxString::Format(wxT("Failed to open file for writing: %s"), this->filePath.c_str()), wxT("Error!"), wxOK | wxICON_ERROR, this);
		return;
	}

	if (!file.Write(wxString(jsonText)))
	{
		wxMessageBox(wxString::Format(wxT("Failed to write to file: %s"), this->filePath.c_str()), wxT("Error!"), wxOK | wxICON_ERROR, this);
		return;
	}

	file.Close();
}

void Frame::OnLoadFromClipboardButtonPushed(wxCommandEvent& event)
{
	if (!wxTheClipboard->Open())
	{
		wxMessageBox(wxT("Failed to open clipboard!"), wxT("Error!"), wxOK | wxICON_ERROR, this);
		return;
	}

	wxTextDataObject dataObject;
	if (!wxTheClipboard->GetData(dataObject))
	{
		wxMessageBox(wxT("Failed to get text object from clipboard!"), wxT("Error!"), wxOK | wxICON_ERROR, this);
		return;
	}

	wxTheClipboard->Close();

	wxString jsonText = dataObject.GetText();
	std::string parseError;
	std::shared_ptr<JsonValue> jsonValue = JsonValue::ParseJson(jsonText.ToStdString(), parseError);
	if (!jsonValue)
	{
		wxMessageBox(wxString::Format(wxT("Failed to parse JSON from clipboard.\n\nError: %s"), parseError.c_str()), wxT("Error!"), wxOK | wxICON_ERROR, this);
		return;
	}

	this->dataViewModel->SetJsonRootValue(jsonValue);
	this->dataViewModel->Cleared();
	this->filePath = "";
}

void Frame::OnSaveToClipboardButtonPushed(wxCommandEvent& event)
{
	JsonValue* jsonValue = this->dataViewModel->GetJsonRootValue().get();
	if (!jsonValue)
	{
		wxMessageBox(wxT("No JSON loaded that can be saved."), wxT("Error!"), wxOK | wxICON_ERROR, this);
		return;
	}

	std::string jsonText;
	if (!jsonValue->PrintJson(jsonText))
	{
		wxMessageBox(wxT("Json print failed!?"), wxT("Error!"), wxOK | wxICON_ERROR, this);
		return;
	}

	if (!wxTheClipboard->Open())
	{
		wxMessageBox(wxT("Failed to open clipboard!"), wxT("Error!"), wxOK | wxICON_ERROR, this);
		return;
	}

	wxTextDataObject dataObject;
	dataObject.SetText(wxString(jsonText));
	if (!wxTheClipboard->SetData(&dataObject))
	{
		wxMessageBox(wxT("Failed to set text object to clipboard!"), wxT("Error!"), wxOK | wxICON_ERROR, this);
		return;
	}

	wxTheClipboard->Close();
}