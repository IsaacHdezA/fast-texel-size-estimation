#include <iostream>

using namespace std;

#ifdef _WIN32
  #include <Windows.h>

  inline int  getScreenWidth() { return (int) ::GetSystemMetrics(SM_CXSCREEN); }
  inline int getScreenHeight() { return (int) ::GetSystemMetrics(SM_CYSCREEN); }

  bool dirExists(const string &path) {
    DWORD attrib = GetFileAttributes(path.c_str());
    return (attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY));
  }
#endif

#include <cstdlib>
#include <fstream>

#define _USE_MATH_DEFINES
#include <cmath>

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace cv;

void writeCSV(string filename, Mat m) {
  ofstream myfile;
  myfile.open(filename.c_str());
  myfile<< cv::format(m, cv::Formatter::FMT_CSV) << std::endl;
  myfile.close();
}

Mat computeHorDifHist(const Mat &, const int = 1, const int = 0);
Mat computeVerDifHist(const Mat &, const int = 1, const int = 0);

Mat normHist(const Mat &);
float computeHomogeneity(const Mat &);
void findMax(const float *, int, float &, int &, int = 10);

Mat findTexel(const Mat &, const string &, int = 10, int = 10);
Mat recreatePattern(const int, const int, const Mat &);

int main() {
  const string IMG_PATH      = "./res/",
               IMG_FILENAME  = "test3",
               IMG_EXTENSION = ".jpg",
               IMG_FULLPATH  = IMG_PATH + IMG_FILENAME + IMG_EXTENSION;

  Mat img = imread(IMG_FULLPATH),
      res;

  // Resize image if it's bigger than half of any dimension and create output directories
  #ifdef _WIN32
    const int SCRN_WIDTH = getScreenWidth(), SCRN_HEIGHT = getScreenHeight();

    if(img.cols > (SCRN_WIDTH / 2) || img.rows > (SCRN_HEIGHT / 2))
      resize(img, img, Size((SCRN_WIDTH / 2.8), (SCRN_HEIGHT / 2.8)));
    
    if(!dirExists("./out/"))      CreateDirectory("./out/",      nullptr);
    if(!dirExists("./out/img/"))  CreateDirectory("./out/img/",  nullptr);
    if(!dirExists("./out/csv/"))  CreateDirectory("./out/csv/",  nullptr);
    if(!dirExists("./out/hist/")) CreateDirectory("./out/hist/", nullptr);
  #endif

  res = img.clone();
  cvtColor(img, img, COLOR_BGR2GRAY);

  imshow(IMG_FILENAME, img);
  moveWindow(IMG_FILENAME, 0, 0);

  // Finding texel size
  int verThres = 10,
      horThres = 10;
  Mat texel = findTexel(img, IMG_FILENAME, verThres, horThres);
  int texWidth = texel.cols, texHeight = texel.rows;

  // Drawing texel size on image
  for(int i = 0; i < 2; i++) {
    for(int j = 0; j < 2; j++) {
      rectangle(
        res,
        Point(j * texWidth, i * texHeight),
        Point((j + 1) * texWidth, (i + 1) * texHeight),
        Scalar(0, 0, 255),
        2,
        LINE_8
      );
    }
  }

  imshow(IMG_FILENAME + " with texel", res);
  imwrite("./out/img/" + IMG_FILENAME + "_texel.jpg", res);
  moveWindow(IMG_FILENAME + " with texel", 0, img.rows);

  // Recreate image using texel
  Mat recreation = recreatePattern(img.cols, img.rows, texel);
  resize(recreation, recreation, Size(), 1.3, 1.3);
  imshow("Texel image recreation", recreation);
  imwrite("./out/img/" + IMG_FILENAME + "_texel-rec.jpg", recreation);

  // Creating plots using external script
  try {
    system(("python plot.py " + IMG_FILENAME + " --save").c_str());

    // Opening plots created with Python
    Mat plot = imread("./out/hist/" + IMG_FILENAME + "_homogeneity.png");
    resize(plot, plot, Size(), 0.4, 0.4);
    imshow(IMG_FILENAME + " homogeneity", plot);
    moveWindow(IMG_FILENAME + " homogeneity", img.cols, 0);
    moveWindow("Texel image recreation", img.cols, plot.rows);
  } catch(Exception ERR) {
    cout << "\n[ERROR] Could not create nor open plot. Try installing Python kernel and the matplotlib module or manually execute the plot.py script" << endl;
    moveWindow("Texel image recreation", img.cols, 0);
  }

  waitKey();
  destroyAllWindows();

  return 0;
}

Mat computeHorDifHist(const Mat &src, const int D, const int THETA) {
  Mat out;

  if(src.empty() || src.channels() > 1 || !src.data) {
    cout << "computeSumHist(): Image is empty or should be grayscale" << endl;
    return out;
  }

  out = Mat::zeros(1, 511, CV_16UC1);
  for(int r = 0; r < src.rows; r++) {
    uchar *row = (uchar *) src.ptr<uchar>(r);
    for(int c = 0; c < (src.cols - D); c++)
      ++out.at<ushort>(0, (row[c] - row[c + D]) + 255);
  }

  return out;
}

Mat computeVerDifHist(const Mat &src, const int D, const int THETA) {
  Mat out;

  if(src.empty() || src.channels() > 1 || !src.data) {
    cout << "computeSumHist(): Image is empty or should be grayscale" << endl;
    return out;
  }

  out = Mat::zeros(1, 511, CV_16UC1);
  for(int c = 0; c < src.cols; c++) {
    for(int r = 0; r < (src.rows - D); r++)
      ++out.at<ushort>(0, (src.at<uchar>(r, c) - src.at<uchar>(r + D, c)) + 255);
  }

  return out;
}

Mat normHist(const Mat &hist) {
  Mat out;

  if(hist.empty() || hist.channels() > 1 || !hist.data || hist.rows > 1) {
    cout << "normHist: Histogram should be one row only" << endl;
    return out;
  }

  out = hist.clone();
  out.convertTo(out, CV_32FC1);

  int totSum = sum(out)[0];
  out = out / totSum;

  return out;
}

float computeHomogeneity(const Mat &difHist) {
  if(difHist.empty() || difHist.channels() > 1 || !difHist.data || difHist.rows > 1) {
    cout << "normHist: Histogram should be one row only" << endl;
    return -1;
  }

  float homogeneity = 0;
  for(int i = 0; i < difHist.cols; i++)
    // homogeneity += (1.0/(1 + ((i - 255) * (i - 255)))) * difHist.at<float>(0, i);
    homogeneity += (1.0/(1 + ((i - 255) * (i - 255)))) * difHist.at<float>(0, i);

  return homogeneity;
}

void findMax(const float *vec, int size, float &val, int &id, int thres) {
  val = vec[0];

  for(int i = thres; i < size; i++)
    if(vec[i] > val) {
      val = vec[i];
      id = i;
    }
}

Mat findTexel(const Mat &src, const string &IMG_FILENAME, int verThres, int horThres) {
  ofstream horHomogeneityFile("./out/csv/" + IMG_FILENAME + "_horHomogeneity.csv", ios::out),
           verHomogeneityFile("./out/csv/" + IMG_FILENAME + "_verHomogeneity.csv", ios::out);

  Mat horNormDifHist,
      verNormDifHist;

  const int M = src.rows, N = src.cols,
            VER_SPACE = M / 2, HOR_SPACE = N / 2;

  // Compute horizontal displacement
  float horHomogeneity = 0,
        horHomogeneityVec[HOR_SPACE] = { 0 };
  for(int d = 2; d < HOR_SPACE; d++) {
      horNormDifHist = normHist(computeHorDifHist(src, d, 0));
      horHomogeneity = computeHomogeneity(horNormDifHist);
      horHomogeneityFile << horHomogeneity << ((d + 1) < HOR_SPACE ? "," : "");

      horHomogeneityVec[d] = horHomogeneity;
  }

  // Compute vertical displacement
  float verHomogeneity = 0,
        verHomogeneityVec[VER_SPACE] = { 0 };
  for(int d = 2; d < VER_SPACE; d++) {
      verNormDifHist = normHist(computeVerDifHist(src, d, 0));
      verHomogeneity = computeHomogeneity(verNormDifHist);
      verHomogeneityFile << verHomogeneity << ((d + 1) < VER_SPACE ? "," : "");

      verHomogeneityVec[d] = verHomogeneity;
  }

  horHomogeneityFile.close();
  verHomogeneityFile.close();

  // Finding texel size
  int texHeight = 0, texWidth = 0;
  float maxVer = 0,   maxHor = 0;
  findMax(verHomogeneityVec, VER_SPACE, maxVer, texHeight, verThres);
  findMax(horHomogeneityVec, HOR_SPACE, maxHor, texWidth, horThres);

  Mat texel = Mat::zeros(texHeight, texWidth, CV_8UC3);
  texel = src(Range(0, texHeight), Range(0, texWidth));

  return texel;
}

Mat recreatePattern(const int imgWidth, const int imgHeight, const Mat &texel) {
  int texWidth = texel.cols, texHeight = texel.rows;
  float texX = (imgWidth / texWidth), texY = (imgHeight / texHeight);

  Mat recreation = Mat::zeros(texel.rows * texY, texel.cols * texX, CV_8UC1);

  for(int c = 0; c < texX; c++) {
    for(int r = 0; r < texY; r++) {
      int x  = c * texWidth,
          y  = r * texHeight;
      texel.copyTo(recreation(Rect(x, y, texWidth, texHeight)));
    }
  }

  return recreation;
}