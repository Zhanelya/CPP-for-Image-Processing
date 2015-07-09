/*** Zhanelya Subebayeva ***/
#include <vector>
class BasicApplication : public wxApp {
 public:
    virtual bool OnInit();
};


class MyFrame : public wxFrame {    
 protected:
    wxMenuBar  *menuBar;//main menu bar
    wxMenu     *fileMenu;//file menu
    wxBitmap *back_bitmap; // offscreen memory buffer for drawing
    wxToolBar *toolbar;//tollbar not necessary to use
    int oldWidth, oldHeight; // save old dimensions

    wxBitmap bitmap;  //structure for the edited image
    wxImage *loadedImage; // image loaded from file
    int imgWidth, imgHeight; // image end dimensions
    int startWidth, startHeight; // image start dimensions
    int stuffToDraw;    
    bool roi;

    std::vector<wxImage> imgVector;//for undo function

   /* declear message handler */
    void OnROIOn(wxCommandEvent & event);
    void appendToUndo(wxImage img);
    void OnUndo(wxCommandEvent & event);
    void OnInvertImage(wxCommandEvent & event);
    void OnScaleImage(wxCommandEvent & event);
    void OnShiftImage(wxCommandEvent & event);
    void OnSaveImage(wxCommandEvent & event);
    void OnOpenRawFile(wxCommandEvent & event); 
    void OnConvoluteImage(wxCommandEvent & event);
    void OnOSFImage(wxCommandEvent & event);
    void OnPointProcessingImage(wxCommandEvent & event);
    void OnThresholdingImage(wxCommandEvent & event);
    void OnHistogramEqualisationImage(wxCommandEvent & event);
    void OnHistogramDisplayImage(wxCommandEvent & event);//---> To be modified! 
 public:
    MyFrame(const wxString title, int xpos, int ypos, int width, int height);
    virtual ~MyFrame();
    
    void OnExit(wxCommandEvent & event);       
    void OnOpenFile(wxCommandEvent & event);       
    void OnPaint(wxPaintEvent & event);	
    
    DECLARE_EVENT_TABLE()
	
};

enum { NOTHING = 0,
       ORIGINAL_IMG,
};

enum { EXIT_ID = wxID_HIGHEST + 1,      
       LOAD_FILE_ID,
       INVERT_IMAGE_ID,
       SCALE_IMAGE_ID,
       SHIFT_IMAGE_ID,
       SAVE_IMAGE_ID,
       LOAD_RAW_ID,
       CONVOLUTE_IMAGE_ID,
       OSF_IMAGE_ID, 
       POINT_PROCESSING_IMAGE_ID, 
       THRESHOLDING_IMAGE_ID, 
       HISTOGRAM_EQUALISATION_IMAGE_ID, 
       HISTOGRAM_DISPLAY_IMAGE_ID,
       ROI_ON_ID,
       UNDO_ID,//--->To be modified!
};

int fsize(FILE *fp){
    int prev=ftell(fp);
    fseek(fp, 0L, SEEK_END);
    int sz=ftell(fp);
    fseek(fp,prev,SEEK_SET); //go back to where we were
    return sz;
}
