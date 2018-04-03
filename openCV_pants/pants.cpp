#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/imgcodecs.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <JsonBox.h>

using namespace JsonBox;

// define matrix for picture
IplImage* image = 0;

IplImage* gray_picture = 0;

// variables
double unit = 0.;

// Object size in mm 90*90
double square_width = 90.;
double square_high = 90.;

// define colors
// [R, G, B]
const CvScalar WHITE = CV_RGB(255, 255, 255);
const CvScalar BLACK = CV_RGB(0, 0, 0);
const CvScalar YELLOW1 = CV_RGB(22, 50, 50);
const CvScalar YELLOW2 = CV_RGB(42, 255, 255);
const CvScalar RED = CV_RGB(255, 0, 0);
const CvScalar BLUE = CV_RGB(0, 0, 255);

std::vector<CvPoint>* getIntersections(IplImage* img, CvPoint a, CvPoint b, CvSeq* contour, int treshold = 10, int linewidth = 2){
	  IplImage *img_contours = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 1);
	cvSet(img_contours, 0);
	cvDrawContours(img_contours,
		   contour,
		   WHITE,
		   BLACK,
    	   255,linewidth , CV_AA, 0);
	IplImage *img_line = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 1);
	cvSet(img_line, 0);
	
	cvLine(img_line,
      a,
      b,
      WHITE,
     linewidth, CV_AA, 0);
	
	cv::Mat res;
	std::vector<CvPoint> *pointst = new std::vector<CvPoint>();
	bitwise_and(cv::cvarrToMat(img_contours),cv::cvarrToMat(img_line), res); 
	cvSaveImage("output/result3.jpg", img_line);
	cvSaveImage("output/result4.jpg", img_contours);
	imwrite("output/result.jpg",res);
	for(int i =0 ;i <res.rows;i++)
		 {
			 for(int j=0; j<res.cols; j++)
			 {
				int rv = res.at<uchar>(i,j);
			   if( rv > 0)
			   {

				   pointst->push_back(CvPoint(j,i));
				   

			   }
				 
				 
				 

			 }
		 }
	
	std::vector<CvPoint> *points = new std::vector<CvPoint>();
	for(std::vector<CvPoint>::iterator it = pointst->begin(); it < pointst->end(); it++){
		CvPoint p = CvPoint((*it).x, (*it).y);
		CvPoint ptmp = CvPoint((*it).x, (*it).y);
		int counter = 1;
		
		for(std::vector<CvPoint>::iterator it2 = pointst->begin(); it2 < pointst->end(); it2++){
			CvPoint diff = CvPoint(p.x - (*it2).x,p.y-(*it2).y);
			
              if (cv::sqrt(diff.x*diff.x + diff.y*diff.y) < treshold){
				  ptmp =CvPoint(ptmp.x + (*it2).x,ptmp.y+(*it2).y);
				  counter++;
				  pointst->erase(it2);
				  it2--;
			  }
		}
		
		ptmp =CvPoint(ptmp.x / counter,ptmp.y/counter);
		points->push_back(ptmp);
		std::cout<<ptmp.x<<" "<<ptmp.y<<std::endl;
	}
	delete pointst;
	return points;

}

CvPoint contourIterate(CvSeq contour, CvPoint point, float num){
	CvPoint result = new CvPoint(0,0);
		return result;
}

int main(int argc, char* argv[])
{
  
  // Load an image
  const char* filename = argc >= 2 ? argv[1] : "test1.jpg";
  image = cvLoadImage(filename, 1);
  printf("[i] image: %s\n", filename);
  assert(image != 0);

  // Create images 
  // 1 channel for GrayScale 
  gray_picture = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 1);

  /// ---------------------------------- Select yellow ---------------------------
  // confert to HSB and select yellow color
  cvCvtColor(image,
    gray_picture,
    //CV_RGB2HSV);
    CV_RGB2GRAY);

  // set ROI
  cvSetImageROI(gray_picture, 
    cvRect(
      (gray_picture -> width) * 0.7,
      0,
      gray_picture->width,
      (gray_picture->height) * 0.3)
  );

  // create image with ROI
  // cvGetSize will return the width and the height of ROI 
  IplImage *img_roi = cvCreateImage(cvGetSize(gray_picture), IPL_DEPTH_8U, 1);

  // copy subimage 
  cvCopy(gray_picture, img_roi, NULL);

  // reset the Region of Interest
  cvResetImageROI(gray_picture);

  // split HSV to different picture
  //cvSplit(yellow_picture, NULL, NULL, NULL, gray_picture);

  /// select color
  // Photoshop measurements
  // Photoshop range Hue [0,360], Saturation [0,100], Value [0,100]
  // H 49-58, S 9-13%, V 95-98%
  // OpenCV range Hue [0,179], Saturation [0,255], Value [0,255]
  // Recommended range
  // Range +-10 H, 50-255 S, 50-255 V
  // H [22,32] S [50,255], V [50,255]
  cvInRangeS(img_roi,     
    220,     
    250,     
    img_roi);
  /// ---------------------------------- End Select yellow -----------------------
  
  /// ---------------------------------- Canny -----------------------------------
  // Smoothing with Gaussian filter
  cvSmooth(img_roi,
    img_roi,
    2, 3,
    0, 0, 0);

  //canny on gray picture
  cvCanny(img_roi,
    img_roi,
    10,
    100,
    3);

  cvDilate(img_roi,
    img_roi,
    0, 1);

  cvErode(img_roi,
    img_roi,
    0, 1);
  /// ---------------------------------- End Canny -------------------------------

  /// --------------------------- yellow Rectangle -------------------------------
  // Find longest contour 
  CvMemStorage* storageContour = cvCreateMemStorage(0);
  CvSeq* contours = 0;

  cvFindContours(img_roi,
    storageContour,
    &contours,
    sizeof(CvContour),
    CV_RETR_LIST,
    CV_CHAIN_APPROX_NONE,
    cvPoint(0, 0));

  CvSeq* seqT = 0;
  double perimT = 0;

  if (contours != 0) {
    // find longest contour
    for (CvSeq* seq0 = contours; seq0 != 0; seq0 = seq0->h_next) {
      double perim = cvContourPerimeter(seq0);
      //      printf("%f\n", perim);
      if (perim>perimT) {
        perimT = perim;
        seqT = seq0;
      }
    }
  }
     
  // Draw contour 
  cvDrawContours(img_roi,
    seqT,
    RED,
    RED,
    0, 10, 8, 0);

  // Draw rectagle
  printf("\n");
  printf("Yellow Rectangle corner\n");

  CvBox2D rect;
  CvPoint2D32f rect_vtx[4];
  CvPoint pt, pt0;

  rect = cvMinAreaRect2(seqT, 0);
  cvBoxPoints(rect, rect_vtx);

  pt0.x = cvRound(rect_vtx[3].x);
  pt0.y = cvRound(rect_vtx[3].y);
  int i = 0;
  for (i = 0; i < 4; i++) 
  {


    printf("rect.x = ");
    printf("%d", pt0.x);

    printf("    rect.y = ");
    printf("%d\n", pt0.y);

    pt.x = cvRound(rect_vtx[i].x);
    pt.y = cvRound(rect_vtx[i].y);
    
    //draw blue rectagle around yellow rectangle
    cvLine(img_roi,
      pt0,
      pt,
      BLUE,
     20, CV_AA, 0);
    pt0 = pt;
  }

  /// --------------------------- End yellow rectangle ---------------------------

  /// ------------------------------- pixels_per_metric --------------------------
  // Calculate points of rectagle
  double length = 0.;
  double length_tmp = 0.;

  printf("\n");
  printf("Rectangle sides length \n");

  for (i = 0; i < 4; i++)
  {
    if (i < 3)
    {
      length_tmp = sqrt(pow((rect_vtx[i + 1].x - rect_vtx[i].x), 2) +
        pow((rect_vtx[i + 1].y - rect_vtx[i].y), 2));

      printf("length = ");
      printf("%f\n", length_tmp);

      length = length + length_tmp;
    }
    else
    {
      length_tmp = sqrt(pow((rect_vtx[0].x - rect_vtx[3].x), 2) +
        pow((rect_vtx[0].y - rect_vtx[3].y), 2));

      printf("length = ");
      printf("%f\n", length_tmp);

      length = length + length_tmp;
    }
  }

  length = length / 4.;

  printf("Average length = ");
  printf("%f\n", length);
  printf("\n");

  unit = length / square_high * 25.4;

  printf("units = ");
  printf("%f", unit);
  printf(" pixel/inch\n");
  printf("\n");
  
  /// --------------------------- end pixels_per_metric --------------------------

  /// ---------------------------------- Canny -----------------------------------
  // threshold for transfer color picture into gray
  cvThreshold(gray_picture,
    gray_picture,
    150,
    255,
    3);

  cvDilate(gray_picture,
    gray_picture,
    0, 2);

  cvErode(gray_picture,
    gray_picture,
    0, 2);

  // Smoothing with Gaussian filter
  cvSmooth(gray_picture,
    gray_picture,
    2, 3,
    0, 0, 0);

  //canny on gray picture
  cvCanny(gray_picture,
    gray_picture,
    10,
    100,
    3);

  cvDilate(gray_picture,
    gray_picture,
    0, 2);

  cvErode(gray_picture,
    gray_picture,
    0, 2);

  cvDilate(gray_picture,
    gray_picture,
    0, 2);

  cvErode(gray_picture,
    gray_picture,
    0, 2);
  /// ---------------------------------- End Canny -------------------------------

  /// ------------------------- finding pant contour ----------------------------
  // Find longest contour 
  CvMemStorage* storageContourT = cvCreateMemStorage(0);
  contours = 0;

  cvFindContours(gray_picture,
    storageContourT,
    &contours,
    sizeof(CvContour),
    CV_RETR_LIST,
    CV_CHAIN_APPROX_NONE,
    cvPoint(0, 0));

  seqT = 0;
  perimT = 0;

  if (contours != 0) {
    // find longest contour
    for (CvSeq* seq0 = contours; seq0 != 0; seq0 = seq0->h_next) {
      double perim = cvContourPerimeter(seq0);
      if (perim>perimT) {
        perimT = perim;
        seqT = seq0;
      }
    }
  }
  // Draw contour 
  cvDrawContours(gray_picture,
    seqT,
    WHITE,
    WHITE,
    0, 10, 8, 0);

  /// --------------------- end finding pant contour ----------------------------


  /// --------------------- approximate pant ------------------------------------

  // find approximation polynome
  CvSeq* result = 0;
  result = cvApproxPoly(seqT,
    sizeof(CvContour),
    storageContourT,
    CV_POLY_APPROX_DP,
    20, 0);

  // change line thickness to -1 for contour filling 
  // Draw approximation contour 
  cvDrawContours(image,
    result,
    RED,
    RED,
    0, 20, 8, 0);

  int maxCorners = 10;
  double qualityLevel = 0.01;
  double minDistance = 20.;
  int blockSize = 3;
  double k = 0.04;

  printf("Longest contour, App poly points %d %d\n", seqT->total, result->total);
	std::vector<CvPoint>* points = getIntersections(gray_picture, CvPoint(0,0),CvPoint(gray_picture->width, gray_picture->height), seqT, 15, 2);
	std::cout<<"intersections count "<<points->size()<<std::endl;
	std::vector<CvPoint>::iterator point = points->begin();
	cvLine(gray_picture,
      CvPoint(0,0),
      CvPoint(gray_picture->width, gray_picture->height),
      WHITE,
     2, CV_AA, 0);
	cvCircle(gray_picture, *point, 50, WHITE, 5, 8, 0);
	point++;
	cvCircle(gray_picture, *point, 50, WHITE, 5, 8, 0);
	 cvSaveImage("output/result2.jpg", gray_picture);
  /*
  // convexHull
  CvSeq* hull2 = 0;
  hull2 = cvConvexHull2(result, 0, CV_CLOCKWISE, 1);

  printf("App poly, Hull points %d %d\n", result->total, hull2->total);

  // Draw contour 
  cvDrawContours(image,
    hull2,
    BLACK,
    BLACK,
    0, 20, 8, 0);

  CvPoint pt10;

  CvMemStorage* storage1 = cvCreateMemStorage(0);
  CvMemStorage* storage2 = cvCreateMemStorage(0);
  CvSeq* ptseq = cvCreateSeq(CV_SEQ_KIND_GENERIC | CV_32SC2, sizeof(CvContour),
    sizeof(CvPoint), storage1);
  CvSeq* hull;
  CvSeq* defects;

  // change approximation contour structure
  for (int i = 0; i < result->total; i++)
  {
    CvPoint* p = CV_GET_SEQ_ELEM(CvPoint, result, i);
    pt10.x = p->x;
    pt10.y = p->y;
    cvSeqPush(ptseq, &pt10);
  }

  // make convex hull
  hull = cvConvexHull2(ptseq, 0, CV_CLOCKWISE, 0);
  int hullcount = hull->total;

  //find defects
  defects = cvConvexityDefects(ptseq, hull, storage2);

  //printf(" defect no %d \n", defects->total);

  CvConvexityDefect* defectArray;

  int j = 0;
  //int m_nomdef=0;
  // This cycle marks all defects of convexity of current contours.  
  for (; defects; defects = defects->h_next)
  {
    int nomdef = defects->total; // defect amount  
                                 //outlet_float( m_nomdef, nomdef );  

                                 //printf(" defect no %d \n",nomdef);

    if (nomdef == 0)
      continue;

    // Alloc memory for defect set.     
    //fprintf(stderr,"malloc\n");  
    defectArray = (CvConvexityDefect*)malloc(sizeof(CvConvexityDefect)*nomdef);

    // Get defect set.  
    //fprintf(stderr,"cvCvtSeqToArray\n");  
    cvCvtSeqToArray(defects, defectArray, CV_WHOLE_SEQ);

    // Draw marks for all defects.  
    for (int i = 0; i < nomdef; i++)
    {
      //printf(" defect depth for defect %d %f \n", i, defectArray[i].depth);
      cvLine(image, *(defectArray[i].start), *(defectArray[i].depth_point), WHITE, 10, CV_AA, 0);
      cvCircle(image, *(defectArray[i].depth_point), 10, WHITE, 10, 8, 0);
      cvCircle(image, *(defectArray[i].start), 10, WHITE, 10, 8, 0);
      cvLine(image, *(defectArray[i].depth_point), *(defectArray[i].end), WHITE, 10, CV_AA, 0);
    }
  }
  */
  /// ----------------- end approximate pants ------------------------------------
    
  /// ----------------- draw min area rect around pants --------------------------
  CvBox2D rect_pants;
  CvPoint2D32f rect_pants_vtx[4];
  // CvPoint pt, pt0;

  rect_pants = cvMinAreaRect2(seqT, 0);
  cvBoxPoints(rect_pants, rect_pants_vtx);

  pt0.x = cvRound(rect_pants_vtx[3].x);
  pt0.y = cvRound(rect_pants_vtx[3].y);
  for (i = 0; i < 4; i++)
  {

    printf("rect_pants.x = ");
    printf("%d", pt0.x);

    printf("    rect_pants.y = ");
    printf("%d\n", pt0.y);

    pt.x = cvRound(rect_pants_vtx[i].x);
    pt.y = cvRound(rect_pants_vtx[i].y);
    cvLine(image,
      pt0,
      pt,
      BLACK,
      10, CV_AA, 0);
    pt0 = pt;
  }


  /// ----------------- end draw min area rect around pant ----------------------


  /// -------------------------------- Display windows ---------------------------
  // resize picture

  //cvSaveImage("src.jpg", image, 0);

  //cvSaveImage("yellow.jpg", yellow_picture, 0);
	Value root;
	root["waistband"] = Value(0);
	root["crotch_length"] = Value(0);
	root["crotch_length_front"]= Value(0);
	root["crotch_length_rear"]= Value(0);
	root["inseam_left"] = Value(0);
	root["thigh_girth_left_horizontal"] = Value(0);
	root["knee_girth_left"] = Value(0);
	root["dist_waistband_knee"] = Value(0);
	root["crotch_knee_height"] = Value(0);
	root["sideseam_waist_left"] = Value(0);
	std::cout<<root;
	

  // Create a window to display results
  cvNamedWindow("original", CV_WINDOW_NORMAL);
  cvNamedWindow("yellow", CV_WINDOW_NORMAL);
  cvNamedWindow("gray", CV_WINDOW_NORMAL);
  cvNamedWindow("roi", CV_WINDOW_NORMAL);

  // Show images
  cvShowImage("original", image);
  cvShowImage("gray", gray_picture);
  cvShowImage("roi", img_roi);


  // --------------------------------- Clear memory ------------------------------
  // От кого я особенно ненавижу - так ето вас всех!
  // Kill them all!

  // Wait until user finishes program
  cvWaitKey(0);

  // release memory
  cvReleaseImage(&image);
  cvReleaseImage(&gray_picture);
  cvReleaseImage(&img_roi);

  // Delete windows
  cvDestroyAllWindows();
  return 0;
}
