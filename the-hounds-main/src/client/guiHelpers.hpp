#ifndef GUIHELPERS_HPP
#define GUIHELPERS_HPP

#include <wx/wx.h>

class InputField : public wxPanel {
 public:
  /**
   * @brief Constructs an input field with a label.
   * @param parent The parent window for this input field.
   * @param label The label for the input field.
   * @param labelWidth The width of the label.
   * @param fieldValue The default value of the input filed.
   * @param fieldWidth The width of the input field.
   */
  InputField(wxWindow* parent, const wxString& label, int labelWidth,
             const wxString& fieldValue, int fieldWidth);

  /**
   * @brief Return the values inside the input field.
   * @return wxString with the value of the InputField
   */
  wxString getValue();

 private:
  wxStaticText* label;   ///< Label for the input field.
  wxTextCtrl* textCtrl;  ///< Text control for user input.
};

/**
 * @class ImagePanel
 * @brief Panel for displaying an image with optional rotation.
 * This panel can be used to display images in the GUI, such as game cards.
 */
class ImagePanel : public wxPanel {
 public:
  /**
   * @brief Constructs an image panel with a specified image.
   * @param parent The parent window for this panel.
   * @param file The path to the image file.
   * @param format The format of the image (e.g., PNG).
   * @param position The position of the panel.
   * @param size The size of the panel.
   * @param rotation The rotation angle for the image.
   */
  ImagePanel(wxWindow* parent, wxString const& file, wxBitmapType format,
             wxPoint position, wxSize size, double rotation = 0.0);

  /**
   * @brief Handles the paint event for the image panel, drawing the image onto
   * the panel. This function checks if the image is valid and scales or rotates
   * it accordingly before drawing it to the screen. If the image dimensions
   * have changed, it will apply a transformation to match the new dimensions.
   * Rotation is performed around the image's center if a non-zero rotation
   * angle is specified.
   * @param event The wxPaintEvent that triggered the paint.
   * @warning If the image has not been loaded correctly, an error message is
   * shown to the user and the paint operation is aborted.
   */
  void paintEvent(wxPaintEvent& event);
  /**
   * @brief Handles the resize event for the image panel.
   *
   * This function triggers a repaint of the panel when its size changes,
   * ensuring that the image is rendered correctly after the panel's dimensions
   * are updated.
   *
   * @param event The wxSizeEvent that triggered the resize event.
   *
   * @note The function calls `Refresh()` to request a redraw of the panel and
   * then skips the default event handling by calling `event.Skip()`.
   */
  void onSize(wxSizeEvent& event);

 private:
  wxImage _image;    ///< Image to hold the original image.
  wxBitmap _bitmap;  ///< Bitmap to hold the image.

  double _rotation;  ///< Rotation angle for the image.
  int _width;        ///< Width of the panel.
  int _height;       ///< Height of the panel.
};

#endif  // GUIHELPERS_HPP
