#include "guiHelpers.hpp"

// InputField
InputField::InputField(wxWindow* parent, const wxString& label, int labelWidth,
                       const wxString& fieldValue, int fieldWidth)
    : wxPanel(parent, wxID_ANY) {
  auto horizontalLayout = new wxBoxSizer(wxHORIZONTAL);

  this->label = new wxStaticText(this, wxID_ANY, label, wxDefaultPosition,
                                 wxSize(labelWidth, -1));
  horizontalLayout->Add(this->label, 0, wxALIGN_CENTER);

  this->textCtrl = new wxTextCtrl(this, wxID_ANY, fieldValue, wxDefaultPosition,
                                  wxSize(fieldWidth, -1));
  this->textCtrl->SetBackgroundColour(wxColour(240, 212, 175));
  horizontalLayout->Add(this->textCtrl, 1, wxEXPAND);

  this->SetSizerAndFit(horizontalLayout);
}

wxString InputField::getValue() { return this->textCtrl->GetValue(); }

// ImagePanel
ImagePanel::ImagePanel(wxWindow* parent, wxString const& file,
                       wxBitmapType format, wxPoint position, wxSize size,
                       double rotation)
    : wxPanel(parent, wxID_ANY, position, size) {
  if (!wxFileExists(file)) {
    wxMessageBox("Could not find file: " + file, "File error", wxICON_ERROR);
    return;
  }

  if (!this->_image.LoadFile(file, format)) {
    wxMessageBox("Could not load file: " + file, "File error", wxICON_ERROR);
    return;
  }

  this->_rotation = rotation;
  this->_width = -1;
  this->_height = -1;

  this->Bind(wxEVT_PAINT, &ImagePanel::paintEvent, this);
  this->Bind(wxEVT_SIZE, &ImagePanel::onSize, this);
}

void ImagePanel::paintEvent(wxPaintEvent& event) {
  if (!this->_image.IsOk()) {
    wxMessageBox("Image not loaded correctly.", "Error", wxICON_ERROR);
    return;
  }

  wxPaintDC deviceContext = wxPaintDC(this);

  int newWidth;
  int newHeight;
  deviceContext.GetSize(&newWidth, &newHeight);

  if (newWidth != this->_width || newHeight != this->_height) {
    wxImage transformed;

    if (this->_rotation == 0.0) {
      transformed =
          this->_image.Scale(newWidth, newHeight, wxIMAGE_QUALITY_BILINEAR);
    } else {
      wxPoint centerOfRotation =
          wxPoint(this->_image.GetWidth() / 2, this->_image.GetHeight() / 2);
      transformed =
          this->_image.Rotate(this->_rotation, centerOfRotation, true);
      transformed =
          transformed.Scale(newWidth, newHeight, wxIMAGE_QUALITY_BILINEAR);
    }
    this->_bitmap = wxBitmap(transformed);
    this->_width = transformed.GetWidth();
    this->_height = transformed.GetHeight();

    deviceContext.DrawBitmap(this->_bitmap, 0, 0, true);
  } else {
    deviceContext.DrawBitmap(this->_bitmap, 0, 0, true);
  }
}

void ImagePanel::onSize(wxSizeEvent& event) {
  // when the panel is resized, we need to repaint it
  this->Refresh();  // Trigger a repaint
  event.Skip();     // Allow the default handling of the size event
}
