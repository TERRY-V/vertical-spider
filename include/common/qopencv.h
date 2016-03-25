/********************************************************************************************
**
** Copyright (C) 2010-2016 Terry Niu (Beijing, China)
** Filename:	qopencv.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2016/01/27
**
*********************************************************************************************/

#ifndef __QOPENCV_H_
#define __QOPENCV_H_

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <opencv2/opencv.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/nonfree/nonfree.hpp>

#include "qglobal.h"

Q_BEGIN_NAMESPACE

// g++编译时需包含`pkg-config --libs --cflags opencv`参数

// 图像尺寸
static int getImageSize(const char* fileName, int* width, int* height)
{
	cv::Mat img = cv::imread(fileName);
	if(img.empty()) {
		printf("getImageSize error, unable to decode (%s)!", fileName);
		return -1;
	}

	cv::Size size = img.size();
	*width = size.width;
	*height = size.height;

	cv::waitKey(0);
	return 0;
}

// 图像截取
static int getSubImage(const char* fileName, const char* newFileName, int x, int y, int width, int height)
{
	cv::Mat img = cv::imread(fileName);
	if(img.empty()) {
		printf("getSubImage error, unable to decode (%s)!", fileName);
		return -1;
	}

	cv::Mat dstImage;
	cv::Rect rect(x, y, width, height);
	img(rect).copyTo(dstImage);
	cv::imwrite(newFileName, dstImage);

	cv::waitKey(0);
	return 0;
}

// 图像腐蚀
static int erode(const char* fileName, const char* newFileName)
{
	cv::Mat img = cv::imread(fileName);
	if(img.empty()) {
		printf("erode error, unable to decode (%s)!", fileName);
		return -1;
	}

	cv::Mat dstImage;
	cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(15, 15));
	cv::erode(img, dstImage, element);
	cv::imwrite(newFileName, dstImage);

	cv::waitKey(0);
	return 0;
}

// 图像模糊
static int blur(const char* fileName, const char* newFileName)
{
	cv::Mat img = cv::imread(fileName);
	if(img.empty()) {
		printf("erode error, unable to decode (%s)!", fileName);
		return -1;
	}

	cv::Mat dstImage;
	cv::blur(img, dstImage, cv::Size(7, 7));
	cv::imwrite(newFileName, dstImage);

	cv::waitKey(0);
	return 0;
}

// canny边缘检测
static int canny(const char* fileName, const char* newFileName)
{
	cv::Mat img = cv::imread(fileName);
	if(img.empty()) {
		printf("erode error, unable to decode (%s)!", fileName);
		return -1;
	}

	cv::Mat dstImage, edge, grayImage;
	dstImage.create(img.size(), img.type());
	// 转灰度图像
	cv::cvtColor(img, grayImage, CV_BGR2GRAY);
	cv::blur(grayImage, edge, cv::Size(3, 3));
	cv::Canny(edge, edge, 3, 9, 3);
	cv::imwrite(newFileName, edge);

	cv::waitKey(0);
	return 0;
}

// 尺寸调整
static int resize(const char* fileName, const char* newFileName, float ratio = 0.5)
{
	cv::Mat img = cv::imread(fileName);
	if(img.empty()) {
		printf("erode error, unable to decode (%s)!", fileName);
		return -1;
	}

	cv::Mat dstImage;
	cv::resize(img, dstImage, cv::Size(img.cols*ratio, img.rows*ratio), (0, 0), (0, 0), 3);
	cv::imwrite(newFileName, dstImage);

	cv::waitKey(0);
	return 0;
}

// SURF特征点检测
static int getSurfFeature(const char* fileName, const char* newFileName)
{
	cv::Mat img = cv::imread(fileName);
	if(img.empty()) {
		printf("erode error, unable to decode (%s)!", fileName);
		return -1;
	}

	int minHessian = 400;
	cv::Mat dstImage;
	std::vector<cv::KeyPoint> keypoints;
	cv::SurfFeatureDetector detector(minHessian);
	detector.detect(img, keypoints);

	drawKeypoints(img, keypoints, dstImage, cv::Scalar::all(-1), cv::DrawMatchesFlags::DEFAULT);
	cv::imwrite(newFileName, dstImage);

	cv::waitKey(0);
	return 0;
}

// FLANN特征点匹配
static int process_flann(const char* fileName1, const char* fileName2, const char* newFileName)
{
	cv::Mat img1 = cv::imread(fileName1, 1);
	if(img1.empty()) {
		printf("erode error, unable to decode (%s)!", fileName1);
		return -1;
	}

	cv::Mat img2 = cv::imread(fileName2, 1);
	if(img2.empty()) {
		printf("erode error, unable to decode (%s)!", fileName2);
		return -2;
	}

	int minHessian = 300;
	cv::SurfFeatureDetector detector(minHessian);
	std::vector<cv::KeyPoint> keypoints1, keypoints2;

	detector.detect(img1, keypoints1);
	detector.detect(img2, keypoints2);

	// 计算描述符(特征向量)
	cv::SurfDescriptorExtractor extractor;
	cv::Mat descriptors1, descriptors2;
	extractor.compute(img1, keypoints1, descriptors1);
	extractor.compute(img2, keypoints2, descriptors2);

	cv::FlannBasedMatcher matcher;
	std::vector<cv::DMatch> matches;
	matcher.match(descriptors1, descriptors2, matches);

	double max_dist=0;
	double min_dist=100;

	for(int i=0; i<descriptors1.rows; ++i) {
		double dist=matches[i].distance;
		if(dist<min_dist) min_dist=dist;
		if(dist>max_dist) max_dist=dist;
	}

	std::vector<cv::DMatch> good_matches;
	for(int i=0; i<descriptors1.rows; ++i) {
		if(matches[i].distance<2*min_dist)
			good_matches.push_back(matches[i]);
	}

	cv::Mat img_matches;
	drawMatches(img1, keypoints1, img2, keypoints2, good_matches, img_matches, cv::Scalar::all(-1), cv::Scalar::all(-1), \
			std::vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

	cv::imwrite(newFileName, img_matches);
	cv::waitKey(0);
}

Q_END_NAMESPACE

#endif // __QOPENCV_H_
