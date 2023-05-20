#include <cstdlib>
#include <iostream>
#include <fstream>

#define _USE_MATH_DEFINES
#include <cmath>

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

inline float toRad(float deg) { return (deg * M_PI)/180; }

Mat computeHorDifHist(const Mat &, const int = 1, const int = 0);
Mat computeVerDifHist(const Mat &, const int = 1, const int = 0);

Mat normHist(const Mat &);
float computeHomogeneity(const Mat &);

int main() {
    const string IMG_PATH      = "./res/",
                 IMG_FILENAME  = "test3",
                 IMG_EXTENSION = ".jpg",
                 IMG_FULLPATH  = IMG_PATH + IMG_FILENAME + IMG_EXTENSION;

    Mat img = imread(IMG_FULLPATH, IMREAD_GRAYSCALE);
    // resize(img, img, Size(), 0.45, 0.45);

    // Mat img = (Mat_<uchar>(5, 5) <<
    //           0, 255,   0,   0,   0,
    //         255, 255, 255,   0, 255,
    //         255, 255,   0, 255, 255,
    //         255,   0, 255, 255, 255,
    //           0, 255, 255,   0, 255
    // );

    ofstream horHomogeneityFile("./out/" + IMG_FILENAME + "_horHomogeneity.csv", ios::out),
             verHomogeneityFile("./out/" + IMG_FILENAME + "_verHomogeneity.csv", ios::out);

    Mat horNormDifHist,
        verNormDifHist;

    for(int d = 1; d < img.cols; d++) {
        horNormDifHist = normHist(computeHorDifHist(img, d, 0));
        horHomogeneityFile << computeHomogeneity(horNormDifHist) << ((d + 1) < img.cols ? "," : "");
    }

    for(int d = 1; d < img.cols; d++) {
        verNormDifHist = normHist(computeVerDifHist(img, d, 0));
        verHomogeneityFile << computeHomogeneity(verNormDifHist) << ((d + 1) < img.cols ? "," : "");
    }

    horHomogeneityFile.close();
    verHomogeneityFile.close();

    // Creating plots
    system(("python plot.py " + IMG_FILENAME + " --save").c_str());
    // system(("python plot.py " + IMG_FILENAME + " --show").c_str());
    // system(("python plot.py " + IMG_FILENAME + " --save-and-show").c_str());

    imshow(IMG_FILENAME, img);
    moveWindow(IMG_FILENAME, 0, 0);

    Mat plot = imread("./out/img/" + IMG_FILENAME + "_homogeneity.png");
    resize(plot, plot, Size(), 0.5, 0.5);
    imshow(IMG_FILENAME + " homogeneity", plot);
    moveWindow(IMG_FILENAME + " homogeneity", img.cols, 0);

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

Mat computeHorDifHist(const Mat &src, const int D, const int THETA) {
    Mat out;
    const float ANGLE = toRad(THETA + 90);
    // cout << THETA << "deg to rad: " << ANGLE << "rad" << '\n'
    //      << "\tx = " << cos(ANGLE) << "\n"
    //      << "\ty = " << sin(ANGLE) << "\n";

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
