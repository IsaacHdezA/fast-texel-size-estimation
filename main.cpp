#include <iostream>
#include <cmath>
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
                 IMG_FILENAME  = "test3",
                 IMG_EXTENSION = ".jpg",
                 IMG_FULLPATH  = IMG_PATH + IMG_FILENAME + IMG_EXTENSION;

    Mat img = imread(IMG_FULLPATH, IMREAD_GRAYSCALE);
    // resize(img, img, Size(), 0.45, 0.45);
    imshow(IMG_FILENAME, img);

    // Mat img = (Mat_<uchar>(5, 5) <<
    //           0, 255,   0,   0,   0,
    //         255, 255, 255,   0, 255,
    //         255, 255,   0, 255, 255,
    //         255,   0, 255, 255, 255,
    //           0, 255, 255,   0, 255
    // );

    ofstream horHomogeneityFile("horHomogeneity.csv", ios::out);
    Mat normDifHist;
    for(int d = 1; d < 256; d++) {
        cout << "difHist with " << d << ": ";
        normDifHist = normHist(computeDifHist(img, d));
        horHomogeneityFile << computeHomogeneity(normDifHist) << ',';
    }
    horHomogeneityFile << endl;
    horHomogeneityFile.close();

    ofstream verHomogeneityFile("verHomogeneity.csv", ios::out);
    for(int d = 1; d < 256; d++) {
        Mat vertDifHist = Mat::zeros(1, 511, CV_16UC1);
        for(int c = 0; c < img.cols; c++) {
            for(int r = 0; r < (img.rows - d); r++)
                ++vertDifHist.at<ushort>(0, (img.at<uchar>(r, c) - img.at<uchar>(r + d, c)) + 255);
        }
        Mat vertNormDifHist = normHist(vertDifHist);
        verHomogeneityFile << computeHomogeneity(vertNormDifHist) << ',';
    }
    
    // writeCSV("sumHist.csv", normSumHist);
    // writeCSV("difHist.csv", normDifHist);

    waitKey();
    destroyAllWindows();

    return 0;
}

Mat computeSumHist(const Mat &src, const int D, const int THETA) {
    Mat out;

    if(src.empty() || src.channels() > 1 || !src.data) {
        cout << "computeSumHist(): Image is empty or should be grayscale" << endl;
        return out;
    }

    out = Mat::zeros(1, 511, CV_16UC1);
    for(int i = 0; i < src.rows; i++) {
        uchar *row = (uchar *) src.ptr<uchar>(i);
        for(int j = 0; j < (src.cols - D); j++)
            ++out.at<ushort>(0, (row[j] + row[j + D]));
    }

    return out;
}

Mat computeDifHist(const Mat &src, const int D, const int THETA) {
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

Mat normHist(const Mat &hist) {
    Mat out;

    if(hist.empty() || hist.channels() > 1 || !hist.data || hist.rows > 1) {
        cout << "normHist: Histogram should be one row only" << endl;
        return out;
    }

    out = hist.clone();
    out.convertTo(out, CV_32FC1);

    int totSum = sum(out)[0];
    cout << totSum << endl;

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
