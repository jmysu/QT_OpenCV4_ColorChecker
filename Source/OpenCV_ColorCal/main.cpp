#include <QCoreApplication>
#include <QFile>

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/mcc.hpp>

using namespace std;
using namespace cv;
using namespace mcc;
using namespace ccm;

// helper function:
// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2
static double angle( Point pt1, Point pt2, Point pt0 )
{
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

// returns sequence of squares detected on the image.
// the sequence is stored in the specified memory storage
static void findSquares( const Mat& image, vector<vector<Point> >& squares )
{
    squares.clear();

    Mat pyr, timg, gray0(image.size(), CV_8U), gray;

    // down-scale and upscale the image to filter out the noise
    pyrDown(image, pyr, Size(image.cols/2, image.rows/2));
    pyrUp(pyr, timg, image.size());
    vector<vector<Point> > contours;

    // find squares in every color plane of the image
    for( int c = 0; c < 3; c++ )
    {
        int ch[] = {c, 0};
        mixChannels(&timg, 1, &gray0, 1, ch, 1);

        // try several threshold levels
        int N = 5;
        for( int l = 0; l < N; l++ )
        {
            // hack: use Canny instead of zero threshold level.
            // Canny helps to catch squares with gradient shading
            if( l == 0 )
            {
                // apply Canny. Take the upper threshold from slider
                // and set the lower to 0 (which forces edges merging)
                Canny(gray0, gray, 0, 50, 5);
                // dilate canny output to remove potential
                // holes between edge segments
                dilate(gray, gray, Mat(), Point(-1,-1));
            }
            else
            {
                // apply threshold if l!=0:
                //     tgray(x,y) = gray(x,y) < (l+1)*255/N ? 255 : 0
                gray = gray0 >= (l+1)*255/N;
            }

            // find contours and store them all as a list
            findContours(gray, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

            vector<Point> approx;

            // test each contour
            for( size_t i = 0; i < contours.size(); i++ )
            {
                // approximate contour with accuracy proportional
                // to the contour perimeter
                approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);

                // square contours should have 4 vertices after approximation
                // relatively large area (to filter out noisy contours)
                // and be convex.
                // Note: absolute value of an area is used because
                // area may be positive or negative - in accordance with the
                // contour orientation
                if( approx.size() == 4 &&
                    fabs(contourArea(Mat(approx))) > 1000 &&
                    isContourConvex(Mat(approx)) )
                {
                    double maxCosine = 0;

                    for( int j = 2; j < 5; j++ )
                    {
                        // find the maximum cosine of the angle between joint edges
                        double cosine = fabs(angle(approx[j%4], approx[j-2], approx[j-1]));
                        maxCosine = MAX(maxCosine, cosine);
                    }

                    // detect ratio that corresponds to a rectangle or a square
                    cv::Rect r = cv::boundingRect(contours[i]);
                    double ratio = std::abs(1 - (double)r.width / r.height);
                    // if cosines of all angles are small
                    // (all angles are ~90 degree)
                    //  and the ratio does not corresponds to
                    // a rectangle then write quandrange
                    // vertices to resultant sequence
                    if( maxCosine < 0.3 && ratio <= 0.09)
                        squares.push_back(approx);
                }
            }
        }
    }
}

// the function draws all the squares in the image
static void drawSquares( Mat& image, const vector<vector<Point> >& squares )
{
    for( size_t i = 0; i < squares.size(); i++ )
    {
        const Point* p = &squares[i][0];
        int n = (int)squares[i].size();
        polylines(image, &p, &n, 1, true, Scalar(0,255,0), 1, LINE_AA);
    }

    imshow("squares", image);
}

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
    //Mat src = loadFromQrc(":/pic/C24.jpg");
    Mat srcImage = loadFromQrc(":/pic/CC24.png");

    if(srcImage.empty())    {
        std::cout << "Could not read the image: " << std::endl;
        return 1;
        }
    // check that it is loaded correctly
    if(!srcImage.data || srcImage.empty())
        cerr << "Problem loading image!!!" << endl;

    //Do Macbech colorchecker
    cv::Ptr<cv::mcc::CCheckerDetector> detector = cv::mcc::CCheckerDetector::create();
    // Marker type to detect
    if (!detector->process(srcImage, cv::mcc::TYPECHART(0), 1))
    {
        qDebug("ChartColor not detected \n");
    }
    else {
        Mat ccImage= srcImage.clone();
        // get checker
        std::vector<cv::Ptr<cv::mcc::CChecker>> checkers = detector->getListColorChecker();

        for (cv::Ptr<cv::mcc::CChecker> checker : checkers)
        {
            // current checker
            cv::Ptr<cv::mcc::CCheckerDraw> cdraw = cv::mcc::CCheckerDraw::create(checker);
            cdraw->draw(ccImage);
            imshow("ColorChecker", ccImage);
            //
            Mat chartsRGB = checker->getChartsRGB();
            Mat src = chartsRGB.col(1).clone().reshape(3, chartsRGB.rows/3);
            src /= 255.0;
            //compte color correction matrix
            //ColorCorrectionModel model1(src, COLORCHECKER_Vinyl);
            ColorCorrectionModel model1(src,COLORCHECKER_Macbeth);
            model1.run();
            Mat ccm = model1.getCCM();
            std::cout<<"ccm "<<ccm<<std::endl;
            double loss = model1.getLoss();
            std::cout<<"loss "<<loss<<std::endl;

            //Calibrate src image
            Mat img_;
            cvtColor(srcImage, img_, COLOR_BGR2RGB);
            img_.convertTo(img_, CV_64F);
            const int inp_size = 255;
            const int out_size = 255;
            img_ = img_ / inp_size;
            Mat calibratedImage= model1.infer(img_);
            Mat out_ = calibratedImage * out_size;
            //Convert colors
            out_.convertTo(out_, CV_8UC3);
            Mat img_out = min(max(out_, 0), out_size);
            Mat out_img;
            cvtColor(img_out, out_img, COLOR_RGB2BGR);
            imshow("Calibrated", out_img);
            cv::moveWindow("Calibrated",srcImage.rows, 0);
        }
    }


    waitKey();
    return 0;
}
