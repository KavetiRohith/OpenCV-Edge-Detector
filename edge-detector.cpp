#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>

using namespace cv;
using namespace std;

struct timespec curr_t;
double curr_time, start_time, frame_rate;
unsigned long long frame_count = 0;

// buffer to hold the frame rate text
char frameratetext[100];

Mat frame, frame_blurred, frame_gray;
Mat grad;
Mat dst, detected_edges;
int lowThreshold = 0;
const int max_lowThreshold = 100;
const int ratio = 3;
const int kernel_size = 3;

// Function to apply Canny edge detection
static void CannyThreshold(int, void *)
{
    // Apply Gaussian blur to reduce noise
    blur(frame_gray, detected_edges, Size(3, 3));
    // Apply Canny edge detection
    Canny(detected_edges, detected_edges, lowThreshold, double(lowThreshold * ::ratio), kernel_size);
    // Create a black image
    dst = Scalar::all(0);
    // Copy original image to the output image using the detected edges as a mask
    frame.copyTo(dst, detected_edges);

    // Calculate frame rate
    frame_count++;
    clock_gettime(CLOCK_MONOTONIC, &curr_t);
    curr_time = (double)curr_t.tv_sec + ((double)curr_t.tv_nsec) / 1000000000.0;
    frame_rate = (double)frame_count / (curr_time - start_time);

    // Convert frame rate to string
    sprintf(frameratetext, "fr: %.3f fc: %lld ctime: %.3f stime: %.3f", frame_rate, frame_count, curr_time, start_time);

    // Display frame rate on the output image
    cv::putText(dst, frameratetext, Point(30, 30), FONT_HERSHEY_COMPLEX_SMALL,
                0.8, Scalar(200, 200, 250), 1, LINE_AA);

    // Show the output image
    imshow("Canny Edge Map", dst);
}

int ksize_slider;
int scale;
int delta;
int ddepth = CV_16S;

// Function to apply Sobel edge detection
static void sobelDemo(int, void *)
{
    Mat grad_x, grad_y;
    Mat abs_grad_x, abs_grad_y;

    // Calculate the kernel size based on the slider value, the slider range is -1 to 15, but we will use only odd numbers betweeen
    // -1 and 31 we can get those by 2*ksize_slider +1
    int ksize = 2 * ksize_slider + 1;

    // Apply Sobel operator in the x and y directions
    Sobel(frame_gray, grad_x, ddepth, 1, 0, ksize, scale, delta, BORDER_DEFAULT);
    Sobel(frame_gray, grad_y, ddepth, 0, 1, ksize, scale, delta, BORDER_DEFAULT);

    // Convert the gradient values to absolute values
    convertScaleAbs(grad_x, abs_grad_x);
    convertScaleAbs(grad_y, abs_grad_y);

    // Combine the gradients using equal weights
    addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);

    // Calculate frame rate
    frame_count++;
    clock_gettime(CLOCK_MONOTONIC, &curr_t);
    curr_time = (double)curr_t.tv_sec + ((double)curr_t.tv_nsec) / 1000000000.0;
    frame_rate = (double)frame_count / (curr_time - start_time);

    // Convert frame rate to string
    sprintf(frameratetext, "fr: %.3f fc: %lld ctime: %.3f stime: %.3f", frame_rate, frame_count, curr_time, start_time);

    // Display frame rate on the output image
    cv::putText(grad, frameratetext, Point(30, 30), FONT_HERSHEY_COMPLEX_SMALL,
                0.8, Scalar(200, 200, 250), 1, LINE_AA);

    // Show the output image
    imshow("Sobel Demo - Simple Edge Detector", grad);
}

int main(int argc, char **argv)
{
    // Open the default camera
    VideoCapture cam0(0);
    if (!cam0.isOpened())
    {
        return EXIT_FAILURE;
    }

    // Set the frame width and height
    cam0.set(CAP_PROP_FRAME_WIDTH, 640);
    cam0.set(CAP_PROP_FRAME_HEIGHT, 480);

    bool showNormal = true;
    bool showCanny = false;
    bool showSobel = false;

    // Create a window to display the normal frame
    namedWindow("Normal Frame", WINDOW_AUTOSIZE);

    // Get the current time as the start time
    clock_gettime(CLOCK_MONOTONIC, &curr_t);
    start_time = (double)curr_t.tv_sec + ((double)curr_t.tv_nsec) / 1000000000.0;

    while (1)
    {
        // Read a frame from the camera
        cam0.read(frame);

        if (showSobel)
        {
            // Remove noise by blurring with a Gaussian filter ( kernel size = 3 )
            GaussianBlur(frame, frame_blurred, Size(3, 3), 0, 0, BORDER_DEFAULT);
            // Convert the frame to grayscale
            cvtColor(frame_blurred, frame_gray, COLOR_BGR2GRAY);

            // Apply Sobel edge detection
            sobelDemo(0, 0);
        }
        else if (showCanny)
        {
            // Create an output image with the same size and type as the input frame
            dst.create(frame.size(), frame.type());

            // Convert the frame to grayscale
            cvtColor(frame, frame_gray, COLOR_BGR2GRAY);

            // Apply Canny edge detection
            CannyThreshold(0, 0);
        }
        else if (showNormal)
        {
            // Calculate frame rate
            frame_count++;
            clock_gettime(CLOCK_MONOTONIC, &curr_t);
            curr_time = (double)curr_t.tv_sec + ((double)curr_t.tv_nsec) / 1000000000.0;
            frame_rate = (double)frame_count / (curr_time - start_time);

            // Convert frame rate to string
            sprintf(frameratetext, "fr: %.3f fc: %lld ctime: %.3f stime: %.3f", frame_rate, frame_count, curr_time, start_time);

            // Display frame rate on the normal frame
            cv::putText(frame, frameratetext, Point(30, 30), FONT_HERSHEY_COMPLEX_SMALL,
                        0.8, Scalar(200, 200, 250), 1, LINE_AA);

            // Show the normal frame
            imshow("Normal Frame", frame);
        }

        // Wait for a key press
        char key = (char)waitKey(1);
        if (key == 27)
        {
            // Exit the program if the Esc key is pressed
            destroyAllWindows();
            return EXIT_SUCCESS;
        }

        if (key == 'c' || key == 'C')
        {
            // Switch to Canny edge detection mode
            showSobel = false;
            showNormal = false;
            showCanny = true;
            destroyAllWindows();

            // Create a window to display the Canny edge map
            namedWindow("Canny Edge Map", WINDOW_AUTOSIZE);
            // Create a trackbar to adjust the minimum threshold for Canny edge detection
            createTrackbar("Min Threshold:", "Canny Edge Map", &lowThreshold, max_lowThreshold, CannyThreshold);

            // Reset frame count and start time
            frame_count = 0;
            clock_gettime(CLOCK_MONOTONIC, &curr_t);
            start_time = (double)curr_t.tv_sec + ((double)curr_t.tv_nsec) / 1000000000.0;
        }
        if (key == 's' || key == 'S')
        {
            // Switch to Sobel edge detection mode
            showSobel = true;
            showNormal = false;
            showCanny = false;
            destroyAllWindows();

            // Create a window to display the Sobel demo
            namedWindow("Sobel Demo - Simple Edge Detector", WINDOW_AUTOSIZE);
            // Add sliders for ksize, scale, and delta
            createTrackbar("ksize", "Sobel Demo - Simple Edge Detector", &ksize_slider, 15, sobelDemo);
            setTrackbarMin("ksize", "Sobel Demo - Simple Edge Detector", -1);
            createTrackbar("scale", "Sobel Demo - Simple Edge Detector", &scale, 30, sobelDemo);
            setTrackbarMin("scale", "Sobel Demo - Simple Edge Detector", 1);
            createTrackbar("delta", "Sobel Demo - Simple Edge Detector", &delta, 30, sobelDemo);
            setTrackbarMin("delta", "Sobel Demo - Simple Edge Detector", 0);

            // Reset frame count and start time
            frame_count = 0;
            clock_gettime(CLOCK_MONOTONIC, &curr_t);
            start_time = (double)curr_t.tv_sec + ((double)curr_t.tv_nsec) / 1000000000.0;
        }
        if (key == 'n' || key == 'N')
        {
            // Switch to normal frame mode
            showSobel = false;
            showNormal = true;
            showCanny = false;
            destroyAllWindows();

            // Create a window to display the normal frame
            namedWindow("Normal Frame", WINDOW_AUTOSIZE);

            // Reset frame count and start time
            frame_count = 0;
            clock_gettime(CLOCK_MONOTONIC, &curr_t);
            start_time = (double)curr_t.tv_sec + ((double)curr_t.tv_nsec) / 1000000000.0;
        }
    }

    return EXIT_SUCCESS;
}
