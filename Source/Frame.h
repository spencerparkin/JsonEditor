#pragma once

#include <wx/frame.h>

/**
 * 
 */
class Frame : public wxFrame
{
public:
	Frame(const wxPoint& position, const wxSize& size);
	virtual ~Frame();
};