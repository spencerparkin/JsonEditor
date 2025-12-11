#include "Frame.h"

Frame::Frame(const wxPoint& position, const wxSize& size) : wxFrame(nullptr, wxID_ANY, wxT("Json Editor"), position, size)
{
}

/*virtual*/ Frame::~Frame()
{
}