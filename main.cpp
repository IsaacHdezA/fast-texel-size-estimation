#include <iostream>

using namespace std;

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace cv;

int main() {
    const string IMG_PATH      = "./res/",
                 IMG_FILENAME  = "Page2",
                 IMG_EXTENSION = ".jpg",
                 IMG_FULLPATH  = IMG_PATH + IMG_FILENAME + IMG_EXTENSION;

    Mat img = imread(IMG_FULLPATH);
    resize(img, img, Size(), 0.45, 0.45);
    imshow(IMG_FILENAME, img);
    
    waitKey();
    destroyAllWindows();

    return 0;
}