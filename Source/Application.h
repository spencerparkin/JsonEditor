#pragma once

#include <wx/app.h>

class Frame;

/**
 * This is the main application class for JsonEditor.
 */
class Application : public wxApp
{
public:
	Application();
	virtual ~Application();

	virtual bool OnInit() override;
	virtual int OnExit() override;

	Frame* GetFrame();

private:

	Frame* frame;
};

wxDECLARE_APP(Application);