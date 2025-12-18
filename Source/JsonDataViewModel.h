#pragma once

#include <wx/dataview.h>
#include <unordered_map>
#include "JsonValue.h"

#define COL_NAME			0
#define COL_TYPE			1
#define COL_VALUE			2

/**
 * This class acts as the glue between the JSON data we're editing and
 * the wxWidgets control that exposes it to the user.
 */
class JsonDataViewModel : public wxDataViewModel
{
public:
	JsonDataViewModel();
	virtual ~JsonDataViewModel();

	void SetJsonRootValue(std::shared_ptr<ParseParty::JsonValue> jsonRootValue);
	std::shared_ptr<ParseParty::JsonValue> GetJsonRootValue();

	virtual void GetValue(wxVariant& variant, const wxDataViewItem& item, unsigned int col) const override;
	virtual bool SetValue(const wxVariant& variant, const wxDataViewItem& item, unsigned int col) override;
	virtual wxDataViewItem GetParent(const wxDataViewItem& item) const override;
	virtual bool IsContainer(const wxDataViewItem& item) const override;
	virtual unsigned int GetChildren(const wxDataViewItem& item, wxDataViewItemArray& children) const override;
	virtual bool HasContainerColumns(const wxDataViewItem& item) const override;

private:
	struct JsonValueMetaData
	{
		std::string name;
		std::weak_ptr<ParseParty::JsonValue> jsonParentValueWeakPtr;
	};
	
	mutable std::unordered_map<ParseParty::JsonValue*, JsonValueMetaData> metaDataMap;
	std::shared_ptr<ParseParty::JsonValue> jsonRootValue;
};