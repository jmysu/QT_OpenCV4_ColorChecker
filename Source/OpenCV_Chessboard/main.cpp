#include <QCoreApplication>
#include <QFile>

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/types_c.h>
//#include <opencv2/mcc.hpp>

using namespace std;
using namespace cv;
//using namespace mcc;
//using namespace ccm;

// Defining the dimensions of checkerboard
//int CHECKERBOARD[2]{9,6}; //Chess.png
//int CHECKERBOARD[2]{6,3}; //Chessboard.jpg
int CHECKERBOARD[2]{8,6}; //Chessboard8x6.jpg


Mat loadFromQrc(QString qrc, int flag = IMREAD_COLOR)
{
    //double tic = double(getTickCount());

    QFile file(qrc);
    Mat m;
    if(file.open(QIODevice::ReadOnly))
    {
        qint64 sz = file.size();
        std::vector<uchar> buf(sz);
        file.read((char*)buf.data(), sz);
        m = imdecode(buf, flag);
    }

    //double toc = (double(getTickCount()) - tic) * 1000.0 / getTickFrequency();
    //qDebug() << "OpenCV loading time: " << toc;

    return m;
}

int main()
{
    // load image
    //Mat srcImage = loadFromQrc(":/pic/Chess.png");
    //Mat srcImage = loadFromQrc(":/pic/Chessboard.jpg");
    Mat srcImage = loadFromQrc(":/pic/Chessboard8x6.png");
    //Mat srcImage = loadFromQrc(":/pic/Chessboard7x7.png");


    if(srcImage.empty())    {
        std::cout << "Could not read the image: " << std::endl;
        return 1;
        }

    // Creating vector to store vectors of 3D points for each checkerboard image
    std::vector<std::vector<cv::Point3f> > objpoints;
    // Creating vector to store vectors of 2D points for each checkerboard image
    std::vector<std::vector<cv::Point2f> > imgpoints;
    // Defining the world coordinates for 3D points
    std::vector<cv::Point3f> objp;
    for(int i{0}; i<CHECKERBOARD[1]; i++) {
      for(int j{0}; j<CHECKERBOARD[0]; j++)
        objp.push_back(cv::Point3f(j,i,0));
        }
    cv::Mat gray;
    cv::cvtColor(srcImage,gray,cv::COLOR_BGR2GRAY);
    // Finding checker board corners

    vector<Point2f> pointBuf;
    bool found;
    int chessBoardFlags = CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE;
    found = findChessboardCorners( gray, cv::Size(CHECKERBOARD[0], CHECKERBOARD[1]), pointBuf, chessBoardFlags);

    if(found){
        //cv::TermCriteria criteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30, 0.001);
        cv::TermCriteria criteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1.0);

        // Displaying the detected corner points on the checker board
        cv::drawChessboardCorners(srcImage, cv::Size(CHECKERBOARD[0], CHECKERBOARD[1]), Mat(pointBuf), found);

        //objpoints.push_back(objp);
        //imgpoints.push_back(corner_pts);
        cv::imshow("Found Chessboard",srcImage);
        }
    else
        cv::imshow("Found No Chessboard",srcImage);

    waitKey();
    return 0;
}
