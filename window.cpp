/*** Zhanelya Subebayeva ***/
#include <wx/wx.h>
#include "window.h"
#include <wx/image.h>
#include <wx/dcmemory.h>
#include <wx/dcclient.h>
#include <wx/dc.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <iostream>
using namespace std; 

static const wxChar *FILETYPES = _T("All files (*.*)|*.*");

IMPLEMENT_APP(BasicApplication)

bool BasicApplication::OnInit()
{
  wxInitAllImageHandlers();
  MyFrame *frame = new MyFrame(_("Basic Frame"), 50, 50, 700, 400);
  frame->Show(TRUE);
  SetTopWindow(frame);
  return TRUE;	
}

MyFrame::MyFrame(const wxString title, int xpos, int ypos, int width, int height): wxFrame((wxFrame *) NULL, -1, title, wxPoint(xpos, ypos), wxSize(width, height)){

  fileMenu = new wxMenu;
  fileMenu->Append(LOAD_FILE_ID, _T("&Open file"));  
  fileMenu->Append(LOAD_RAW_ID, _T("&Open raw file")); //--->To be modified!
  fileMenu->AppendSeparator();
//###########################################################//
//----------------------START MY MENU -----------------------//
//###########################################################// 
  fileMenu->Append( ROI_ON_ID, _T("&Select ROI"));
  fileMenu->AppendSeparator();
  fileMenu->Append( UNDO_ID, _T("&Undo"));
  fileMenu->AppendSeparator();
  fileMenu->Append(INVERT_IMAGE_ID, _T("&Invert image"));
  fileMenu->Append(SCALE_IMAGE_ID, _T("&Scale image"));
  fileMenu->Append(SHIFT_IMAGE_ID, _T("&Shift image"));
  fileMenu->Append(CONVOLUTE_IMAGE_ID, _T("&Convolution"));
  fileMenu->Append(OSF_IMAGE_ID, _T("&Order statistic filtering (and noise)"));
  fileMenu->Append(POINT_PROCESSING_IMAGE_ID, _T("&Point processing"));
  fileMenu->Append(THRESHOLDING_IMAGE_ID, _T("&Thresholding"));
  fileMenu->Append(HISTOGRAM_EQUALISATION_IMAGE_ID, _T("&Histogram equalisation"));
  fileMenu->Append(HISTOGRAM_DISPLAY_IMAGE_ID, _T("&Histogram display"));
//###########################################################//
//----------------------END MY MENU -------------------------//
//###########################################################// 

  fileMenu->AppendSeparator();
  fileMenu->Append(SAVE_IMAGE_ID, _T("&Save image"));
  fileMenu->Append(EXIT_ID, _T("E&xit"));

  menuBar = new wxMenuBar;
  menuBar->Append(fileMenu, _T("&File"));
  
  SetMenuBar(menuBar);
  CreateStatusBar(3); 
  oldWidth = 0;
  oldHeight = 0;
  loadedImage = 0;

/* initialise the variables that we added */
  imgWidth = imgHeight = 0;
  startWidth = startHeight = 0;
  stuffToDraw = 0;

}

MyFrame::~MyFrame(){
/* release resources */
  if(loadedImage){
    loadedImage->Destroy();
    loadedImage = 0;
  }

}

void MyFrame::OnOpenFile(wxCommandEvent & event){
  wxFileDialog *openFileDialog = new wxFileDialog ( this, _T("Open file"), _T(""), _T(""), FILETYPES, wxOPEN, wxDefaultPosition);  
  if(openFileDialog->ShowModal() == wxID_OK){
    wxString filename = openFileDialog->GetFilename();
    wxString path = openFileDialog->GetPath();
    printf("Loading image form file...");    
    loadedImage = new wxImage(path); //Image Loaded form file 
    if(loadedImage->Ok()){
      stuffToDraw = ORIGINAL_IMG;    // set the display flag
      printf("Done! \n");    
    }
    else {
      printf("error:...");
      loadedImage->Destroy();
      loadedImage = 0;
    }
    Refresh();
  }    
}


//###########################################################//
//-----------------------------------------------------------//
//---------- DO NOT MODIFY THE CODE ABOVE--------------------//
//-----------------------------------------------------------//
//###########################################################//

//###########################################################//
//-----------------------------------------------------------//
//---------------------- SELECT ROI -------------------------//
//-----------------------------------------------------------//
//###########################################################//

void MyFrame::OnROIOn(wxCommandEvent & event){
  int i0,iMax,j0,jMax;

	wxString startX = wxGetTextFromUser(wxT("Enter x start"), wxT("start x"));
	i0 = wxAtoi(startX);
	
	wxString endX = wxGetTextFromUser(wxT("Enter x end"), wxT("end x"));
	iMax = wxAtoi(endX);
	
	wxString startY = wxGetTextFromUser(wxT("Enter y start"), wxT("start y"));
	j0 = wxAtoi(startY);
	
	wxString endY = wxGetTextFromUser(wxT("Enter y end"), wxT("end y"));
	jMax = wxAtoi(endY);

	startWidth = i0;
	startHeight = j0;
	imgWidth = iMax;
	imgHeight = jMax;
	printf("ROI ADDED\n");

}

//###########################################################//
//-----------------------------------------------------------//
//--------------------- UNDO FUNCTION -----------------------//
//-----------------------------------------------------------//
//###########################################################//

void MyFrame::appendToUndo(wxImage img){
	imgVector.push_back(img);
}
void MyFrame::OnUndo(wxCommandEvent & event){
	if(imgVector.size()>0){
		printf("Undo...");
		free(loadedImage);

		loadedImage = new wxImage(bitmap.ConvertToImage());
		wxImage img = imgVector.back();
		imgVector.pop_back();

		for(int i = 0; i < imgWidth; i++)
	   	  for(int j = 0; j < imgHeight; j++){
			loadedImage->SetRGB(i,j, img.GetRed(i,j), img.GetGreen(i,j), img.GetBlue(i,j));
		  }
		printf("Finish undo.\n");
		Refresh();
	}
}

//INVERT IMAGE
void MyFrame::OnInvertImage(wxCommandEvent & event){
  
    printf("Inverting...");
    free(loadedImage);
    loadedImage = new wxImage(bitmap.ConvertToImage());
    appendToUndo(*loadedImage); 
    for( int i=startWidth;i<imgWidth;i++)
       for(int j=startHeight;j<imgHeight;j++){
 	loadedImage->SetRGB(i,j,255-loadedImage->GetRed(i,j), 
				255-loadedImage->GetGreen(i,j),
				255-loadedImage->GetBlue(i,j));
    }

    printf(" Finished inverting.\n");
    Refresh();
}

//###########################################################//
//-----------------------------------------------------------//
//---------------------- LOADING RAW-------------------------//
//-----------------------------------------------------------//
//###########################################################//

void MyFrame::OnOpenRawFile(wxCommandEvent & event){
  int size;
  free(loadedImage);
  wxFileDialog *openFileDialog = new wxFileDialog ( this, _T("Open file"), _T(""), _T(""), FILETYPES, wxOPEN, wxDefaultPosition);  
  if(openFileDialog->ShowModal() == wxID_OK){
      wxString filename = openFileDialog->GetFilename();
      wxString path = openFileDialog->GetPath();
      printf("Loading raw image...");

      FILE * fp = fopen( path.mb_str(), "rb" );
      size = (int)sqrt((double)fsize(fp));
      printf("Size is %d...",size);
      if (size*size == fsize(fp)){
	unsigned char imgArray[size][size];
        fread(&imgArray,sizeof(char),size*size,fp);

	    loadedImage = new wxImage(size,size);

	    unsigned char r,g,b;

	    for( int i=0;i<size;i++)
	       for(int j=0;j<size;j++){
		// GET THE RGB VALUES 	
		r = imgArray[j][i];
		g = imgArray[j][i];
		b = imgArray[j][i];
		
		// SAVE THE RGB VALUES
		loadedImage->SetRGB(i,j,r,g,b); 
	    }
	
	stuffToDraw = ORIGINAL_IMG;
	printf("Done.\n");
        Refresh();
	
      }else{
        printf("error:...");
      }
      fclose(fp);
      
  }
}

//###########################################################//
//-----------------------------------------------------------//
//------------------- IMAGE SCALEING-------------------------//
//-----------------------------------------------------------//
//###########################################################//

void MyFrame::OnScaleImage(wxCommandEvent & event){
    double n;
    wxString number = wxGetTextFromUser(_T("Please enter the re-scaling factor.\nThe re-scaling factors should be floating-point numbers,\nwhich can be from 0 to 2, e.g. 0.5, 0.7, 1.3 and 2. "), _T(""), _T(""));
    if(number.ToDouble(&n) && n<=2 && n>=0){
	    int r,g,b;
	    printf("Scaling...");
	    free(loadedImage);
	    loadedImage = new wxImage(bitmap.ConvertToImage());
	    appendToUndo(*loadedImage); 
	    for( int i=startWidth;i<imgWidth;i++)
       	      for(int j=startHeight;j<imgHeight;j++){
		r = (int)round(n* loadedImage->GetRed(i,j));
		g = (int)round(n* loadedImage->GetGreen(i,j));
		b = (int)round(n* loadedImage->GetBlue(i,j));

		//if a value is greater than 255, just set it 255.		
		if(r>255){r=255;}
		if(g>255){g=255;}
		if(b>255){b=255;}
		
		//if a value is less than 0, just set it 0; 
		if(r<0){r=0;}
		if(g<0){g=0;}
		if(b<0){b=0;}
		
	 	loadedImage->SetRGB(i,j,r, g, b);
		
	      }
	    printf(" Finished scaling.\n");
	    Refresh();
    }else{
	    wxMessageBox(_T("The number that you entered is in wrong format.\nPlease try again."));
    }
}

//###########################################################//
//-----------------------------------------------------------//
//------------------- IMAGE SHIFTING-------------------------//
//-----------------------------------------------------------//
//###########################################################//

void MyFrame::OnShiftImage(wxCommandEvent & event){
    long int n;
    wxString number = wxGetTextFromUser(_T("Please enter the shift.\nThe shift can be a negative or a positive integer ranging from –255 to 255. "), _T(""), _T(""));
    if(number.ToLong(&n) && n>=(-255) && n<=255){
	    int r,g,b;
	    printf("Shifting...");
	    free(loadedImage);
	    loadedImage = new wxImage(bitmap.ConvertToImage());
	    appendToUndo(*loadedImage); 
	    for( int i=startWidth;i<imgWidth;i++)
              for(int j=startHeight;j<imgHeight;j++){
		
		r = (int)loadedImage->GetRed(i,j)+n;
		g = (int)loadedImage->GetGreen(i,j)+n;
		b = (int)loadedImage->GetBlue(i,j)+n;

		//if a value is greater than 255, just set it 255.		
		if(r>255){r=255;}
		if(g>255){g=255;}
		if(b>255){b=255;}
		
		//if a value is less than 0, just set it 0; 
		if(r<0){r=0;}
		if(g<0){g=0;}
		if(b<0){b=0;}
		
	 	loadedImage->SetRGB(i,j,r, g, b);
		
	      }
	    printf(" Finished shifting.\n");
	    Refresh();
    }else{
	    wxMessageBox(_T("The number that you entered is in wrong format.\nPlease try again."));
    }
}

//###########################################################//
//-----------------------------------------------------------//
//------------------- IMAGE CONVOLUTION----------------------//
//-----------------------------------------------------------//
//###########################################################//

void MyFrame::OnConvoluteImage(wxCommandEvent & event){
    int err=0;
    long int k;//for size of matrix switch
    long int n;//for external switch
    long int m;//for internal switch
    float mask33[3][3];
    float mask55[5][5];

    float sr=0,sg=0,sb=0;
free(loadedImage);
loadedImage = new wxImage(bitmap.ConvertToImage());
appendToUndo(*loadedImage); 

wxImage *newImage  = new wxImage((bitmap.ConvertToImage()).GetWidth(),(bitmap.ConvertToImage()).GetHeight(),true);

wxString number = wxGetTextFromUser(_T("Please choose the matrix size for convolution.\n1  3x3 \n2  5x5"), _T(""), _T(""));
if(number.ToLong(&k)&&(k==1||k==2)){
switch(k){
case 1:
	printf("3x3 Filtering\n");
	/* 3x3 matrix*/
	    number = wxGetTextFromUser(_T("Please choose the convolution.\n1  Smoothing the image \n2  Edge detection in the image"), _T(""), _T(""));
	    if(number.ToLong(&n)&&(n==1||n==2)){
		printf("Convolution...");
		
		switch(n){
		        case 1:
	/******************** IMAGE SMOOTING ***********************/
				printf("Smoothing...\n");
				number = wxGetTextFromUser(_T("Please choose the matrix.\n1  1/9*{{1,1,1},{1,1,1},{1,1,1}} \n2  1/16*{{1,2,1},{2,4,2},{1,2,1}} "), _T(""), _T(""));
				if(number.ToLong(&m)&&(m==1||m==2)){
					switch(m){
						//Averaging
						case 1:
							printf("3x3. Case1\n");
							for(int i=0;i<3;i++){
								for(int j=0;j<3;j++){
									mask33[i][j]=(float)1/9;
									printf("%f ",mask33[i][j]*9);						
								}	
								printf("\n");				
							}
							break;
						//Weighted Averaging
						case 2:
							printf("3x3. Case2\n");
							for(int i=0;i<3;i++){
								for(int j=0;j<3;j++){
									if(i==1&&j==1){
										mask33[i][j]=(float)4/16;
									}else if(j==1||i==1){
										mask33[i][j]=(float)2/16;
									}else{
										mask33[i][j]=(float)1/16;
									}
									printf("%f ",mask33[i][j]*16);						
								}	
								printf("\n");				
							}
							break;
						default:
							wxMessageBox(_T("The number that you entered is in wrong format.\nPlease try again."));
							err=-1;
					}
				}else{
					wxMessageBox(_T("The number that you entered is in wrong format.\nPlease try again."));		
					err=-1;	
				}
				break;
			case 2:
	/******************** EDGE DETECTION ***********************/
				printf("Edge Detection...\n");
				number = wxGetTextFromUser(_T("Please choose the method.\n1  4-neighbour Laplacian \n2  8-neighbour Laplacian \n3  4-neighbour Laplacian Enhancement\n4  8-neighbour Laplacian Enhancement\n5  Roberts1 {{0,0,0},{0,0,-1},{0,1,0}} \n6  Roberts2 {{0,0,0},{0,-1,0},{0,0,1}}\n7  SobelX\n8  SobelY  "), _T(""), _T(""));
				if(number.ToLong(&m)&&(m==1||m==2||m==3||m==4||m==5||m==6||m==7||m==8)){
					switch(m){
						//4-neighbour Laplacian
						case 1:
							printf("3x3. Case1\n");
							for(int i=0;i<3;i++){
								for(int j=0;j<3;j++){
									if(i==1&&j==1){
										mask33[i][j]=(float)(4);
									}else if(j==1||i==1){
										mask33[i][j]=(float)(-1);
									}else{
										mask33[i][j]=(float)0;
									}
									printf("%f ",mask33[i][j]);						
								}	
								printf("\n");					
							}
							break;
						//8-neighbour Laplacian
						case 2:
							printf("3x3. Case2\n");
							for(int i=0;i<3;i++){
								for(int j=0;j<3;j++){
									if(i==1&&j==1){
										mask33[i][j]=(float)8;
									}else if(j==1||i==1){
										mask33[i][j]=(float)(-1);
									}else{
										mask33[i][j]=(float)(-1);
									}
									printf("%f ",mask33[i][j]);						
								}	
								printf("\n");				
							}
							break;
						//4-neighbour Laplacian Enhancement
						case 3:
							printf("3x3. Case3\n");
							for(int i=0;i<3;i++){
								for(int j=0;j<3;j++){
									if(i==1&&j==1){
										mask33[i][j]=(float)5;
									}else if(j==1||i==1){
										mask33[i][j]=(float)(-1);
									}else{
										mask33[i][j]=(float)0;
									}
									printf("%f ",mask33[i][j]);						
								}		
								printf("\n");				
							}
							break;
						//8-neighbour Laplacian Enhancement
						case 4:
							printf("3x3. Case3\n");
							for(int i=0;i<3;i++){
								for(int j=0;j<3;j++){
									if(i==1&&j==1){
										mask33[i][j]=(float)9;
									}else if(j==1||i==1){
										mask33[i][j]=(float)(-1);
									}else{
										mask33[i][j]=(float)(-1);
									}
									printf("%f ",mask33[i][j]);						
								}					
								printf("\n");
							}
							break;
						//Roberts1 {{0,0,0},{0,0,-1},{0,1,0}} 
						case 5:
							printf("3x3. Case3\n");
							for(int i=0;i<3;i++){
								for(int j=0;j<3;j++){
									if(i==1&&j==2){
										mask33[i][j]=(float)(-1);
									}else if(i==2&&j==1){
										mask33[i][j]=(float)1;
									}else{
										mask33[i][j]=(float)0;
									}
									printf("%f ",mask33[i][j]);						
								}		
								printf("\n");				
							}
							break;
						//Roberts2 {{0,0,0},{0,-1,0},{0,0,1}}
						case 6:
							printf("3x3. Case3\n");
							for(int i=0;i<3;i++){
								for(int j=0;j<3;j++){
									if(i==1&&j==1){
										mask33[i][j]=(float)(-1);
									}else if(i==2&&j==2){
										mask33[i][j]=(float)1;
									}else{
										mask33[i][j]=(float)0;
									}
									printf("%f ",mask33[i][j]);						
								}		
								printf("\n");				
							}
							break;
						//SobelX
						case 7:
							printf("3x3. Case3\n");
							for(int i=0;i<3;i++){
								for(int j=0;j<3;j++){
									if((i==0&&j==0)||(i==2&&j==0)){
										mask33[i][j]=(float)(-1);
									}else if(i==1&&j==0){
										mask33[i][j]=(float)(-2);
									}else if((i==0&&j==2)||(i==2&&j==2)){
										mask33[i][j]=(float)1;
									}else if(i==1&&j==2){
										mask33[i][j]=(float)2;
									}else{
										mask33[i][j]=(float)0;
									}
									printf("%f ",mask33[i][j]);						
								}		
								printf("\n");				
							}
							break;
						//SobelY
						case 8:
							printf("3x3. Case3\n");
							for(int i=0;i<3;i++){
								for(int j=0;j<3;j++){
									if((i==0&&j==0)||(i==0&&j==2)){
										mask33[i][j]=(float)(-1);
									}else if(i==0&&j==1){
										mask33[i][j]=(float)(-2);
									}else if((i==2&&j==0)||(i==2&&j==2)){
										mask33[i][j]=(float)1;
									}else if(i==2&&j==1){
										mask33[i][j]=(float)2;
									}else{
										mask33[i][j]=(float)0;
									}
									printf("%f ",mask33[i][j]);						
								}	
								printf("\n");				
							}
							break;
						default:
							wxMessageBox(_T("The number that you entered is in wrong format.\nPlease try again."));
							err=-1;
					}
				}else{
					wxMessageBox(_T("The number that you entered is in wrong format.\nPlease try again."));	
					err=-1;		
				}
				break;
			default:
				wxMessageBox(_T("The number that you entered is in wrong format.\nPlease try again."));	
				err=-1;
		}
		if(err==0){
			//Iterate over image pixel values and apply the mask
			for( int i=startWidth;i<imgWidth;i++)
              		   for(int j=startHeight;j<imgHeight;j++){
				sr=0;
				sg=0;
				sb=0;
				for(int x=-1;x<=1;x++){
					for(int y=-1;y<=1;y++){
						/* Handle corners and borders */
						//handle top left corner
						if(i==0&&j==0){
							if((x==-1&&y==-1)||(x==0&&y==-1)||(y==0&&x==-1)){
								sr += loadedImage->GetRed(i,j)*mask33[x+1][y+1];
								sg += loadedImage->GetGreen(i,j)*mask33[x+1][y+1];
								sb += loadedImage->GetBlue(i,j)*mask33[x+1][y+1];
							}else if(x==-1&&y==1){
								sr += loadedImage->GetRed(i,j+y)*mask33[x+1][y+1];
								sg += loadedImage->GetGreen(i,j+y)*mask33[x+1][y+1];
								sb += loadedImage->GetBlue(i,j+y)*mask33[x+1][y+1];
							}else if(x==1&&y==-1){
								sr += loadedImage->GetRed(i+x,j)*mask33[x+1][y+1];
								sg += loadedImage->GetGreen(i+x,j)*mask33[x+1][y+1];
								sb += loadedImage->GetBlue(i+x,j)*mask33[x+1][y+1];	
							}
						}else if(i==1&&j==1){
							if((x==-1&&y==-1)||(x==0&&y==-1)||(y==0&&x==-1)){
								sr += loadedImage->GetRed(i,j)*mask33[x+1][y+1];
								sg += loadedImage->GetGreen(i,j)*mask33[x+1][y+1];
								sb += loadedImage->GetBlue(i,j)*mask33[x+1][y+1];
							}else if(x==-1&&y==1){
								sr += loadedImage->GetRed(i,j+y)*mask33[x+1][y+1];
								sg += loadedImage->GetGreen(i,j+y)*mask33[x+1][y+1];
								sb += loadedImage->GetBlue(i,j+y)*mask33[x+1][y+1];
							}else if(x==1&&y==-1){
								sr += loadedImage->GetRed(i+x,j)*mask33[x+1][y+1];
								sg += loadedImage->GetGreen(i+x,j)*mask33[x+1][y+1];
								sb += loadedImage->GetBlue(i+x,j)*mask33[x+1][y+1];	
							}
						//handle top right corner
						}else if(i==(imgWidth-1)&&j==0){
							if((x==1&&y==-1)||(x==0&&y==-1)||(y==0&&x==1)){
								sr += loadedImage->GetRed(i,j)*mask33[x+1][y+1];
								sg += loadedImage->GetGreen(i,j)*mask33[x+1][y+1];
								sb += loadedImage->GetBlue(i,j)*mask33[x+1][y+1];
							}else if(x==1&&y==1){
								sr += loadedImage->GetRed(i,j+y)*mask33[x+1][y+1];
								sg += loadedImage->GetGreen(i,j+y)*mask33[x+1][y+1];
								sb += loadedImage->GetBlue(i,j+y)*mask33[x+1][y+1];
							}else if(x==-1&&y==-1){
								sr += loadedImage->GetRed(i+x,j)*mask33[x+1][y+1];
								sg += loadedImage->GetGreen(i+x,j)*mask33[x+1][y+1];
								sb += loadedImage->GetBlue(i+x,j)*mask33[x+1][y+1];	
							}
						//handle bottom left corner
						}else if(i==0&&j==(imgHeight-1)){
							if((x==-1&&y==0)||(x==-1&&y==1)||(y==1&&x==0)){
								sr += loadedImage->GetRed(i,j)*mask33[x+1][y+1];
								sg += loadedImage->GetGreen(i,j)*mask33[x+1][y+1];
								sb += loadedImage->GetBlue(i,j)*mask33[x+1][y+1];
							}else if(x==-1&&y==-1){
								sr += loadedImage->GetRed(i,j+y)*mask33[x+1][y+1];
								sg += loadedImage->GetGreen(i,j+y)*mask33[x+1][y+1];
								sb += loadedImage->GetBlue(i,j+y)*mask33[x+1][y+1];
							}else if(x==1&&y==1){
								sr += loadedImage->GetRed(i+x,j)*mask33[x+1][y+1];
								sg += loadedImage->GetGreen(i+x,j)*mask33[x+1][y+1];
								sb += loadedImage->GetBlue(i+x,j)*mask33[x+1][y+1];	
							}
						//handle bottom right corner
						}else if(i==(imgWidth-1)&&j==(imgHeight-1)){
							if((x==1&&y==0)||(x==1&&y==1)||(y==1&&x==0)){
								sr += loadedImage->GetRed(i,j)*mask33[x+1][y+1];
								sg += loadedImage->GetGreen(i,j)*mask33[x+1][y+1];
								sb += loadedImage->GetBlue(i,j)*mask33[x+1][y+1];
							}else if(x==1&&y==-1){
								sr += loadedImage->GetRed(i,j+y)*mask33[x+1][y+1];
								sg += loadedImage->GetGreen(i,j+y)*mask33[x+1][y+1];
								sb += loadedImage->GetBlue(i,j+y)*mask33[x+1][y+1];
							}else if(x==-1&&y==1){
								sr += loadedImage->GetRed(i+x,j)*mask33[x+1][y+1];
								sg += loadedImage->GetGreen(i+x,j)*mask33[x+1][y+1];
								sb += loadedImage->GetBlue(i+x,j)*mask33[x+1][y+1];	
							}
						//handle top and bottom borders	
						}else if((i==0)||(i==(imgWidth-1))){
								sr += loadedImage->GetRed(i,j+y)*mask33[x+1][y+1];
								sg += loadedImage->GetGreen(i,j+y)*mask33[x+1][y+1];
								sb += loadedImage->GetBlue(i,j+y)*mask33[x+1][y+1];
						//handle left and right borders					
						}else if((j==0)||(j==(imgHeight-1))){
								sr += loadedImage->GetRed(i+x,j)*mask33[x+1][y+1];
								sg += loadedImage->GetGreen(i+x,j)*mask33[x+1][y+1];
								sb += loadedImage->GetBlue(i+x,j)*mask33[x+1][y+1];
						//handle the rest of the img
						}else{	
						sr += loadedImage->GetRed(i+x,j+y)*mask33[x+1][y+1];
						sg += loadedImage->GetGreen(i+x,j+y)*mask33[x+1][y+1];
						sb += loadedImage->GetBlue(i+x,j+y)*mask33[x+1][y+1];
						}
					}						
				}
	
				//absolute value conversion
				if(sr<0.0) sr=abs(sr);
				if(sg<0.0) sg=abs(sg);
				if(sb<0.0) sb=abs(sb);
			
				if(sr>255.0) sr=255;
				if(sg>255.0) sg=255;
				if(sb>255.0) sb=255;
			
				newImage->SetRGB(i,j,round(sr), round(sg), round(sb));

			      }
			for( int i=0;i<max(imgWidth,oldWidth);i++)
			   for(int j=0;j<max(imgHeight,oldWidth);j++){
				sr = newImage->GetRed(i,j);
				sg = newImage->GetGreen(i,j);
				sb = newImage->GetBlue(i,j);
				
				if((sr==0)&&(sg==0)&&(sb==0)){
					sr = loadedImage->GetRed(i,j);
					sg = loadedImage->GetGreen(i,j);
					sb = loadedImage->GetBlue(i,j);
					newImage->SetRGB(i,j,sr,sg,sb);
				}
			   }
			loadedImage = newImage;
		
			printf(" Finished convolution.\n");
			Refresh();
		}
	    }else{
		wxMessageBox(_T("The number that you entered is in wrong format.\nPlease try again."));
	    }

	break;
case 2:
	printf("5x5 Filtering\n");
	number = wxGetTextFromUser(_T("Please choose the matrix.\n1  1/25*{{1,1,1,1,1},{1,1,1,1,1},{1,1,1,1,1},{1,1,1,1,1},{1,1,1,1,1}} \n2  1/52*{{1,1,2,1,1},{1,2,4,2,1},{2,4,8,4,2},{1,2,4,2,1},{1,1,2,1,1}} "), _T(""), _T(""));
	if(number.ToLong(&m)&&(m==1||m==2)){
		switch(m){
			//Averaging
			case 1:
				printf("5x5. Case1\n");
				for(int i=0;i<5;i++){
					for(int j=0;j<5;j++){
						mask55[i][j]=(float)1/25;
						printf("%f ",mask55[i][j]*25);						
					}	
					printf("\n");				
				}
				break;
			//Weighted Averaging
			case 2:
				printf("5x5. Case2\n");
				
				for(int i=0;i<5;i++){
					for(int j=0;j<5;j++){
						if(i==2&&j==2){
							mask55[i][j]=(float)8/52;
						}else if((i==2&&j==1)||(i==3&&j==2)||(i==1&&j==2)||(i==2&&j==3)){
							mask55[i][j]=(float)4/52;
						}else if( (i==2&&j==0)||(i==1&&j==1)||(i==0&&j==2)||(i==1&&j==3)
							||(i==2&&j==4)||(i==3&&j==3)||(i==4&&j==2)||(i==3&&j==1)){
							mask55[i][j]=(float)2/52;
						}else{
							mask55[i][j]=(float)1/52;
						}
						printf("%f ",mask55[i][j]*52);						
					}	
					printf("\n");				
				}
				
				break;
			default:
				wxMessageBox(_T("The number that you entered is in wrong format.\nPlease try again."));
				err=-1;
		}
		if(err==0){
			//Iterate over image pixel values and apply the mask
			for( int i=startWidth;i<imgWidth;i++)
              		   for(int j=startHeight;j<imgHeight;j++){
				sr=0;
				sg=0;
				sb=0;
				for(int x=-2;x<=2;x++){
					for(int y=-2;y<=2;y++){
							sr += loadedImage->GetRed(i+x,j+y)*mask55[x+1][y+1];
							sg += loadedImage->GetGreen(i+x,j+y)*mask55[x+1][y+1];
							sb += loadedImage->GetBlue(i+x,j+y)*mask55[x+1][y+1];
					}						
				}
	
				//absolute value conversion
				if(sr<0.0) sr=abs(sr);
				if(sg<0.0) sg=abs(sg);
				if(sb<0.0) sb=abs(sb);
			
				if(sr>255.0) sr=255;
				if(sg>255.0) sg=255;
				if(sb>255.0) sb=255;
			
				newImage->SetRGB(i,j,round(sr), round(sg), round(sb));

			      }
			for( int i=0;i<max(imgWidth,oldWidth);i++)
			   for(int j=0;j<max(imgHeight,oldWidth);j++){
				sr = newImage->GetRed(i,j);
				sg = newImage->GetGreen(i,j);
				sb = newImage->GetBlue(i,j);
				
				if((sr==0)&&(sg==0)&&(sb==0)){
					sr = loadedImage->GetRed(i,j);
					sg = loadedImage->GetGreen(i,j);
					sb = loadedImage->GetBlue(i,j);
					newImage->SetRGB(i,j,sr,sg,sb);
				}
			   }
			loadedImage = newImage;
		
			printf(" Finished convolution.\n");
			Refresh();
		}
	}else{
		wxMessageBox(_T("The number that you entered is in wrong format.\nPlease try again."));		
		err=-1;	
	}
	break;
default:
	wxMessageBox(_T("The number that you entered is in wrong format.\nPlease try again."));	
	err=-1;
}
}else{
wxMessageBox(_T("The number that you entered is in wrong format.\nPlease try again."));
}
}

//###########################################################//
//-----------------------------------------------------------//
//--------------- ORDER STATISTIC FILTERING------------------//
//-----------------------------------------------------------//
//###########################################################//

void MyFrame::OnOSFImage(wxCommandEvent & event){
    long int n;//for switch
    int x;
    int y;
    wxString number = wxGetTextFromUser(_T("Please choose the action.\n1  Salt-and-Pepper Noise\n2  Min Filtering\n3  Max Filtering \n4  Midpoint Filtering\n5  Median Filtering  "), _T(""), _T(""));
    int err=0;
    float mask33[3][3];
    if(number.ToLong(&n)&&(n==1||n==2||n==3||n==4||n==5)){
	    float sr=0,sg=0,sb=0;
	    float maxr=0,maxg=0,maxb=0;
	    float minr=0,ming=0,minb=0;
	    float rarr[9];
	    float garr[9];
            float barr[9];
		
	    printf("Order statistic filtering...");
	    free(loadedImage);
	    loadedImage = new wxImage(bitmap.ConvertToImage());
	    appendToUndo(*loadedImage); 
	    wxImage *newImage  = new wxImage((bitmap.ConvertToImage()).GetWidth(),(bitmap.ConvertToImage()).GetHeight(),true);
	    switch(n){
/************** ADDING SALT-AND-PEPPER NOISE ****************/		
                case 1:
			printf("adding salt-and-pepper...");
			
			for(int i=startWidth*startHeight;i<imgWidth*imgHeight*0.05;i++){
			      
				x = rand()%imgWidth;
				y = rand()%imgHeight;
				loadedImage->SetRGB(x,y,255, 255, 255);
		
			}
			for(int i=startWidth*startHeight;i<imgWidth*imgHeight*0.05;i++){
			      
				x = rand()%imgWidth;
				y = rand()%imgHeight;
				loadedImage->SetRGB(x,y,0, 0, 0);
		
			}	    
		    	printf(" Finished filtering.\n");
		    	Refresh();
			break;
/******************** MIN FILTERING ***********************/
		case 2:
			printf("min filtering...");
			//Iterate over image pixel values
			for( int i=startWidth;i<imgWidth;i++)
              		    for(int j=startHeight;j<imgHeight;j++){
				minr=255;
				ming=255;
				minb=255;
				for(int x=-1;x<=1;x++){
					for(int y=-1;y<=1;y++){
						//handle top left corner
						if(i==0&&j==0){
							if((x==-1&&y==-1)||(x==0&&y==-1)||(y==0&&x==-1)){
								sr = (float)loadedImage->GetRed(i,j);
								sg = (float) loadedImage->GetGreen(i,j);
								sb = (float) loadedImage->GetBlue(i,j);
							}else if(x==-1&&y==1){
								sr = (float) loadedImage->GetRed(i,j+y);
								sg = (float) loadedImage->GetGreen(i,j+y);
								sb = (float) loadedImage->GetBlue(i,j+y);
							}else if(x==1&&y==-1){
								sr = (float) loadedImage->GetRed(i+x,j);
								sg = (float) loadedImage->GetGreen(i+x,j);
								sb = (float) loadedImage->GetBlue(i+x,j);	
							}
						//handle top right corner
						}else if(i==(imgWidth-1)&&j==0){
							if((x==1&&y==-1)||(x==0&&y==-1)||(y==0&&x==1)){
								sr = (float) loadedImage->GetRed(i,j);
								sg = (float) loadedImage->GetGreen(i,j);
								sb = (float) loadedImage->GetBlue(i,j);
							}else if(x==1&&y==1){
								sr = (float) loadedImage->GetRed(i,j+y);
								sg = (float) loadedImage->GetGreen(i,j+y);
								sb = (float) loadedImage->GetBlue(i,j+y);
							}else if(x==-1&&y==-1){
								sr = (float) loadedImage->GetRed(i+x,j);
								sg = (float) loadedImage->GetGreen(i+x,j);
								sb = (float) loadedImage->GetBlue(i+x,j);	
							}
						//handle bottom left corner
						}else if(i==0&&j==(imgHeight-1)){
							if((x==-1&&y==0)||(x==-1&&y==1)||(y==1&&x==0)){
								sr = (float) loadedImage->GetRed(i,j);
								sg = (float) loadedImage->GetGreen(i,j);
								sb = (float) loadedImage->GetBlue(i,j);
							}else if(x==-1&&y==-1){
								sr = (float) loadedImage->GetRed(i,j+y);
								sg = (float) loadedImage->GetGreen(i,j+y);
								sb = (float) loadedImage->GetBlue(i,j+y);
							}else if(x==1&&y==1){
								sr = (float) loadedImage->GetRed(i+x,j);
								sg = (float) loadedImage->GetGreen(i+x,j);
								sb = (float) loadedImage->GetBlue(i+x,j);	
							}
						//handle bottom right corner
						}else if(i==(imgWidth-1)&&j==(imgHeight-1)){
							if((x==1&&y==0)||(x==1&&y==1)||(y==1&&x==0)){
								sr = (float) loadedImage->GetRed(i,j);
								sg = (float) loadedImage->GetGreen(i,j);
								sb = (float) loadedImage->GetBlue(i,j);
							}else if(x==1&&y==-1){
								sr = (float) loadedImage->GetRed(i,j+y);
								sg = (float) loadedImage->GetGreen(i,j+y);
								sb = (float) loadedImage->GetBlue(i,j+y);
							}else if(x==-1&&y==1){
								sr = (float) loadedImage->GetRed(i+x,j);
								sg = (float) loadedImage->GetGreen(i+x,j);
								sb = (float) loadedImage->GetBlue(i+x,j);	
							}
						//handle top and bottom borders	
						}else if((i==0)||(i==(imgWidth-1))){
								sr = (float) loadedImage->GetRed(i,j+y);
								sg = (float) loadedImage->GetGreen(i,j+y);
								sb = (float) loadedImage->GetBlue(i,j+y);
						//handle left and right borders					
						}else if((j==0)||(j==(imgHeight-1))){
								sr = (float) loadedImage->GetRed(i+x,j);
								sg = (float) loadedImage->GetGreen(i+x,j);
								sb = (float) loadedImage->GetBlue(i+x,j);
						//handle the rest of the img
						}else{	
							sr = (float)loadedImage->GetRed(i+x,j+y);
							sg = (float)loadedImage->GetGreen(i+x,j+y);
							sb = (float)loadedImage->GetBlue(i+x,j+y);
						}
						if(sr<minr) minr =  sr;
						if(sg<ming) ming =  sg;
						if(sb<minb) minb =  sb;
					}	
				}					

				newImage->SetRGB(i,j,round(minr), round(ming), round(minb));

			      }
		    	printf(" Finished filtering.\n");
			for( int i=0;i<max(imgWidth,oldWidth);i++)
			   for(int j=0;j<max(imgHeight,oldWidth);j++){
				sr = newImage->GetRed(i,j);
				sg = newImage->GetGreen(i,j);
				sb = newImage->GetBlue(i,j);
				
				if((sr==0)&&(sg==0)&&(sb==0)){
					sr = loadedImage->GetRed(i,j);
					sg = loadedImage->GetGreen(i,j);
					sb = loadedImage->GetBlue(i,j);
					newImage->SetRGB(i,j,sr,sg,sb);
				}
			   }
		    	loadedImage = newImage;
		   	Refresh();
			break;
/******************** MAX FILTERING ***********************/
		case 3:
			printf("max filtering...");
			//Iterate over image pixel values
			for( int i=startWidth;i<imgWidth;i++)
              		    for(int j=startHeight;j<imgHeight;j++){
				maxr=0;
				maxg=0;
				maxb=0;
				for(int x=-1;x<=1;x++){
					for(int y=-1;y<=1;y++){
						//handle top left corner
						if(i==0&&j==0){
							if((x==-1&&y==-1)||(x==0&&y==-1)||(y==0&&x==-1)){
								sr = (float)loadedImage->GetRed(i,j);
								sg = (float) loadedImage->GetGreen(i,j);
								sb = (float) loadedImage->GetBlue(i,j);
							}else if(x==-1&&y==1){
								sr = (float) loadedImage->GetRed(i,j+y);
								sg = (float) loadedImage->GetGreen(i,j+y);
								sb = (float) loadedImage->GetBlue(i,j+y);
							}else if(x==1&&y==-1){
								sr = (float) loadedImage->GetRed(i+x,j);
								sg = (float) loadedImage->GetGreen(i+x,j);
								sb = (float) loadedImage->GetBlue(i+x,j);	
							}
						//handle top right corner
						}else if(i==(imgWidth-1)&&j==0){
							if((x==1&&y==-1)||(x==0&&y==-1)||(y==0&&x==1)){
								sr = (float) loadedImage->GetRed(i,j);
								sg = (float) loadedImage->GetGreen(i,j);
								sb = (float) loadedImage->GetBlue(i,j);
							}else if(x==1&&y==1){
								sr = (float) loadedImage->GetRed(i,j+y);
								sg = (float) loadedImage->GetGreen(i,j+y);
								sb = (float) loadedImage->GetBlue(i,j+y);
							}else if(x==-1&&y==-1){
								sr = (float) loadedImage->GetRed(i+x,j);
								sg = (float) loadedImage->GetGreen(i+x,j);
								sb = (float) loadedImage->GetBlue(i+x,j);	
							}
						//handle bottom left corner
						}else if(i==0&&j==(imgHeight-1)){
							if((x==-1&&y==0)||(x==-1&&y==1)||(y==1&&x==0)){
								sr = (float) loadedImage->GetRed(i,j);
								sg = (float) loadedImage->GetGreen(i,j);
								sb = (float) loadedImage->GetBlue(i,j);
							}else if(x==-1&&y==-1){
								sr = (float) loadedImage->GetRed(i,j+y);
								sg = (float) loadedImage->GetGreen(i,j+y);
								sb = (float) loadedImage->GetBlue(i,j+y);
							}else if(x==1&&y==1){
								sr = (float) loadedImage->GetRed(i+x,j);
								sg = (float) loadedImage->GetGreen(i+x,j);
								sb = (float) loadedImage->GetBlue(i+x,j);	
							}
						//handle bottom right corner
						}else if(i==(imgWidth-1)&&j==(imgHeight-1)){
							if((x==1&&y==0)||(x==1&&y==1)||(y==1&&x==0)){
								sr = (float) loadedImage->GetRed(i,j);
								sg = (float) loadedImage->GetGreen(i,j);
								sb = (float) loadedImage->GetBlue(i,j);
							}else if(x==1&&y==-1){
								sr = (float) loadedImage->GetRed(i,j+y);
								sg = (float) loadedImage->GetGreen(i,j+y);
								sb = (float) loadedImage->GetBlue(i,j+y);
							}else if(x==-1&&y==1){
								sr = (float) loadedImage->GetRed(i+x,j);
								sg = (float) loadedImage->GetGreen(i+x,j);
								sb = (float) loadedImage->GetBlue(i+x,j);	
							}
						//handle top and bottom borders	
						}else if((i==0)||(i==(imgWidth-1))){
								sr = (float) loadedImage->GetRed(i,j+y);
								sg = (float) loadedImage->GetGreen(i,j+y);
								sb = (float) loadedImage->GetBlue(i,j+y);
						//handle left and right borders					
						}else if((j==0)||(j==(imgHeight-1))){
								sr = (float) loadedImage->GetRed(i+x,j);
								sg = (float) loadedImage->GetGreen(i+x,j);
								sb = (float) loadedImage->GetBlue(i+x,j);
						//handle the rest of the img
						}else{	
							sr = (float)loadedImage->GetRed(i+x,j+y);
							sg = (float)loadedImage->GetGreen(i+x,j+y);
							sb = (float)loadedImage->GetBlue(i+x,j+y);
						}
						if(sr>maxr) maxr =  sr;
						if(sg>maxg) maxg =  sg;
						if(sb>maxb) maxb =  sb;
					}		
				}					

				newImage->SetRGB(i,j,round(maxr), round(maxg), round(maxb));

			      }
		    	printf(" Finished filtering.\n");
			for( int i=0;i<max(imgWidth,oldWidth);i++)
			   for(int j=0;j<max(imgHeight,oldWidth);j++){
				sr = newImage->GetRed(i,j);
				sg = newImage->GetGreen(i,j);
				sb = newImage->GetBlue(i,j);
				
				if((sr==0)&&(sg==0)&&(sb==0)){
					sr = loadedImage->GetRed(i,j);
					sg = loadedImage->GetGreen(i,j);
					sb = loadedImage->GetBlue(i,j);
					newImage->SetRGB(i,j,sr,sg,sb);
				}
			   }
		    	loadedImage = newImage;
		   	Refresh();
			break;
/****************** MIDPOINT FILTERING *********************/
		case 4:
			printf("midpoint filtering...");
			//Iterate over image pixel values
			for( int i=startWidth;i<imgWidth;i++)
                            for(int j=startHeight;j<imgHeight;j++){
				maxr=0;
				maxg=0;
				maxb=0;
				minr=255;
				ming=255;
				minb=255;
				for(int x=-1;x<=1;x++){
					for(int y=-1;y<=1;y++){
						//handle top left corner
						if(i==0&&j==0){
							if((x==-1&&y==-1)||(x==0&&y==-1)||(y==0&&x==-1)){
								sr = (float)loadedImage->GetRed(i,j);
								sg = (float) loadedImage->GetGreen(i,j);
								sb = (float) loadedImage->GetBlue(i,j);
							}else if(x==-1&&y==1){
								sr = (float) loadedImage->GetRed(i,j+y);
								sg = (float) loadedImage->GetGreen(i,j+y);
								sb = (float) loadedImage->GetBlue(i,j+y);
							}else if(x==1&&y==-1){
								sr = (float) loadedImage->GetRed(i+x,j);
								sg = (float) loadedImage->GetGreen(i+x,j);
								sb = (float) loadedImage->GetBlue(i+x,j);	
							}
						//handle top right corner
						}else if(i==(imgWidth-1)&&j==0){
							if((x==1&&y==-1)||(x==0&&y==-1)||(y==0&&x==1)){
								sr = (float) loadedImage->GetRed(i,j);
								sg = (float) loadedImage->GetGreen(i,j);
								sb = (float) loadedImage->GetBlue(i,j);
							}else if(x==1&&y==1){
								sr = (float) loadedImage->GetRed(i,j+y);
								sg = (float) loadedImage->GetGreen(i,j+y);
								sb = (float) loadedImage->GetBlue(i,j+y);
							}else if(x==-1&&y==-1){
								sr = (float) loadedImage->GetRed(i+x,j);
								sg = (float) loadedImage->GetGreen(i+x,j);
								sb = (float) loadedImage->GetBlue(i+x,j);	
							}
						//handle bottom left corner
						}else if(i==0&&j==(imgHeight-1)){
							if((x==-1&&y==0)||(x==-1&&y==1)||(y==1&&x==0)){
								sr = (float) loadedImage->GetRed(i,j);
								sg = (float) loadedImage->GetGreen(i,j);
								sb = (float) loadedImage->GetBlue(i,j);
							}else if(x==-1&&y==-1){
								sr = (float) loadedImage->GetRed(i,j+y);
								sg = (float) loadedImage->GetGreen(i,j+y);
								sb = (float) loadedImage->GetBlue(i,j+y);
							}else if(x==1&&y==1){
								sr = (float) loadedImage->GetRed(i+x,j);
								sg = (float) loadedImage->GetGreen(i+x,j);
								sb = (float) loadedImage->GetBlue(i+x,j);	
							}
						//handle bottom right corner
						}else if(i==(imgWidth-1)&&j==(imgHeight-1)){
							if((x==1&&y==0)||(x==1&&y==1)||(y==1&&x==0)){
								sr = (float) loadedImage->GetRed(i,j);
								sg = (float) loadedImage->GetGreen(i,j);
								sb = (float) loadedImage->GetBlue(i,j);
							}else if(x==1&&y==-1){
								sr = (float) loadedImage->GetRed(i,j+y);
								sg = (float) loadedImage->GetGreen(i,j+y);
								sb = (float) loadedImage->GetBlue(i,j+y);
							}else if(x==-1&&y==1){
								sr = (float) loadedImage->GetRed(i+x,j);
								sg = (float) loadedImage->GetGreen(i+x,j);
								sb = (float) loadedImage->GetBlue(i+x,j);	
							}
						//handle top and bottom borders	
						}else if((i==0)||(i==(imgWidth-1))){
								sr = (float) loadedImage->GetRed(i,j+y);
								sg = (float) loadedImage->GetGreen(i,j+y);
								sb = (float) loadedImage->GetBlue(i,j+y);
						//handle left and right borders					
						}else if((j==0)||(j==(imgHeight-1))){
								sr = (float) loadedImage->GetRed(i+x,j);
								sg = (float) loadedImage->GetGreen(i+x,j);
								sb = (float) loadedImage->GetBlue(i+x,j);
						//handle the rest of the img
						}else{	
							sr = (float)loadedImage->GetRed(i+x,j+y);
							sg = (float)loadedImage->GetGreen(i+x,j+y);
							sb = (float)loadedImage->GetBlue(i+x,j+y);
						}
						if(sr<minr) minr =  sr;
						if(sg<ming) ming =  sg;
						if(sb<minb) minb =  sb;
						if(sr>maxr) maxr =  sr;
						if(sg>maxg) maxg =  sg;
						if(sb>maxb) maxb =  sb;
					}
				}					

				newImage->SetRGB(i,j,round((maxr+minr)/2), round((maxg+ming)/2), round((maxb+minb)/2));

			      }
		    	printf(" Finished filtering.\n");
			for( int i=0;i<max(imgWidth,oldWidth);i++)
			   for(int j=0;j<max(imgHeight,oldWidth);j++){
				sr = newImage->GetRed(i,j);
				sg = newImage->GetGreen(i,j);
				sb = newImage->GetBlue(i,j);
				
				if((sr==0)&&(sg==0)&&(sb==0)){
					sr = loadedImage->GetRed(i,j);
					sg = loadedImage->GetGreen(i,j);
					sb = loadedImage->GetBlue(i,j);
					newImage->SetRGB(i,j,sr,sg,sb);
				}
			   }
		    	loadedImage = newImage;
		   	Refresh();
			break;
/******************* MEDIAN FILTERING **********************/
		case 5:
			printf("median filtering...");
			//Iterate over image pixel values
			for( int i=startWidth;i<imgWidth;i++)
              		    for(int j=startHeight;j<imgHeight;j++){
				int count = 0;
				for(int x=-1;x<=1;x++){
					for(int y=-1;y<=1;y++){
						//handle top left corner
						if(i==0&&j==0){
							if((x==-1&&y==-1)||(x==0&&y==-1)||(y==0&&x==-1)){
								sr = (float)loadedImage->GetRed(i,j);
								sg = (float) loadedImage->GetGreen(i,j);
								sb = (float) loadedImage->GetBlue(i,j);
							}else if(x==-1&&y==1){
								sr = (float) loadedImage->GetRed(i,j+y);
								sg = (float) loadedImage->GetGreen(i,j+y);
								sb = (float) loadedImage->GetBlue(i,j+y);
							}else if(x==1&&y==-1){
								sr = (float) loadedImage->GetRed(i+x,j);
								sg = (float) loadedImage->GetGreen(i+x,j);
								sb = (float) loadedImage->GetBlue(i+x,j);	
							}
						//handle top right corner
						}else if(i==(imgWidth-1)&&j==0){
							if((x==1&&y==-1)||(x==0&&y==-1)||(y==0&&x==1)){
								sr = (float) loadedImage->GetRed(i,j);
								sg = (float) loadedImage->GetGreen(i,j);
								sb = (float) loadedImage->GetBlue(i,j);
							}else if(x==1&&y==1){
								sr = (float) loadedImage->GetRed(i,j+y);
								sg = (float) loadedImage->GetGreen(i,j+y);
								sb = (float) loadedImage->GetBlue(i,j+y);
							}else if(x==-1&&y==-1){
								sr = (float) loadedImage->GetRed(i+x,j);
								sg = (float) loadedImage->GetGreen(i+x,j);
								sb = (float) loadedImage->GetBlue(i+x,j);	
							}
						//handle bottom left corner
						}else if(i==0&&j==(imgHeight-1)){
							if((x==-1&&y==0)||(x==-1&&y==1)||(y==1&&x==0)){
								sr = (float) loadedImage->GetRed(i,j);
								sg = (float) loadedImage->GetGreen(i,j);
								sb = (float) loadedImage->GetBlue(i,j);
							}else if(x==-1&&y==-1){
								sr = (float) loadedImage->GetRed(i,j+y);
								sg = (float) loadedImage->GetGreen(i,j+y);
								sb = (float) loadedImage->GetBlue(i,j+y);
							}else if(x==1&&y==1){
								sr = (float) loadedImage->GetRed(i+x,j);
								sg = (float) loadedImage->GetGreen(i+x,j);
								sb = (float) loadedImage->GetBlue(i+x,j);	
							}
						//handle bottom right corner
						}else if(i==(imgWidth-1)&&j==(imgHeight-1)){
							if((x==1&&y==0)||(x==1&&y==1)||(y==1&&x==0)){
								sr = (float) loadedImage->GetRed(i,j);
								sg = (float) loadedImage->GetGreen(i,j);
								sb = (float) loadedImage->GetBlue(i,j);
							}else if(x==1&&y==-1){
								sr = (float) loadedImage->GetRed(i,j+y);
								sg = (float) loadedImage->GetGreen(i,j+y);
								sb = (float) loadedImage->GetBlue(i,j+y);
							}else if(x==-1&&y==1){
								sr = (float) loadedImage->GetRed(i+x,j);
								sg = (float) loadedImage->GetGreen(i+x,j);
								sb = (float) loadedImage->GetBlue(i+x,j);	
							}
						//handle top and bottom borders	
						}else if((i==0)||(i==(imgWidth-1))){
								sr = (float) loadedImage->GetRed(i,j+y);
								sg = (float) loadedImage->GetGreen(i,j+y);
								sb = (float) loadedImage->GetBlue(i,j+y);
						//handle left and right borders					
						}else if((j==0)||(j==(imgHeight-1))){
								sr = (float) loadedImage->GetRed(i+x,j);
								sg = (float) loadedImage->GetGreen(i+x,j);
								sb = (float) loadedImage->GetBlue(i+x,j);
						//handle the rest of the img
						}else{	
							sr = (float)loadedImage->GetRed(i+x,j+y);
							sg = (float)loadedImage->GetGreen(i+x,j+y);
							sb = (float)loadedImage->GetBlue(i+x,j+y);
						}
						
						rarr[count] = sr;	
						garr[count] = sg;
						barr[count] = sb;
						count++;
					}
				}					
				//sort rgb arrays
				int u,v;
				int size = 9;
				float temp;
				for(u=0;u<size-1;u++){
					for(v=0;v<size-1;v++){
						if (rarr[v] > rarr[v+1])
						   {
							temp = rarr[v+1];
							rarr[v+1] = rarr[v];
							rarr[v] = temp;
						   }
					}
				}
				for(u=0;u<size-1;u++){
					for(v=0;v<size-1;v++){
						if (garr[v] > garr[v+1])
						   {
							temp = garr[v+1];
							garr[v+1] = garr[v];
							garr[v] = temp;
						   }
					}
				}	
				for(u=0;u<size-1;u++){
					for(v=0;v<size-1;v++){
						if (barr[v] > barr[v+1])
						   {
							temp = barr[v+1];
							barr[v+1] = barr[v];
							barr[v] = temp;
						   }
					}
				}
					
				newImage->SetRGB(i,j,round(rarr[4]), round(garr[4]), round(barr[4]));

			      }
			printf(" Finished filtering.\n");
			for( int i=0;i<max(imgWidth,oldWidth);i++)
			   for(int j=0;j<max(imgHeight,oldWidth);j++){
				sr = newImage->GetRed(i,j);
				sg = newImage->GetGreen(i,j);
				sb = newImage->GetBlue(i,j);
				
				if((sr==0)&&(sg==0)&&(sb==0)){
					sr = loadedImage->GetRed(i,j);
					sg = loadedImage->GetGreen(i,j);
					sb = loadedImage->GetBlue(i,j);
					newImage->SetRGB(i,j,sr,sg,sb);
				}
			   }
		    	loadedImage = newImage;
		   	Refresh();
			break;
		default:
			wxMessageBox(_T("The number that you entered is in wrong format.\nPlease try again."));	
			err=-1;
	    }
    }else{
	    wxMessageBox(_T("The number that you entered is in wrong format.\nPlease try again."));
    }
}

//###########################################################//
//-----------------------------------------------------------//
//-------------------- POINT PROCESSING----------------------//
//-----------------------------------------------------------//
//###########################################################//

void MyFrame::OnPointProcessingImage(wxCommandEvent & event){
    long int n;//for switch
    float r,g,b;
    float rmax,gmax,bmax;
    float pmax;
    double power=1.0;
    int lookup_table [256];

    wxString number = wxGetTextFromUser(_T("Please choose the action.\n1  Negative Linear Transform\n2  Logarithmic Function\n3  Power-Law \n4  Random Look-up Table"), _T(""), _T(""));
    int err=0;
    if(number.ToLong(&n)&&(n==1||n==2||n==3||n==4)){
	printf("Point processing...");
	free(loadedImage);
    	loadedImage = new wxImage(bitmap.ConvertToImage());
	appendToUndo(*loadedImage); 
	float c = 1.0;
			
   	switch(n){
/*************** NEGATIVE LINEAR TRANSFORM *****************/
		case 1:	
			printf("negative linear transform...");
			for( int i=startWidth;i<imgWidth;i++)
                            for(int j=startHeight;j<imgHeight;j++){
			 	loadedImage->SetRGB(i,j,255-loadedImage->GetRed(i,j), 
							255-loadedImage->GetGreen(i,j),
							255-loadedImage->GetBlue(i,j));
			    }
			printf("Finished point processing\n");
			break;
/***************** LOGARITHMIC FUNCTION ********************/
		case 2:
			printf("log fn...");
			pmax = 255;
			c = 255/(log(1+pmax));
			for( int i=startWidth;i<imgWidth;i++)
              	 	    for(int j=startHeight;j<imgHeight;j++){
				r = loadedImage->GetRed(i,j);
				g = loadedImage->GetGreen(i,j);
				b = loadedImage->GetBlue(i,j);
				r = c*log(1+r);
				g = c*log(1+g);
				b = c*log(1+b);
			        loadedImage->SetRGB(i,j,round(r),round(g),round(b));
			    }

			printf("Finished point processing\n");
			break;
/********************** POWER-LAW **************************/
		case 3:
			printf("power-law...");
			number = wxGetTextFromUser(_T("Please enter the power.\nThe power should be a number,\nwhich can be from 0.01 to 25"), _T(""), _T(""));
		    	if(number.ToDouble(&power) && power<=25 && power>=0.01){
				pmax = 255;
				c = 255/pow(pmax,power);
				for( int i=startWidth;i<imgWidth;i++)
              			    for(int j=startHeight;j<imgHeight;j++){
				 	loadedImage->SetRGB(i,j,c*pow(loadedImage->GetRed(i,j),power), 
								c*pow(loadedImage->GetGreen(i,j),power),
								c*pow(loadedImage->GetBlue(i,j),power));
				    }
				printf("Finished point processing\n");
			}else{
	    			wxMessageBox(_T("The number that you entered is in wrong format.\nPlease try again."));
			}
			break;
/***************** RANDOM LOOK-UP TABLE ********************/
		case 4:
			printf("random look-up table...");
			for (int i=0;i<256;i++){
				lookup_table[i] = rand() % 256; //assign random numbers between 0 and 255, inclusively			
			}
			for( int i=startWidth;i<imgWidth;i++)
              			for(int j=startHeight;j<imgHeight;j++){
				 	loadedImage->SetRGB(i,j,lookup_table[loadedImage->GetRed(i,j)], 
								lookup_table[loadedImage->GetGreen(i,j)],
								lookup_table[loadedImage->GetBlue(i,j)]);
				    }
			printf("Finished point processing\n");
			break;
		default:
			wxMessageBox(_T("The number that you entered is in wrong format.\nPlease try again."));
			err=-1;
	}
	Refresh();
    }else{
	    wxMessageBox(_T("The number that you entered is in wrong format.\nPlease try again."));
    }
}

//###########################################################//
//-----------------------------------------------------------//
//------------------- IMAGE THRESHOLDING---------------------//
//-----------------------------------------------------------//
//###########################################################//

void MyFrame::OnThresholdingImage(wxCommandEvent & event){
    long int n;//for switch
   
    wxString number = wxGetTextFromUser(_T("Please choose the action.\n1  Mean and Standard Deviation\n2  Simple thresholding\n3  Automated thresholding \n4  Adaprive thresholding"), _T(""), _T(""));
    int err=0;
    if(number.ToLong(&n)&&(n==1||n==2||n==3||n==4)){
	printf("Thresholding...");
	free(loadedImage);
    	loadedImage = new wxImage(bitmap.ConvertToImage());
	appendToUndo(*loadedImage); 
	int histogram_r[256] = {0};
	int histogram_g[256] = {0};
	int histogram_b[256] = {0};
	//initialise normalised histogram
	float norm_histogram_r[256] = {0};
	float norm_histogram_g[256] = {0};
	float norm_histogram_b[256] = {0};
	//number of pixels
	float c = 0;

	float mean_r = 0;
	float mean_g = 0;
	float mean_b = 0;
	float var_r = 0;
	float var_g = 0;
	float var_b = 0;
	float sd_r = 0;
	float sd_g = 0;
	float sd_b = 0;
			
	double r,g,b;
	double s_threshold;

	float t0_r = 1;
	float t_r;
	float t1_r;
	float t0_g = 1;
	float t_g;
	float t1_g;
	float t0_b = 1;
	float t_b;
	float t1_b;
	float mean_obj_r = 0;
	float mean_obj_g = 0;
	float mean_obj_b = 0;
	float mean_back_r = 0;
	float mean_back_g = 0;
	float mean_back_b = 0;
	int cnt_obj_r = 0;
	int cnt_obj_g = 0;
	int cnt_obj_b = 0;
	int cnt_back_r = 0;
	int cnt_back_g = 0;
	int cnt_back_b = 0;

	int n_blocks = 0;
	float var = 0;
	float mean = 0;
	float mean_obj = 0;
	float mean_back = 0;
	float t0 = 1;
	float t;
	float t1;
	int cnt_obj = 0;
	int cnt_back = 0;
	//user input variance
	double variance = 0;
	switch(n){
/*************** Mean and standard deviation *****************/
		case 1:	
			printf("mean and standard deviation...\n");
			//Find histogram values
			for( int i=startWidth;i<imgWidth;i++)
              			for(int j=startHeight;j<imgHeight;j++){
					histogram_r[loadedImage->GetRed(i,j)] += 1;
					histogram_g[loadedImage->GetGreen(i,j)] += 1;
					histogram_b[loadedImage->GetBlue(i,j)] += 1;	
				     	c += 1; 
				    }
			//Normalise histogram values
			for (int i=0; i<256; i++){
				norm_histogram_r[i] = histogram_r[i] / c;
				norm_histogram_g[i] = histogram_g[i] / c;
				norm_histogram_b[i] = histogram_b[i] / c;	
			}
			//find mean
			for (int i=0; i<256; i++){
				mean_r+= (i*norm_histogram_r[i]);
				mean_g+= (i*norm_histogram_g[i]);
				mean_b+= (i*norm_histogram_b[i]);	
			}
			printf("Mean: [%f|%f|%f]\n", mean_r, mean_g, mean_b);
			//find standard deviation (sqrt of variance)
			for (int i=0; i<256; i++){
				var_r+= pow((i-mean_r),2)*norm_histogram_r[i];
				var_g+= pow((i-mean_g),2)*norm_histogram_g[i];
				var_b+= pow((i-mean_b),2)*norm_histogram_b[i];	
			}
			sd_r+= sqrt(var_r);
			sd_g+= sqrt(var_g) ;
			sd_b+= sqrt(var_b) ;	
			printf("Standard deviation is: [%f|%f|%f]\n", sd_r, sd_g, sd_b);
			printf("Finished thresholding\n");
			break;
/***************** Simple thresholding ********************/
		case 2:
			printf("simple thresholding...");
			number = wxGetTextFromUser(_T("Please enter the value of the threshold.\nThe threshold should be a number between 0 and 255"), _T(""), _T(""));
		    	if(number.ToDouble(&s_threshold) && s_threshold<=255 && s_threshold>=0){
				for( int i=startWidth;i<imgWidth;i++)
              			    for(int j=startHeight;j<imgHeight;j++){
					r = loadedImage->GetRed(i,j);
					g = loadedImage->GetGreen(i,j);
					b = loadedImage->GetBlue(i,j);
					if(r >= s_threshold){
						r = 255;
					}else if (r < s_threshold){
						r = 0;
					}
					if(g >= s_threshold){
						g = 255;
					}else if (g < s_threshold){
						g = 0;
					}
					if(b >= s_threshold){
						b = 255;
					}else if (b < s_threshold){
						b = 0;
					}
			
				 	loadedImage->SetRGB(i,j,r,g,b);
				    }
				printf("Finished thresholding\n");
			}else{
	    			wxMessageBox(_T("The number that you entered is in wrong format.\nPlease try again."));
			}
			break;
/***************** Automated thresholding *****************/
		case 3:
			printf("automated thresholding...\n");
			mean_back_r = 0;
			mean_back_g = 0;
			mean_back_b = 0;
			mean_obj_r = 0;
			mean_obj_g = 0;
			mean_obj_b = 0;
			//Initiation: assume that background is only 4 corners and object the others
			//find mean of obj and mean of background
			for( int i=startWidth;i<imgWidth;i++){
              		    for(int j=startHeight;j<imgHeight;j++){
				r = loadedImage->GetRed(i,j);
				g = loadedImage->GetGreen(i,j);
				b = loadedImage->GetBlue(i,j);

				if((i == 0 && j == 0) 
				|| (i == imgWidth-1 && j == 0) 
				|| (i == 0 && j == imgHeight-1) 
				|| (i ==imgWidth-1 && j == imgHeight-1)){
					mean_back_r += r;
					mean_back_g += g;
					mean_back_b += b;
				}else{
					mean_obj_r += r;
					mean_obj_g += g;
					mean_obj_b += b;
				}
			    }
			}
			mean_back_r = mean_back_r/4;
			mean_back_g = mean_back_g/4;
			mean_back_b = mean_back_b/4;
			mean_obj_r = mean_obj_r/((imgWidth*imgHeight)-4);
			mean_obj_g = mean_obj_g/((imgWidth*imgHeight)-4);
			mean_obj_b = mean_obj_b/((imgWidth*imgHeight)-4);
			t1_r = (mean_back_r + mean_obj_r)/2;
			t1_g = (mean_back_g + mean_obj_g)/2;
			t1_b = (mean_back_b + mean_obj_b)/2;
			printf("Initial background mean: [%f|%f|%f]\n",mean_back_r,mean_back_g,mean_back_b);
			printf("Initial object mean: [%f|%f|%f]\n",mean_obj_r,mean_obj_g,mean_obj_b);
			printf("Initial T_(t+1): [%f|%f|%f]\n",t1_r,t1_g,t1_b);
			
			//Iteration to find proper threshold for red
			while(1){
				mean_obj_r = 0;
				mean_back_r = 0;
				cnt_obj_r = 0;
				cnt_back_r = 0;
				t_r = t1_r;
				for( int i=startWidth;i<imgWidth;i++)
              			    for(int j=startHeight;j<imgHeight;j++){
					r = loadedImage->GetRed(i,j);
					//compute background and object mean over segmented image
					if(r >= t_r){
						mean_obj_r += r;
						cnt_obj_r += 1;
					}else if (r < t_r){
						mean_back_r += r;
						cnt_back_r += 1;
					}
				     }
				if(mean_back_r>0){mean_back_r = mean_back_r/cnt_back_r;}
				if(mean_obj_r>0){mean_obj_r = mean_obj_r/cnt_obj_r;}

				t1_r = (mean_back_r + mean_obj_r)/2;
				//Stop loop
				if(abs(t1_r - t_r)<t0_r){
					break;			
				}			
			}
			//green
			while(1){
				mean_obj_g = 0;
				mean_back_g = 0;
				cnt_obj_g = 0;
				cnt_back_g = 0;
				t_g = t1_g;
				for( int i=startWidth;i<imgWidth;i++)
              			    for(int j=startHeight;j<imgHeight;j++){
					g = loadedImage->GetGreen(i,j);
					//compute background and object mean over segmented image
					if(g >= t_g){
						mean_obj_g += g;
						cnt_obj_g += 1;
					}else if (g < t_g){
						mean_back_g += g;
						cnt_back_g += 1;
					}
				     }
				if(mean_back_g>0){mean_back_g = mean_back_g/cnt_back_g;}
				if(mean_obj_g>0){mean_obj_g = mean_obj_g/cnt_obj_g;}
				
				t1_g = (mean_back_g + mean_obj_g)/2;
				//Stop loop
				if(abs(t1_g - t_g)<t0_g){
					break;			
				}			
			}
			//blue
			while(1){
				mean_obj_b = 0;
				mean_back_b = 0;
				cnt_obj_b = 0;
				cnt_back_b = 0;
				t_b = t1_b;
				for( int i=startWidth;i<imgWidth;i++)
              			    for(int j=startHeight;j<imgHeight;j++){
					b = loadedImage->GetBlue(i,j);
					//compute background and object mean over segmented image
					if(b >= t_b){
						mean_obj_b += b;
						cnt_obj_b += 1;
					}else if (b < t_b){
						mean_back_b += b;
						cnt_back_b += 1;
					}
				     }
				if(mean_back_b>0){mean_back_b = mean_back_b/cnt_back_b;}
				if(mean_obj_b>0){mean_obj_b = mean_obj_b/cnt_obj_b;}
				t1_b = (mean_back_b + mean_obj_b)/2;
				//Stop loop
				if(abs(t1_b - t_b)<t0_b){
					break;			
				}			
			}
			//Apply threshold in order to segment image
			for( int i=startWidth;i<imgWidth;i++)
              		    for(int j=startHeight;j<imgHeight;j++){
				r = loadedImage->GetRed(i,j);
				g = loadedImage->GetGreen(i,j);
				b = loadedImage->GetBlue(i,j);
				if(r >= t1_r){
					r = 255;
				}else if (r < t1_r){
					r = 0;
				}
				if(g >= t1_g){
					g = 255;
				}else if (g < t1_g){
					g = 0;
				}
				if(b >= t1_b){
					b = 255;
				}else if (b < t1_b){
					b = 0;
				}
	
			 	loadedImage->SetRGB(i,j,r,g,b);
			    }
			printf("Finished thresholding\n");
			break;
/***************** Adaptive thresholding ******************/
		case 4:
			printf("adaptive thresholding...\n");
			number = wxGetTextFromUser(_T("Please enter the value of the variance."), _T(""), _T(""));
		    	if(number.ToDouble(&variance)){
				//assuming that the image is square, split image into 8x8 blocks (subimages)
				n_blocks = imgWidth/8*imgHeight/8;
				//convert to grayscale				
				for( int i=startWidth;i<imgWidth;i++)
              			    for(int j=startHeight;j<imgHeight;j++){
					r = loadedImage->GetRed(i,j);
					g = loadedImage->GetGreen(i,j);
					b = loadedImage->GetBlue(i,j);
					r = (r+g+b)/3;
					loadedImage->SetRGB(i,j,r,r,r);
				    }
				for( int i=startWidth/8;i<imgWidth/8;i++)
              			    for(int j=startHeight/8;j<imgHeight/8;j++){
					memset(histogram_r, 0, sizeof(histogram_r));
					
					//initialise normalised histogram
					memset(norm_histogram_r, 0, sizeof(norm_histogram_r));
					
					//number of pixels
					c = 0;
					mean = 0;
					var = 0;
					
					for(int x=0;x<8;x++){
					    for(int y=0;y<8;y++){
						//Find histogram values for each block
						histogram_r[loadedImage->GetRed(8*i+x,8*j+y)] += 1;
						c += 1; 
					    }
					}
					//Normalise histogram values for each block
					for (int h=0; h<256; h++){
						norm_histogram_r[h] = histogram_r[h] / c;
					}
				
					//find mean for each block
					for (int h=0; h<256; h++){
						mean += (h*norm_histogram_r[h]);
						
					}
					//find variance for each block
					for (int h=0; h<256; h++){
						var += pow((h-mean_r),2)*norm_histogram_r[h];
						
					}
					
					if(var<variance){
					//set block pixels with variance < n to obj or background (depending on mean)
						if(mean>=125){
							r = 255;
						}else{
							r = 0;					
						}					
						for(int x=0;x<8;x++){
						    for(int y=0;y<8;y++){
							loadedImage->SetRGB(8*i+x,8*j+y,r,r,r);
						    }
						}
					}else{//do automated (optimal) thresholding for a block with variance >= n
						//Initiation: assume that background is only 4 corners and object the others
						//find mean of obj and mean of background
						mean_obj = 0;
						mean_back = 0;
						for(int x=0;x<8;x++){
						    for(int y=0;y<8;y++){
							r = loadedImage->GetRed(8*i+x,8*j+y);
							g = loadedImage->GetGreen(8*i+x,8*j+y);
							b = loadedImage->GetBlue(8*i+x,8*j+y);
							
							if((8*i+x == 0 && 8*j+y == 0) 
							|| (8*i+x == 8-1 && 8*j+y == 0) 
							|| (8*i+x == 0 && 8*j+y == 8-1) 
							|| (8*i+x ==8-1 && 8*j+y == 8-1)){
								mean_back += (round((r+g+b)/3));
							}else{
								mean_obj += (round((r+g+b)/3));
							}
						    }
						}
						mean_back = mean_back/4;
						mean_obj = mean_obj/((8*8)-4);
						t1 = (mean_back + mean_obj)/2;
						//printf("Initial T_(t+1): %f\n",t1);
						while(1){
							mean_obj = 0;
							mean_back = 0;
							cnt_obj = 0;
							cnt_back = 0;
							t = t1;
							for(int x=0;x<8;x++)
						    	    for(int y=0;y<8;y++){
								r = loadedImage->GetRed(8*i+x,8*j+y);
								g = loadedImage->GetGreen(8*i+x,8*j+y);
								b = loadedImage->GetBlue(8*i+x,8*j+y);
								//average colors to get gray level
								r = round((r+g+b)/3);
								//compute background and object mean over segmented image
								if(r >= t){
									mean_obj += r;
									cnt_obj += 1;
								}else if (r < t){
									mean_back += r;
									cnt_back += 1;
								}
							     }
							if(mean_back>0){mean_back = mean_back/cnt_back;}
							if(mean_obj>0){mean_obj = mean_obj/cnt_obj;}
							t1 = (mean_back + mean_obj)/2;
							//Stop loop
							if(abs(t1 - t)<t0){
								break;			
							}			
						}
						//printf("Final T_(t+1): %f\n\n",t1);
						//Apply threshold in order to segment image
						for(int x=0;x<8;x++)
					    	    for(int y=0;y<8;y++){
							r = loadedImage->GetRed(8*i+x,8*j+y);
							g = loadedImage->GetGreen(8*i+x,8*j+y);
							b = loadedImage->GetBlue(8*i+x,8*j+y);
							if(((r+g+b)/3) >= t1){
								r = 255;
							}else if (((r+g+b)/3) < t1){
								r = 0;
							}
					
						 	loadedImage->SetRGB(8*i+x,8*j+y,r,r,r);
						    }	
					}
				
				    }
				
			}
			printf("Finished thresholding\n");
			break;
		default:
			wxMessageBox(_T("The number that you entered is in wrong format.\nPlease try again."));
			err=-1;
	}
	Refresh();
    }else{
	    wxMessageBox(_T("The number that you entered is in wrong format.\nPlease try again."));
    }
}

//###########################################################//
//-----------------------------------------------------------//
//----------------- HISTOGRAM EQUALISATION-------------------//
//-----------------------------------------------------------//
//###########################################################//

void MyFrame::OnHistogramEqualisationImage(wxCommandEvent & event){
    printf("Histogram equalisation...\n");
    free(loadedImage);
    loadedImage = new wxImage(bitmap.ConvertToImage());
    appendToUndo(*loadedImage); 
    //initialise histogram			
    int histogram_r[256] = {0};
    int histogram_g[256] = {0};
    int histogram_b[256] = {0};
    //initialise normalised histogram
    float norm_histogram_r[256] = {0};
    float norm_histogram_g[256] = {0};
    float norm_histogram_b[256] = {0};
    //number of pixels
    float n = 0;
    //initialise cumulative histogram
    float c_histogram_r[256] = {0};
    float c_histogram_g[256] = {0};
    float c_histogram_b[256] = {0};
    //helper accumulators for calculating cumulative histogram
    float c_accumulator_r =0;
    float c_accumulator_g =0;
    float c_accumulator_b =0;
    //initialise s-k, or transform
    int sk_r[256] = {0};
    int sk_g[256] = {0};
    int sk_b[256] = {0};
    //initialise tmp RGB values
    unsigned char r,g,b;
    /*Find histogram values*/
    for( int i=startWidth;i<imgWidth;i++)
	    for(int j=startHeight;j<imgHeight;j++){
		histogram_r[loadedImage->GetRed(i,j)] += 1;
		histogram_g[loadedImage->GetGreen(i,j)] += 1;
		histogram_b[loadedImage->GetBlue(i,j)] += 1;	
             	n += 1; 
	    }
    /*Normalise histogram values*/
    for (int i=0; i<256; i++){
 	printf("(hist. for r=%d: [%d|%d|%d], ", i, histogram_r[i], histogram_g[i], histogram_b[i]);
	norm_histogram_r[i] = histogram_r[i] / n;
	norm_histogram_g[i] = histogram_g[i] / n;
	norm_histogram_b[i] = histogram_b[i] / n;	
        printf("norm. hist.: [%f|%f|%f|])\n ", norm_histogram_r[i], norm_histogram_g[i], norm_histogram_b[i]);
    }
    /*Histogram equalisation*/
    //find the cumulative distribution
    for (int i=0; i<256; i++){
	c_accumulator_r += norm_histogram_r[i];
	c_accumulator_g += norm_histogram_g[i];
	c_accumulator_b += norm_histogram_b[i];
 	c_histogram_r[i] = c_accumulator_r;
	c_histogram_g[i] = c_accumulator_g;
	c_histogram_b[i] = c_accumulator_b;	
    }
    //multiply cumulative distribution by the max gray-level value (L-1=255) to find the transform
    for (int i=0; i<256; i++){
	sk_r[i] = round(c_histogram_r[i] * 255);
	sk_g[i] = round(c_histogram_g[i] * 255);
	sk_b[i] = round(c_histogram_b[i] * 255);	
    }
    //apply transform on original image
    for( int i=startWidth;i<imgWidth;i++)
	    for(int j=startHeight;j<imgHeight;j++){
		r = sk_r[loadedImage->GetRed(i,j)];
		g = sk_g[loadedImage->GetGreen(i,j)];
		b = sk_b[loadedImage->GetBlue(i,j)];	
             	loadedImage->SetRGB(i,j,r,g,b);
	    }
    /*Display updated image*/
    Refresh();
}

//###########################################################//
//-----------------------------------------------------------//
//------------------- HISTOGRAM DISPLAY----------------------//
//-----------------------------------------------------------//
//###########################################################//

void MyFrame::OnHistogramDisplayImage(wxCommandEvent & event){
    printf("Histogram equalisation...\n");
    free(loadedImage);
    loadedImage = new wxImage(bitmap.ConvertToImage());
    //initialise histogram			
    int histogram_r[256] = {0};
    int histogram_g[256] = {0};
    int histogram_b[256] = {0};
    //initialise normalised histogram
    float norm_histogram_r[256] = {0};
    float norm_histogram_g[256] = {0};
    float norm_histogram_b[256] = {0};
    //number of pixels
    float n = 0;
    //initialise tmp RGB values
    unsigned char r,g,b;
    /*Find histogram values*/
    for( int i=startWidth;i<imgWidth;i++)
	    for(int j=startHeight;j<imgHeight;j++){
		histogram_r[loadedImage->GetRed(i,j)] += 1;
		histogram_g[loadedImage->GetGreen(i,j)] += 1;
		histogram_b[loadedImage->GetBlue(i,j)] += 1;	
             	n += 1; 
	    }
    /*Normalise histogram values*/
    for (int i=0; i<256; i++){
 	printf("(hist. for r=%d: [%d|%d|%d], ", i, histogram_r[i], histogram_g[i], histogram_b[i]);
	norm_histogram_r[i] = histogram_r[i] / n;
	norm_histogram_g[i] = histogram_g[i] / n;
	norm_histogram_b[i] = histogram_b[i] / n;	
        printf("norm. hist.: [%f|%f|%f|])\n ", norm_histogram_r[i], norm_histogram_g[i], norm_histogram_b[i]);
    }
    
    long int choice;//for switch
   
    wxString number = wxGetTextFromUser(_T("Please choose which histogram to display .\n1  red\n2  green\n3  blue"), _T(""), _T(""));
    int err=0;
    if(number.ToLong(&choice)&&(choice==1||choice==2||choice==3)){
	//initialise the histogram
	loadedImage = new wxImage(256,256);
	for( int i=0;i<256;i++)
		for(int j=0;j<256;j++){
			loadedImage->SetRGB(i,j,255,255,255); 
	       	}
	switch(choice){
/*********** Choose red, green or blue histogram ************/
		case 1:	
			for( int k=0;k<256;k++)
				for(int j=0;j<=(norm_histogram_r[k]*255*10);j++){//multiplied by coefficient to see values more clearly
					loadedImage->SetRGB(j,k,0,0,0); 
			       	}
			break;
		case 2:	
			for( int k=0;k<256;k++)
				for(int j=0;j<=(norm_histogram_g[k]*255*10);j++){//multiplied by coefficient to see values more clearly
					loadedImage->SetRGB(j,k,0,0,0); 
			       	}
			break;
		case 3:	
			for( int k=0;k<256;k++)
				for(int j=0;j<=(norm_histogram_b[k]*255*10);j++){//multiplied by coefficient to see values more clearly
					loadedImage->SetRGB(j,k,0,0,0); 
			       	}
			break;
		default:
			wxMessageBox(_T("The number that you entered is in wrong format.\nPlease try again."));
			err=-1;
	}
    /*Display updated image*/
    if(err==0) Refresh();
    }else{
	    wxMessageBox(_T("The number that you entered is in wrong format.\nPlease try again."));
    }
    
}

//###########################################################//
//-----------------------------------------------------------//
//---------- DO NOT MODIFY THE CODE BELOW--------------------//
//-----------------------------------------------------------//
//###########################################################//


//IMAGE SAVING
void MyFrame::OnSaveImage(wxCommandEvent & event){

    printf("Saving image...");
    free(loadedImage);
    loadedImage = new wxImage(bitmap.ConvertToImage());

    loadedImage->SaveFile(wxT("Saved_Image.bmp"), wxBITMAP_TYPE_BMP);

    printf("Finished Saving.\n");
}


void MyFrame::OnExit (wxCommandEvent & event){
  Close(TRUE);
}


void MyFrame::OnPaint(wxPaintEvent & event){
  wxPaintDC dc(this);
  int cWidth, cHeight;  
  GetSize(&cWidth, &cHeight);
  if ((back_bitmap == NULL) || (oldWidth != cWidth) || (oldHeight != cHeight)) {
    if (back_bitmap != NULL)
      delete back_bitmap;
    back_bitmap = new wxBitmap(cWidth, cHeight);
    oldWidth = cWidth;
    oldHeight = cHeight;
  }
  wxMemoryDC *temp_dc = new wxMemoryDC(&dc);
  temp_dc->SelectObject(*back_bitmap);
  // We can now draw into the offscreen DC...
  temp_dc->Clear();
  if(loadedImage)
    temp_dc->DrawBitmap(wxBitmap(*loadedImage), 0, 0, false);//given bitmap xcoord y coord and transparency
       
  switch(stuffToDraw){
     case NOTHING:
        break;
     case ORIGINAL_IMG:
       bitmap.CleanUpHandlers; // clean the actual image header
       bitmap = wxBitmap(*loadedImage); // Update the edited/loaded image
       break;
  }

// update image size
imgWidth  = (bitmap.ConvertToImage()).GetWidth();
imgHeight = (bitmap.ConvertToImage()).GetHeight();

 temp_dc->SelectObject(bitmap);//given bitmap 

  //end draw all the things
  // Copy from this DC to another DC.
  dc.Blit(0, 0, cWidth, cHeight, temp_dc, 0, 0);
  delete temp_dc; // get rid of the memory DC  
}

BEGIN_EVENT_TABLE (MyFrame, wxFrame)
  EVT_MENU ( LOAD_FILE_ID,  MyFrame::OnOpenFile)
  EVT_MENU ( EXIT_ID,  MyFrame::OnExit)

//###########################################################//
//----------------------START MY EVENTS ---------------------//
//###########################################################// 
  EVT_MENU ( ROI_ON_ID,  MyFrame::OnROIOn)
  EVT_MENU ( UNDO_ID,  MyFrame::OnUndo)
  EVT_MENU ( INVERT_IMAGE_ID,  MyFrame::OnInvertImage)
  EVT_MENU ( SCALE_IMAGE_ID,  MyFrame::OnScaleImage)
  EVT_MENU ( SHIFT_IMAGE_ID, MyFrame::OnShiftImage)
  EVT_MENU ( SAVE_IMAGE_ID,  MyFrame::OnSaveImage)
  EVT_MENU ( LOAD_RAW_ID,  MyFrame::OnOpenRawFile)
  EVT_MENU ( CONVOLUTE_IMAGE_ID,  MyFrame::OnConvoluteImage)
  EVT_MENU ( OSF_IMAGE_ID,  MyFrame::OnOSFImage)
  EVT_MENU ( POINT_PROCESSING_IMAGE_ID,  MyFrame::OnPointProcessingImage)
  EVT_MENU ( THRESHOLDING_IMAGE_ID,  MyFrame::OnThresholdingImage)
  EVT_MENU ( HISTOGRAM_EQUALISATION_IMAGE_ID,  MyFrame::OnHistogramEqualisationImage)
  EVT_MENU ( HISTOGRAM_DISPLAY_IMAGE_ID,  MyFrame::OnHistogramDisplayImage)//--->To be modified!
//###########################################################//
//----------------------END MY EVENTS -----------------------//
//###########################################################// 

  EVT_PAINT (MyFrame::OnPaint)
END_EVENT_TABLE()
