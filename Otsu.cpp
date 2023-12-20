#include <iostream>
// #include <filesystem>
#include <vector>
#include <algorithm>
#include <stdio.h>
// #include <unistd.h>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>


// Method that is used to compute the histogram color values 
// This Method only calculates values for gray-scaled Images
std::vector<int> computeHistogram(cv::Mat image)
{
    // cv::Mat histogram;
    // int histSize = 256;
    // float range[] = { 0, 256 };
    // const float* histRange[] = { range };
    // int channels[] = {0};
    // cv::calcHist(&image, 1, channels, cv::Mat(), histogram, 1, &histSize, histRange, true, false);

    std::vector<int> histogram(255);

    for (int y = 0; y < image.size().height; y++)
    {
        for (int x = 0; x < image.size().width; x++)
        {
            cv::Scalar intensity = image.at<cv::Vec3b>(cv::Point(y, x));
            int value = intensity[0];
            histogram[value]++;
        }
    }

    return histogram;
}


// Does the numerator calculation of the standard deviation equation
double numeratorCalculation(std::vector<int> data, double mean)
{
    double summation = 0;

    for (auto &value : data)
    {
        double squaredValue = std::pow((value - mean), 2);
        summation += squaredValue;
    }

    return summation;
}

// Calculates the standard deviation given the data and size
double calculateStandardDeviation(std::vector<int> data, int size)
{
    double mean, sum = 0;

    for (auto &value : data)
    {
        sum += value;
    }

    mean = sum / size;

    double numerator = numeratorCalculation(data, mean);

    return std::pow((numerator / size), 0.5);
}

// Calculates the standard deviation given only the data
double calculateStandardDeviation(std::vector<int> data)
{
    double mean, sum = 0;

    for (auto &value : data)
    {
        sum += value;
    }

    mean = sum / data.size();

    double numerator = numeratorCalculation(data, mean);

    return std::pow((numerator / data.size()), 0.5);
}

// Gets part of a vector from a whole vector given the start and end position of the vector and the 
// the whole vector
std::vector<int> getPartOfVector(int start, int end, std::vector<int> data)
{
    std::vector<int> freshVector;

    for (int i = start; i < end; i++)
    {
        freshVector.push_back(data[i]);
    }

    return freshVector;
}

// Calculates the max value out of the vector and returns the index of the max value
int max (std::vector<int> histogram) {
    int max = -1;
    int index = -1;
    for (int i = 0; i < histogram.size(); i++) {
        if (histogram[i] > max) {
            max = histogram[i];
            index = i;
        }
    }
    return index;
}
// Shit method that wrote because I'm stupid
int determineMiddlePoint(std::vector<int> histogramFirst, std::vector<int>histogramSecond, std::vector<int> originalHistogram) {
    int firstHistoMax = max(histogramFirst);
    int secondHistoMax = max(histogramSecond);

    int min = INT_MAX;
    int index = -1;
    for (int i = firstHistoMax; i < histogramFirst.size() + secondHistoMax; i++) {
        if (originalHistogram[i] < min) {
            min = originalHistogram[i];
            index = i;
        }
    }

    return index;
}

// Generates the new image from the Otsu Method or the all the previous methods that I wrote above 
// this method, which is now used to create a new image.
cv::Mat newImage(cv::Mat originalImage, std::vector<int> histogram, double threshold, int minIndex)
{
    cv::Mat otsuImage(originalImage.size().width, originalImage.size().height, CV_8UC3, cv::Scalar(255, 255, 255));
    cv::Vec3b white(255, 255, 255);
    cv::Vec3b black(0, 0, 0);

    // Change the comparison value to get different grayscale images. Ex: histogram.size / 2, middle point, middle point + whatever value you want.
    // for (int i = 0; i < histogram.size(); i++) {
    //     double intensity = histogram[i];
    //     if (intensity >= threshold) 
    //     {
    //         cv::Vec3b color(i, i, i);
    //         for (int y = 0; y < originalImage.size().height; y++)
    //         {
    //             for (int x = 0; x < originalImage.size().width; x++)
    //             {
    //                 cv::Vec3b pixelValue = originalImage.at<cv::Vec3b>(cv::Point(y, x));
    //                 if (pixelValue == color) 
    //                 {
    //                     otsuImage.at<cv::Vec3b>(cv::Point(y, x)) = black;
    //                 }
    //             }
    //         }
    //     }
    // }

    for (int y = 0; y < originalImage.size().height; y++) {
        for (int x = 0; x < originalImage.size().width; x++) {
            cv::Vec3b pixelValue = originalImage.at<cv::Vec3b>(cv::Point(y, x));
            int colorValue = pixelValue.val[0];
            if (histogram[colorValue] > threshold && colorValue < minIndex) {
                otsuImage.at<cv::Vec3b>(cv::Point(y, x)) = black;
            }
            
        }
    }

    return otsuImage;
}

// generates all possible thresholds and stores in a vector
std::vector<double> getAllThresholds (std::vector<int> histogram) {
    std::vector<double> thresholds;

    for (int i = 0; i < histogram.size(); i++) {
        std::vector<int> firstHalf = getPartOfVector(0, i, histogram);
        std::vector<int> secondHalf = getPartOfVector(i, histogram.size(), histogram);

        double firstHalfSTD = calculateStandardDeviation(firstHalf, firstHalf.size());
        double secondHalfSTD = calculateStandardDeviation(secondHalf, secondHalf.size());
        double threshold = std::abs(firstHalfSTD - secondHalfSTD);
        thresholds.push_back(threshold);
    }

    return thresholds;

}


// used to determine the lowest threshold value that is used in the Otsu method
int min (std::vector<double> thresholds) {
    int min = INT_MAX;
    int index = -1;
    for (int i = 0; i < thresholds.size(); i++) {
        if (thresholds[i] < min) {
            min = thresholds[i];
            index = i;
        }
    }
    return index;
}

int main(int argc, char *argv[])
{
    try
    {
        // store the directory path into string path from the command line
        std::string path = argv[1];
        cv::Mat grayscaleImage = cv::imread(path);
        std::vector<int> values = computeHistogram(grayscaleImage);
        // std::cout << calculateStandardDeviation(values) << std::endl;

        // std::vector<int> firstHalf = getPartOfVector(0, values.size() / 2, values);
        // std::vector<int> secondHalf = getPartOfVector((values.size() / 2) + 1, values.size(), values);

        // double firstHalfSTD = calculateStandardDeviation(firstHalf, firstHalf.size());
        // double secondHalfSTD = calculateStandardDeviation(secondHalf, secondHalf.size());
        // double threshold = std::abs(firstHalfSTD - secondHalfSTD);
        // std::cout << firstHalfSTD << std::endl;
        // std::cout << secondHalfSTD << std::endl;
        // std::cout << threshold << std::endl;

        // int middlePoint = determineMiddlePoint(firstHalf, secondHalf, values);

        // std::cout << middlePoint << std::endl;

        std::vector<double> thresholds = getAllThresholds(values);
        int count = 0;
        for (auto &value : thresholds) {
            std::cout << value << std::endl;
            count++;
        }
        std::cout << count << std::endl;
        int minIndex = min(thresholds);
        double min = thresholds[minIndex];
        cv::Mat otsuImage = newImage(grayscaleImage, values, min, minIndex);
        cv::imwrite("OtsuImage.png", otsuImage);
    }
    catch (std::exception &e)
    {
        std::cout << "Directory not opening" << std::endl;
    }

    return 0;
}
