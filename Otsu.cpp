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

std::vector<int> getPartOfVector(int start, int end, std::vector<int> data)
{
    std::vector<int> freshVector;

    for (int i = start; i < end; i++)
    {
        freshVector.push_back(data[i]);
    }

    return freshVector;
}

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

cv::Mat newImage(cv::Mat originalImage, std::vector<int> histogram, double threshold, int middlePoint)
{
    cv::Mat otsuImage(originalImage.size().width, originalImage.size().height, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::Vec3b white(255, 255, 255);
    cv::Vec3b black(0, 0, 0);

    // Change the comparison value to get different grayscale images. Ex: histogram.size / 2, middle point, middle point + whatever value you want.
    for (int i = 0; i < histogram.size() / 2; i++) {
        double intensity = histogram[i];
        if (intensity > threshold) 
        {
            cv::Vec3b color(i, i, i);
            for (int y = 0; y < originalImage.size().height; y++)
            {
                for (int x = 0; x < originalImage.size().width; x++)
                {
                    cv::Vec3b pixelValue = originalImage.at<cv::Vec3b>(cv::Point(y, x));
                    if (pixelValue == color) 
                    {
                        otsuImage.at<cv::Vec3b>(cv::Point(y, x)) = white;
                    }
                }
            }
        }
    }
    return otsuImage;
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

        std::vector<int> firstHalf = getPartOfVector(0, values.size() / 2, values);
        std::vector<int> secondHalf = getPartOfVector((values.size() / 2) + 1, values.size(), values);

        double firstHalfSTD = calculateStandardDeviation(firstHalf, firstHalf.size());
        double secondHalfSTD = calculateStandardDeviation(secondHalf, secondHalf.size());
        double threshold = std::abs(firstHalfSTD - secondHalfSTD);
        // std::cout << firstHalfSTD << std::endl;
        // std::cout << secondHalfSTD << std::endl;
        // std::cout << threshold << std::endl;

        int middlePoint = determineMiddlePoint(firstHalf, secondHalf, values);

        std::cout << middlePoint << std::endl;

        cv::Mat otsuImage = newImage(grayscaleImage, values, threshold, middlePoint);
        cv::imwrite("OtsuImage.png", otsuImage);
    }
    catch (std::exception &e)
    {
        std::cout << "Directory not opening" << std::endl;
    }

    return 0;
}