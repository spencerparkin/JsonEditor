#include "Frame.h"
#include "JsonValue.h"
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/file.h>

using namespace ParseParty;

Frame::Frame(const wxPoint& position, const wxSize& size) : wxFrame(nullptr, wxID_ANY, wxT("Json Editor"), position, size)
{
	wxButton* loadButton = new wxButton(this, wxID_ANY, wxT("Load"));
	loadButton->Bind(wxEVT_BUTTON, &Frame::OnLoadButtonPushed, this);

	wxButton* saveButton = new wxButton(this, wxID_ANY, wxT("Save"));
	saveButton->Bind(wxEVT_BUTTON, &Frame::OnSaveButtonPushed, this);

	auto nameRenderer = new wxDataViewTextRenderer(wxDataViewTextRenderer::GetDefaultType(), wxDATAVIEW_CELL_EDITABLE, wxALIGN_LEFT);
	auto typeRenderer = new wxDataViewTextRenderer(wxDataViewTextRenderer::GetDefaultType(), wxDATAVIEW_CELL_EDITABLE, wxALIGN_LEFT);
	auto valueRenderer = new wxDataViewTextRenderer(wxDataViewTextRenderer::GetDefaultType(), wxDATAVIEW_CELL_EDITABLE, wxALIGN_LEFT);

	auto nameColumn = new wxDataViewColumn(wxT("Name"), nameRenderer, COL_NAME, 250);
	auto typeColumn = new wxDataViewColumn(wxT("Type"), typeRenderer, COL_TYPE, 150);
	auto valueColumn = new wxDataViewColumn(wxT("Value"), valueRenderer, COL_VALUE, 250);

	this->dataViewControl = new wxDataViewCtrl(this, wxID_ANY);
	this->dataViewControl->AppendColumn(nameColumn);
	this->dataViewControl->AppendColumn(typeColumn);
	this->dataViewControl->AppendColumn(valueColumn);

	this->dataViewModel = new JsonDataViewModel();
	this->dataViewControl->AssociateModel(this->dataViewModel);

	wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	buttonSizer->Add(loadButton, 0, wxGROW);
	buttonSizer->Add(saveButton, 0, wxGROW | wxLEFT, 5);

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
	wxFileDialog fileDialog(this, wxT("Select JSON file to open."), wxEmptyString, wxEmptyString, wxT("Json Files (*.json)|*.json"), wxFD_FILE_MUST_EXIST);
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
	JsonValue* jsonValue = JsonValue::ParseJson(jsonText.ToStdString(), parseError);
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
}