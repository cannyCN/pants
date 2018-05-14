#include <cv.h>
#include <highgui.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <vector>
#include <string>

#include <iostream>
#include <opencv2/imgcodecs.hpp>

int DEBUG = 0;

// define matrix for picture
IplImage* image = 0;
IplImage* gray_picture = 0;
IplImage *img_roi = 0;
IplImage* HSB_picture = 0;

// variables
double unit = 0.;

// Object size in inch 3*3
double square_width = 3 * 25.4;
double square_high = 3 * 25.4;
char tmpstr[500];

// define colors
// [R, G, B]
const CvScalar WHITE = CV_RGB(255, 255, 255);
const CvScalar BLACK = CV_RGB(0, 0, 0);
const CvScalar YELLOW1 = CV_RGB(22, 50, 50);
const CvScalar YELLOW2 = CV_RGB(42, 255, 255);
const CvScalar RED = CV_RGB(255, 0, 0);
const CvScalar BLUE = CV_RGB(0, 0, 255);

char *replace_str(char *str, const char *orig, const char *rep) {
  static char buffer[4096];
  char *p;

  if(!(p = strstr(str, orig)))  // Is 'orig' even in 'str'?
    return str;

  strncpy(buffer, str, p-str); // Copy characters from 'str' start to 'orig' st$
  buffer[p-str] = '\0';

  sprintf(buffer+(p-str), "%s%s", rep, p+strlen(orig));

  return buffer;
}

int debug_printf(int is_debug, const char* format, ...) {
  if (is_debug) {
    va_list args;
    va_start (args, format);
    int res = vprintf (format, args);
    va_end (args);

    return res;
  } else {
    return 0;
  }
}

void printOnIm(IplImage* im, char* text, CvPoint* point, int size=6){
  CvFont font;
  double hScale=size;
  double vScale=size;
  int    lineWidth=10;
  cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, hScale,vScale,0,lineWidth);

  cvPutText (im, text, *point, &font, cvScalar(255,255,255));
}
/// ---------------------- calculate length ot contour segment -----------------
CvPoint contourIterate(CvSeq* contour, CvPoint point, int lenght, int CW) {

  // result - work contour
  // pointStart - point in contour from which we start
  // lenght -  current length of segment in pixels
  // PointerContour - index of contour points
  // CW - direction of contour. CW - clockwise
  CvSeq* result = contour;
  CvPoint pointStart = (0, 0);

  // set pointStart value
  pointStart.x = point.x;
  pointStart.y = point.y;

  // internal variable
  // lengthTmp - current segment lenght
  int lengthTmp = 0;
  int CountourStep = 0;

  // clockwise or opposite
  if (CW == 1)
  {
    CountourStep = 1;
  }
  else
  {
    CountourStep = -1;
  }

  // find most start point
  CvPoint* pointTmp;
  int pointIndex;
  for (int i = 0; i < result->total; i++)
  {
    pointTmp = CV_GET_SEQ_ELEM(CvPoint, result, i);
    if ((pointTmp->x) == (pointStart.x))
    {
      if ((pointTmp->y) == (pointStart.y))
      {
        pointIndex = i;
      }
    }
  }

  // draw start point circle
  pointTmp = CV_GET_SEQ_ELEM(CvPoint, result, pointIndex);
  pointStart.x = pointTmp->x;
  pointStart.y = pointTmp->y;

  cvCircle(image,
    pointStart,
    50,
    WHITE,
    5, 8, 0);

  ///
    CvPoint ttt;
  ///

  pointIndex = pointIndex + CountourStep;
  int pointFinish = 0;
  // find contour segment
  while ((lengthTmp < lenght) && (pointFinish == 0))
  {
    // read next point
    pointTmp = CV_GET_SEQ_ELEM(CvPoint, result, pointIndex);

    //printf("pointIndex = ");
    //printf("%d", pointIndex);
    //printf("\n");

    /// draw line
    ttt.x = pointTmp->x;
    ttt.y = pointTmp->y;
    cvLine(image, pointStart, ttt, WHITE, 15, 8, 0);
    ///

    // calculate part of contour lehgtn
    int lengthPart = 0;
    lengthPart = cvRound(sqrt(pow((pointStart.x - (pointTmp->x)), 2) +
      pow((pointStart.y - (pointTmp->y)), 2)));

    // calculate current segment length
    if ((lengthTmp + lengthPart) > lenght)
      // calculate part of current part of contour
    {
      // calculate line equation
      // line from pointStart to pointTmp

      // calculate length of current part
      // as % from curent part
      //       float j = 0.0;
      float j = static_cast <float> ((lenght - lengthTmp)) / lengthPart;
      lengthPart = lenght - lengthTmp;

      // calculate new pointTmp
      pointTmp->x = cvRound(pointStart.x - j*(pointStart.x - (pointTmp->x)));
      pointTmp->y = cvRound(pointStart.y - j*(pointStart.y - (pointTmp->y)));

      // calculate new length of current part
      lengthPart = cvRound(sqrt(pow((pointStart.x - (pointTmp->x)), 2) +
        pow((pointStart.y - (pointTmp->y)), 2)));

      pointFinish = 1;
    }
    // add current part of contour to segment length
    lengthTmp = lengthTmp + lengthPart;

    pointStart.x = pointTmp->x;
    pointStart.y = pointTmp->y;
    pointIndex = pointIndex + CountourStep;
  }

  // final point in pointStart

  // draw finish point circle
  cvCircle(image,
    pointStart,
    50,
    RED,
    15, 8, 0);

  return pointStart;
}
/// ------------------ end calculate length ot contour segment -----------------

/// ------------------ calculate intersection line and contour -----------------
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
    debug_printf(DEBUG, "%d %d\n", ptmp.x, ptmp.y);
	}
	delete pointst;
	return points;
}
/// -------------- end calculate intersection line and contour -----------------

int contourSegmentLenght(CvSeq* contour, CvPoint Start, CvPoint Finish, int CW) {

	// calculate length of contour from pointStart to pointFinish
	// cont - work contour
	// pointStart - point in contour from which we start
	// pointFinish - point in contour from which we start
	// CW - direction of contour. CW - clockwise = 1
	CvSeq* cont = contour;
	CvPoint pointStart = (0, 0);
	CvPoint pointFinish = (0, 0);

	// set points value
	if (CW > 0)
	{
		pointStart.x = Start.x;
		pointStart.y = Start.y;
		pointFinish.x = Finish.x;
		pointFinish.y = Finish.y;
	}
	else
	{
		pointStart.x = Finish.x;
		pointStart.y = Finish.y;
		pointFinish.x = Start.x;
		pointFinish.y = Start.y;
	}

	CW = 1;

	// internal variable
	// lengthTmp - current segment lenght
	int lengthTmp = 0;
	int CountourStep = CW;

	// find start point
	CvPoint* pointTmp;
	int pointStartIndex = 0;
	for (int i = 0; i < cont->total; i++)
	{
		pointTmp = CV_GET_SEQ_ELEM(CvPoint, cont, i);
		if ((pointTmp->x) == (pointStart.x))
		{
			if ((pointTmp->y) == (pointStart.y))
			{
				pointStartIndex = i;
			}
		}
	}

	// draw start point circle
	cvCircle(image,
		pointStart,
		50,
		BLACK,
		5, 8, 0);

	// find finish point
	int pointFinishIndex = 0;
	for (int i = 0; i < cont->total; i++)
	{
		pointTmp = CV_GET_SEQ_ELEM(CvPoint, cont, i);
		if ((pointTmp->x) == (pointFinish.x))
		{
			if ((pointTmp->y) == (pointFinish.y))
			{
				pointFinishIndex = i;
			}
		}
	}

	// draw finish point circle
	cvCircle(image,
		pointFinish,
		50,
		WHITE,
		5, 8, 0);

	///
	CvPoint ttt;
	///

	// find contour segment
	for (pointStartIndex = pointStartIndex + CountourStep; pointStartIndex <= pointFinishIndex; pointStartIndex = pointStartIndex + CountourStep)
	{
		// read next point
		pointTmp = CV_GET_SEQ_ELEM(CvPoint, cont, pointStartIndex);

		//printf("pointIndex = ");
		//printf("%d", pointIndex);
		//printf("\n");

		/// draw line
		ttt.x = pointTmp->x;
		ttt.y = pointTmp->y;
		cvLine(image, pointStart, ttt, WHITE, 15, 8, 0);
		///

		// calculate part of contour lehgtn
		int lengthPart = 0;
		lengthPart = cvRound(sqrt(pow((pointStart.x - (pointTmp->x)), 2) +
			pow((pointStart.y - (pointTmp->y)), 2)));

		// add current part of contour to segment length
		lengthTmp = lengthTmp + lengthPart;

		pointStart.x = pointTmp->x;
		pointStart.y = pointTmp->y;
	}
	return lengthTmp;
}

/// -------------------- end segment length function -----------------------

int main(int argc, char* argv[])
{
  int file_arg = 1;
  if (argv[1][0] == '-' && argv[1][1] == 'd') {
    DEBUG = 1;
    file_arg = 2;
  }

  // Load an image
  char* filename = argc >= 2 ? argv[file_arg] : (char*)"test.jpg";
  image = cvLoadImage(filename, 1);
  debug_printf(DEBUG, "[i] image: %s\n", filename);
  assert(image != 0);

  /// Create images
  // 1 channel for GrayScale
  gray_picture = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 1);
  // 1 image for ROI
  img_roi = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 1);
  // 3 channels image for HSB
  HSB_picture = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 3);

  /// ---------------------------------- Select yellow ---------------------------
  // confert to HSB and select yellow color
  cvCvtColor(image,
    gray_picture,
    //CV_RGB2HSV);
    CV_RGB2GRAY);

  // copy image
  cvCopy(gray_picture, img_roi, NULL);

  // set ROI
  cvSetImageROI(img_roi,
    cvRect(0, 0,
      cvRound((img_roi->width) * 0.7), (img_roi->height))
  );

  // set ROI to zero
  cvSet(img_roi, 0, img_roi);
  cvResetImageROI(img_roi);

  // set ROI
  cvSetImageROI(img_roi,
    cvRect(0, cvRound((gray_picture->height) * 0.3),
    (img_roi->width), (img_roi->height))
  );

  // set ROI to zero
  cvSet(img_roi, 0, img_roi);
  cvResetImageROI(img_roi);

  // use grayscale
  cvInRangeS(img_roi,
    180,
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
  debug_printf(DEBUG, "\n");
  debug_printf(DEBUG, "Yellow Rectangle corner\n");

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


    debug_printf(DEBUG, "rect.x = ");
    debug_printf(DEBUG, "%d", pt0.x);

    debug_printf(DEBUG, "    rect.y = ");
    debug_printf(DEBUG, "%d\n", pt0.y);

    pt.x = cvRound(rect_vtx[i].x);
    pt.y = cvRound(rect_vtx[i].y);

    //draw blue rectagle around yellow rectangle
    cvLine(image,
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

  debug_printf(DEBUG, "\n");
  debug_printf(DEBUG, "Rectangle sides length \n");

  for (i = 0; i < 4; i++)
  {
    if (i < 3)
    {
      length_tmp = sqrt(pow((rect_vtx[i + 1].x - rect_vtx[i].x), 2) +
        pow((rect_vtx[i + 1].y - rect_vtx[i].y), 2));

      debug_printf(DEBUG, "length = ");
      debug_printf(DEBUG, "%f\n", length_tmp);

      length = length + length_tmp;
    }
    else
    {
      length_tmp = sqrt(pow((rect_vtx[0].x - rect_vtx[3].x), 2) +
        pow((rect_vtx[0].y - rect_vtx[3].y), 2));

      debug_printf(DEBUG, "length = ");
      debug_printf(DEBUG, "%f\n", length_tmp);

      length = length + length_tmp;
    }
  }

  length = length / 4.;

  debug_printf(DEBUG, "Average length = ");
  debug_printf(DEBUG, "%f\n", length);
  debug_printf(DEBUG, "\n");

  unit = length / square_high * 25.4;

  debug_printf(DEBUG, "units = ");
  debug_printf(DEBUG, "%f", unit);
  debug_printf(DEBUG, " pixel/inch\n");
  debug_printf(DEBUG, "\n");

  /// --------------------------- end pixels_per_metric --------------------------

  /// ---------------------------- find background -------------------------------

  // confert to HSB and select yellow color
  cvCvtColor(image,
    HSB_picture,
    CV_RGB2HSV);

  // split HSV to different picture
  //cvSplit(HSB_picture, img_roi, NULL, NULL, NULL);

  /// select color
  // Photoshop measurements for background
  // H 133-139, S 74-86%, V 56-65%
  // Photoshop range Hue [0,360], Saturation [0,100], Value [0,100]
  // OpenCV range Hue [0,179], Saturation [0,255], Value [0,255]
  // Recommended range
  // Range +-10 H, 50-255 S, 50-255 V
  // H [60,80] S [50,255], V [50,255]
  cvInRangeS(HSB_picture,
    cvScalar(40, 50, 50),
    cvScalar(80, 255, 255),
    gray_picture);

  // fill yellow square with white
  // use yellow square contour
  cvDrawContours(gray_picture,
    seqT,
    WHITE,
    WHITE,
    0, -1, 8, 0);

  cvDrawContours(gray_picture,
    seqT,
    WHITE,
    WHITE,
    0, 50, 8, 0);

  //
  cvDilate(gray_picture,
    gray_picture,
    0, 1);

  cvErode(gray_picture,
    gray_picture,
    0, 1);

  /// --------------------------- end find background ----------------------------

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
  // contour direction - clockwise (CW)
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

  debug_printf(DEBUG, "Longest contour, App poly points %d %d\n", seqT->total, result->total);
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
    pt.x = cvRound(rect_pants_vtx[i].x);
    pt.y = cvRound(rect_pants_vtx[i].y);

    debug_printf(DEBUG, "rect_pants.x = ");
    debug_printf(DEBUG, "%d", pt.x);

    debug_printf(DEBUG, "    rect_pants.y = ");
    debug_printf(DEBUG, "%d\n", pt.y);

    cvLine(image,
      pt0,
      pt,
      BLACK,
      10, CV_AA, 0);
    pt0 = pt;
  }


  /// ----------------- end draw min area rect around pant ----------------------

  /// ----------------- find left upper corner of min area rect-----------------
  pt0.x = cvRound(rect_pants_vtx[0].x);
  pt0.y = cvRound(rect_pants_vtx[0].y);
  // first corner - most right and lower - ***
  //  +------+
  //  |      |
  //  |      |
  //  |      |
  //  |      |
  //  +------***

  // find most left and upper corner

  // corner's sum of coordinates
  // first value - sum of first corner coordinates
  double point_sum = cvRound(rect_pants_vtx[0].x) + cvRound(rect_pants_vtx[0].y);

  // index of left and upper corner
  int corner_l_u = 0;
  // index of right and upper corner
  int corner_r_u = 0;

  // find most left and upper corner
  // corner_l_u
  for (i = 0; i < 4; i++)
  {
    if (point_sum > cvRound(rect_pants_vtx[i].x) + cvRound(rect_pants_vtx[i].y))
    {
      pt0.x = cvRound(rect_pants_vtx[i].x);
      pt0.y = cvRound(rect_pants_vtx[i].y);

      point_sum = cvRound(rect_pants_vtx[i].x) + cvRound(rect_pants_vtx[i].y);
      corner_l_u = i;
    }
  }

  // find most right and upper corner
  // corner_r_u
  // this is prev or next corner of most left and upper

  if (cvRound(rect_pants_vtx[corner_l_u + 1].x) >
    cvRound(rect_pants_vtx[corner_l_u - 1].x))
  {
    pt0.x = cvRound(rect_pants_vtx[corner_l_u + 1].x);
    pt0.y = cvRound(rect_pants_vtx[corner_l_u + 1].y);
    corner_r_u = corner_l_u + 1;
  }
  else
  {
    pt0.x = cvRound(rect_pants_vtx[corner_l_u - 1].x);
    pt0.y = cvRound(rect_pants_vtx[corner_l_u - 1].y);
    corner_r_u = corner_l_u - 1;
  }
  /// ------------- end find left upper corner of min area rect-----------------

  /// ------------- upper side of min area rect ---------------------------------
  // calculate equations
  // line from left/right upper corner of min area rect
  // y = KU*x + BU
  //
  //       y1 - y2
  // KU = ------------
  //       x1 - x2
  //
  // BU = y1 - K*x1
  // 1 - l_u
  // 2 - r_u

  float KU, BU;
  KU = (rect_pants_vtx[corner_l_u].y - rect_pants_vtx[corner_r_u].y) /
    (rect_pants_vtx[corner_l_u].x - rect_pants_vtx[corner_r_u].x);
  BU = rect_pants_vtx[corner_l_u].y - KU * rect_pants_vtx[corner_l_u].x;

  // x of left test point set to 0
  // y - calculate
  CvPoint Left_test = (0, 0);
  // x of rigth point set image width
  CvPoint Right_test = (0, 0);

  Left_test.y = cvRound(BU);
  Right_test.x = (image->width);
  Right_test.y = cvRound(KU*(image->width) + BU);

  //draw left to right lines
  cvLine(image,
    Left_test,
    Right_test,
    BLUE,
    5, CV_AA, 0);
  /// ------------- end upper side of min area rect -----------------------------

  /// ----------------- mark pants appr contour points in ROI--------------------
  // ROI - left upper square of min area rect
  // we will find right upper side of contour

  // border of ROI
  // X must be less then x_lim = l_u.x + rect_width*0.2
  // Y must be less then y_lim = l_u.y + rect_width*0.2

  int x_lim, y_lim, rect_width;

  rect_width = cvRound(rect_pants_vtx[corner_r_u].x - rect_pants_vtx[corner_l_u].x);
  x_lim = cvRound(rect_pants_vtx[corner_l_u].x + cvRound(rect_width * 0.2));
  y_lim = cvRound(rect_pants_vtx[corner_l_u].y + cvRound(rect_width * 0.2));

  debug_printf(DEBUG, "x_lim = ");
  debug_printf(DEBUG, "%d\n", x_lim);
  debug_printf(DEBUG, "y_lim = ");
  debug_printf(DEBUG, "%d\n", y_lim);

  // set tpmPoint to (0, 0)
  CvPoint pt10 = (0, 0);

  // temp contour
  CvMemStorage* storage12 = cvCreateMemStorage(0);
  CvSeq* result_copy = cvCreateSeq(CV_SEQ_KIND_GENERIC | CV_32SC2, sizeof(CvContour),
    sizeof(CvPoint), storage12);
  // number of  points in ROI
  int number_ROI_points = 0;

  // select points in ROI and set other points in lowest right corner of image
  // create additional structure from contour with this two sorts of point
  for (int i = 0; i < result->total; i++)
  {
    CvPoint* p = CV_GET_SEQ_ELEM(CvPoint, result, i);
    pt10.x = p->x;
    pt10.y = p->y;

    if ((pt10.x > x_lim) || (pt10.y > y_lim))
    {
      pt10.x = image->width;
      pt10.y = image->height;
    }
    else
    {
      debug_printf(DEBUG, "i = ");
      debug_printf(DEBUG, "%d", i);
      debug_printf(DEBUG, "; pt10.x = ");
      debug_printf(DEBUG, "%d", pt10.x);
      debug_printf(DEBUG, "; pt10.y = ");
      debug_printf(DEBUG, "%d\n", pt10.y);
      number_ROI_points++;
      cvSeqPush(result_copy, &pt10);
    }

    // draw right point circle
    cvCircle(image,
      pt10,
      20,
      BLUE,
      30, 8, 0);

  }

  debug_printf(DEBUG, "result_copy size=%d\n", result_copy->total);

  /// ----------------- end mark pants appr contour points in ROI ----------------

  /// ----------------- mark left upper angle of pants ---------------------------
  // left upper angle of pants
  CvPoint pants_l_u = (0, 0);
  pants_l_u.x = image->width;
  pants_l_u.y = image->height;
  int point_index = 0;
  int exit_flag = 0;

  if (number_ROI_points == 1)
  {
    CvPoint* p = CV_GET_SEQ_ELEM(CvPoint, result_copy, 1);
    pants_l_u.x = p->x;
    pants_l_u.y = p->y;
  }
  else
  {
    while ((number_ROI_points != 0) && (exit_flag != 1))
    {
      // hightest point
      for (int i = 0; i < result_copy->total; i++)
      {
        CvPoint* p = CV_GET_SEQ_ELEM(CvPoint, result_copy, i);
        if (p->y < pants_l_u.y)
        {
          pants_l_u.x = p->x;
          pants_l_u.y = p->y;
          point_index = i;
        }
      }

      // is hightest point most left point?
      // not_left - flag
      // if flag not change - our highest point is most left point
      int not_left = 0;
      int j = 0;
      while ((j < result_copy->total) || (not_left != 0))
      {
        CvPoint* p = CV_GET_SEQ_ELEM(CvPoint, result_copy, j);
        // point have X more or equal to highest point's X
        debug_printf(DEBUG, "%d %d %d\n", j, point_index, result_copy->total);



        if (p->x <= pants_l_u.x)
        {
          if (point_index != j)
            // this is not my highest point
            // change value of highest point to left lowest corner of image
          {
            //            result_copy[j].x = 0;
            //            result_copy[j].y = image->height;
            point_index = 0;
            number_ROI_points--;
            not_left = 1;
          }

        }
        j++;
      }

      // find right and upper corner of pants
      // exit from loop
      if (not_left == 0)
      {
        exit_flag = 1;
      }

    }
  }

  // draw right point circle
  cvCircle(image,
    pants_l_u,
    50,
    RED,
    30, 8, 0);

  /// ----------------- end mark rght upper angle of pants -----------------------


  /// ----------------- define G size (lowest point of pant) ---------------------

  // set side lower points
  // left point set to rigth upper point
  CvPoint SideLeft = (image->width, image->height);
  // rigth point set to left lower point
  CvPoint SideRight = (0, 0);
  // set tpmPoint to (0, 0)
  CvPoint* pointTmp = (0, 0);
  // poiner for left/right point in contour
  int pointIndex = 0;

  // pant appr rect, not min square rect
  CvRect PantRect;

  PantRect = cvBoundingRect(result, 0);

  // find most left point below 75% of high of appr rect
  // result - contour which has been get with approximation
  //          longest contour of image

  for (int i = 0; i < result->total; i++)
  {
    pointTmp = CV_GET_SEQ_ELEM(CvPoint, result, i);
    if (pointTmp->y >(PantRect.y + cvRound(PantRect.height*0.8)))
    {
      if (pointTmp->x < SideLeft.x)
      {
        SideLeft.x = pointTmp->x;
        SideLeft.y = pointTmp->y;
        pointIndex = i;
      }
    }
  }
  // draw left point circle
  cvCircle(image,
    SideLeft,
    50,
    BLUE,
    15, 8, 0);

  // find most right point below 75%
  for (int i = 0; i < result->total; i++)
  {
    pointTmp = CV_GET_SEQ_ELEM(CvPoint, result, i);
    if (pointTmp->y >(PantRect.y + cvRound(PantRect.height*0.8)))
    {
      if (pointTmp->x > SideRight.x)
      {
        SideRight.x = pointTmp->x;
        SideRight.y = pointTmp->y;
        pointIndex = i;
      }
    }
  }
  // draw left point circle
  cvCircle(image,
    SideRight,
    50,
    BLUE,
    5, 8, 0);

  // calculate line equation
  // line from left/right point
  // y = KG*x + BG
  //
  //       y1 - y2
  // KG = ------------
  //       x1 - x2
  //
  // BG = y1 - KG*x1
  // 1 - sideLeft
  // 2 - sideRight

  float KG, BG;
  KG = (SideLeft.y - SideRight.y) / (SideLeft.x - SideRight.x);
  BG = SideLeft.y - KG * SideLeft.x;

  //draw left and right lines
  cvLine(image,
    SideRight,
    SideLeft,
    BLUE,
    40, CV_AA, 0);

  // calculate G
  double G = 0;
  G = sqrt(pow((SideLeft.x - SideRight.x), 2) +
    pow((SideLeft.y - SideRight.y), 2)) / unit;

	sprintf(tmpstr, "G=%f", G);
	printOnIm(image, tmpstr, new CvPoint(SideRight.x+60, SideRight.y), 4);
  debug_printf(DEBUG, "G = ");
  debug_printf(DEBUG, "%f", G);
  debug_printf(DEBUG, " inch\n");
  /// ------------- end define G size (lowest point of pant) ---------------------

  /// ---------------------- find crotch point -----------------------------------
  // find most right point of pant
  // result - contour which has been get with approximation
  //          longest contour of image
  CvPoint CrotchPoint = (0, 0);

  for (int i = 0; i < result->total; i++)
  {
    pointTmp = CV_GET_SEQ_ELEM(CvPoint, result, i);
    if (pointTmp->x > CrotchPoint.x)
    {
      CrotchPoint.x = pointTmp->x;
      CrotchPoint.y = pointTmp->y;
      pointIndex = i;
    }
  }

  // draw left point circle
  cvCircle(image,
    CrotchPoint,
    50,
    BLUE,
    15, 8, 0);
  /// ---------------------- end find crotch point -------------------------------


  /// --------------------------- Find E -----------------------------------------
  CvPoint ERight = contourIterate(seqT, CrotchPoint, 1.0*unit, 1);
  CvPoint ELeft = (0, 0);

  // calculate ERight point
  // use line parallel to G line
  // y = KG*x + BE
  float BE = ERight.y - KU * ERight.x;
  ELeft.y = cvRound(KU*ELeft.x + BE);

	std::vector<CvPoint>* points =getIntersections(image, ELeft, ERight, seqT);
	ELeft = points->front();

  for (std::vector<CvPoint>::iterator point =  points-> begin(); point != points->end(); point++){
	  //printf("%d ______\n", ELeft.x);
	  if (point->x<ELeft.x){
		  ELeft = *point;
	  }
  }
	debug_printf(DEBUG, "%d %d %d %d\n",ELeft.x, ELeft.y, ERight.x, ERight.y );
	//delete(points);
  cvCircle(image,
      ELeft,
      50,
      WHITE,
      15, 8, 0);

  //draw left and right lines
  cvLine(image,
      ERight,
      ELeft,
      BLUE,
      10, CV_AA, 0);

  float E = sqrt(pow((ELeft.x - ERight.x), 2) +
    pow((ELeft.y - ERight.y), 2)) / unit;

	sprintf(tmpstr, "E=%f", E);
	printOnIm(image, tmpstr, new CvPoint(ERight.x+60, ERight.y), 4);

  debug_printf(DEBUG, "E = ");
  debug_printf(DEBUG, "%f", E);
  debug_printf(DEBUG, " inch\n");

  /// --------------------------- End find E -------------------------------------

  /// --------------------------- Find F -----------------------------------------
  CvPoint FLeft = contourIterate(seqT, pants_l_u, 23.0*unit, -1);
  CvPoint FRight = (0, 0);

  // calculate FRight point
  // use line parallel to G line
  // y = KG*x + BE
  float BF = FLeft.y - KU * FLeft.x;
  FRight.x = image->width;
  FRight.y = cvRound(KU*FRight.x + BF);

	points = getIntersections(image, FLeft, FRight, seqT);
	FRight = points->front();

  for (std::vector<CvPoint>::iterator point =  points-> begin(); point != points->end(); point++){
	  if (point->x>FRight.x){
		  FRight = *point;
	  }
  }
	debug_printf(DEBUG, "%d %d %d %d\n",FLeft.x, FLeft.y, FRight.x, FRight.y );
  delete(points);
  cvCircle(image,
    FRight,
    50,
    WHITE,
    15, 8, 0);

  //draw left and right lines
  cvLine(image,
    FRight,
    FLeft,
    BLUE,
    10, CV_AA, 0);

  float F = sqrt(pow((FLeft.x - FRight.x), 2) +
    pow((FLeft.y - FRight.y), 2)) / unit;

	sprintf(tmpstr, "F=%f", F);
	printOnIm(image, tmpstr, new CvPoint(FRight.x+60, FRight.y), 4);


  debug_printf(DEBUG, "F = ");
  debug_printf(DEBUG, "%f", F);
  debug_printf(DEBUG, " inch\n");

  /// --------------------------- End find F -------------------------------------


  /// --------------------------- Find D -----------------------------------------
  CvPoint DLeft = contourIterate(seqT, pants_l_u, 8.5*unit, -1);
  CvPoint DRight = (0, 0);

  // calculate DRight point
  // use line parallel to G line
  // y = KG*x + BE
  float BD = DLeft.y - KU * DLeft.x;
  DRight.x = image->width;
  DRight.y = cvRound(KU*DRight.x + BD);

  points =getIntersections(image, DLeft, DRight, seqT);
	DRight = points->front();

  for (std::vector<CvPoint>::iterator point =  points-> begin(); point != points->end(); point++){
	  if (point->x>DRight.x){
		  DRight = *point;
	  }
  }

	delete(points);

  cvCircle(image,
    DRight,
    50,
    WHITE,
    15, 8, 0);

  //draw left and right lines
  cvLine(image,
    DRight,
    DLeft,
    BLUE,
    10, CV_AA, 0);

  float D = sqrt(pow((DLeft.x - DRight.x), 2) +
    pow((DLeft.y - DRight.y), 2)) / unit;

	sprintf(tmpstr, "D=%f", D);
	printOnIm(image, tmpstr, new CvPoint(DRight.x+60, DRight.y), 4);

  debug_printf(DEBUG, "D = ");
  debug_printf(DEBUG, "%f", D);
  debug_printf(DEBUG, " inch\n");

  /// --------------------------- End find D -------------------------------------

  /// ------------------------------  find B2 ------------------------------------
  float B2 = contourSegmentLenght(seqT, CrotchPoint, SideRight, 1) / unit;
	sprintf(tmpstr, "B2=%f", B2);
	printOnIm(image, tmpstr, new CvPoint(ERight.x+60, ERight.y-100), 4);
  debug_printf(DEBUG, "B2 = ");
  debug_printf(DEBUG, "%f", B2);
  debug_printf(DEBUG, " inch\n");
  /// --------------------------- End find B2 ------------------------------------

  /// --------------------------- find Outseam -----------------------------------
  float Outseam = contourSegmentLenght(seqT, pants_l_u, SideLeft, -1) / unit;
  debug_printf(DEBUG, "Outseam = ");
  debug_printf(DEBUG, "%f", Outseam);
  debug_printf(DEBUG, " inch\n");
  /// --------------------------- End find Outseam -------------------------------

  /// -------------------------------- Display windows ---------------------------
  // resize picture

  cvSaveImage(replace_str(filename, ".jpg", "-out.jpg"), image, 0);

  // // Create a window to display results
  // cvNamedWindow("original", CV_WINDOW_NORMAL);
  // cvNamedWindow("gray", CV_WINDOW_NORMAL);

  // // Show images
  // cvShowImage("original", image);
  // cvShowImage("gray", gray_picture);

  // --------------------------------- Clear memory ------------------------------
  // От кого я особенно ненавижу - так ето вас всех!
  // Kill them all!

  // Wait until user finishes program
  // cvWaitKey(0);

  // release memory
  cvReleaseImage(&image);
  cvReleaseImage(&gray_picture);

  printf("%s, %f, %f, %f, %f, %f, %f\n", filename, G*2.54, E*2.54, F*2.54, D*4*2.54, B2*2.54, Outseam*2.54);
  // Delete windows
  // cvDestroyAllWindows();
  return 0;
}
