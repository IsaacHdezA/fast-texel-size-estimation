#include <iostream>
#include <fstream>

using namespace std;

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

Mat computeGLCM(const Mat &, int = 1, float = 0);
Mat computeSumHist(const Mat &, const int = 1, const int = 0);
Mat computeDifHist(const Mat &, const int = 1, const int = 0);

Mat normHist(const Mat &);
float computeHomogeneity(const Mat &);

int main() {
    const string IMG_PATH      = "./res/",
                 IMG_FILENAME  = "test",
                 IMG_EXTENSION = ".jpg",
                 IMG_FULLPATH  = IMG_PATH + IMG_FILENAME + IMG_EXTENSION;

    // Mat img = imread(IMG_FULLPATH, IMREAD_GRAYSCALE);
    // resize(img, img, Size(), 0.45, 0.45);
    // imshow(IMG_FILENAME, img);

    Mat img = (Mat_<uchar>(5, 5) <<
              0, 255,   0,   0,   0,
            255, 255, 255,   0, 255,
            255, 255,   0, 255, 255,
            255,   0, 255, 255, 255,
              0, 255, 255,   0, 255
    );

    Mat sumHist = computeSumHist(img),
        difHist = computeDifHist(img);

    Mat normSumHist = normHist(sumHist),
        normDifHist = normHist(difHist);
    
    writeCSV("sumHist.csv", normSumHist);
    writeCSV("difHist.csv", normDifHist);

    waitKey();
    destroyAllWindows();

    return 0;
}

Mat glcm(const Mat &src, int d, float theta) {

}

Mat computeSumHist(const Mat &src, const int D, const int THETA) {
    Mat out;

    if(src.empty() || src.channels() > 1 || !src.data) {
        cout << "computeSumHist(): Image is empty or should be grayscale" << endl;
        return out;
    }

    out = Mat::zeros(1, 512, CV_8UC1);
    for(int i = 0; i < src.rows; i++) {
        uchar *row = (uchar *) src.ptr<uchar>(i);
        for(int j = 0; j < (src.cols - D); j++)
            ++out.at<uchar>(0, (row[j] + row[j + D]));
    }

    return out;
}

Mat computeDifHist(const Mat &src, const int D, const int THETA) {
    Mat out;

    if(src.empty() || src.channels() > 1 || !src.data) {
        cout << "computeSumHist(): Image is empty or should be grayscale" << endl;
        return out;
    }

    out = Mat::zeros(1, 512, CV_8UC1);
    for(int i = 0; i < src.rows; i++) {
        uchar *row = (uchar *) src.ptr<uchar>(i);
        for(int j = 0; j < (src.cols - D); j++)
            ++out.at<uchar>(0, (row[j] - row[j + D]) + 255);
    }

    return out;
}

Mat normHist(const Mat &src) {
    Mat out;

    if(src.empty() || src.channels() > 1 || !src.data || src.rows > 1) {
        cout << "normHist: Histogram should be one row only" << endl;
        return out;
    }

    out = src.clone();
    out.convertTo(out, CV_32FC1);

    int totSum = sum(out)[0];

    out = out / totSum;

    return out;
}