#include <Windows.h>
#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include "opencv2/core/core.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;

char* imgFile1 = "C:/Users/Administrator/Desktop/image/image1/cat.jpg";
void normalMatch(Mat &descriptors_object, Mat &descriptors_scene, std::vector<DMatch> &good_matches);
void knnMatch(Mat &descriptors_object, Mat &descriptors_scene, std::vector<DMatch> &good_matches);

void Screen();
BOOL HBitmapToMat(HBITMAP& _hBmp, Mat& _mat);
HBITMAP	hBmp;
HBITMAP	hOld;

int main()
{
	Mat img_object = imread(imgFile1, CV_LOAD_IMAGE_GRAYSCALE);
	if (!img_object.data)
	{
		std::cout << " --(!) Error reading images " << std::endl; 
		return -1;
	}
	//--------------------------------------------------------
	//-- Step 1: Detect the keypoints using SURF Detector
	//--------------------------------------------------------
	int minHessian = 800;
	SurfFeatureDetector detector(minHessian);
	std::vector<KeyPoint> keypoints_object, keypoints_scene;
	detector.detect(img_object, keypoints_object);

	//--------------------------------------------------------
	//-- Step 2: Calculate descriptors (feature vectors)
	//--------------------------------------------------------
	SurfDescriptorExtractor extractor;
	Mat descriptors_object, descriptors_scene;
	extractor.compute(img_object, keypoints_object, descriptors_object);
	//extractor.compute(img_scene, keypoints_scene, descriptors_scene);

	while (1)
	{
		Mat src;
		Mat dst;
		//屏幕截图
		Screen();
		//类型转换
		//HBitmapToMat(hBmp, src);
		//调整大小
		//resize(src, dst, cvSize(800, 450), 0, 0);
		
		HBitmapToMat(hBmp, dst);

		detector.detect(dst, keypoints_scene);
		extractor.compute(dst, keypoints_scene, descriptors_scene);
		std::vector< DMatch > good_matches;
		knnMatch(descriptors_object, descriptors_scene, good_matches);
		if (good_matches.size() >= 4) {
			std::vector<Point2f> obj;
			std::vector<Point2f> scene;
			for (int i = 0; i < good_matches.size(); i++)
			{
				//-- Get the keypoints from the good matches
				obj.push_back(keypoints_object[good_matches[i].queryIdx].pt);
				scene.push_back(keypoints_scene[good_matches[i].trainIdx].pt);
			}
			Mat H = findHomography(obj, scene, CV_RANSAC);

			//-- Get the corners from the image_1 ( the object to be "detected" )
			std::vector<Point2f> obj_corners(4);
			obj_corners[0] = cvPoint(0, 0);
			obj_corners[1] = cvPoint(img_object.cols, 0);
			obj_corners[2] = cvPoint(img_object.cols, img_object.rows);
			obj_corners[3] = cvPoint(0, img_object.rows);
			std::vector<Point2f> scene_corners(4);
			perspectiveTransform(obj_corners, scene_corners, H);
			//-- Draw lines between the corners (the mapped object in the scene - image_2 )
			line(dst, scene_corners[0], scene_corners[1], Scalar(0, 255, 255, 1), 5);
			line(dst, scene_corners[1], scene_corners[2], Scalar(0, 0, 0, 1), 5);
			line(dst, scene_corners[2], scene_corners[3], Scalar(0, 0, 0, 1), 5);
			line(dst, scene_corners[3], scene_corners[0], Scalar(0, 0, 0, 1), 5);
		}
		else {
			//std::cout << "未检测到" << std::endl;
		}

		imshow("dst", dst);
		DeleteObject(hBmp);
		waitKey(1);//这里调节帧数  现在200ms是5帧
	}
}

//抓取当前屏幕函数
void Screen() {
	//HWND hWnd = FindWindow(NULL, "image");
	HWND hWnd = FindWindow(NULL, "merge1 - 美图看看 2.7.8");
	//HWND hWnd = FindWindow(NULL, "image");
	HDC hScreen;
	int32_t ScrWidth = 0, ScrHeight = 0;
	RECT rect = { 0 };
	if (hWnd == NULL)
	{
		hScreen = CreateDC("DISPLAY", NULL, NULL, NULL);
		ScrWidth = GetDeviceCaps(hScreen, HORZRES);
		ScrHeight = GetDeviceCaps(hScreen, VERTRES);
	}
	else
	{
		hScreen = GetDC(hWnd);
		GetWindowRect(hWnd, &rect);
		ScrWidth = rect.right - rect.left;
		ScrHeight = rect.bottom - rect.top;
	}
	//创建画板
	//HDC hScreen	= CreateDC("DISPLAY", NULL, NULL, NULL);
	HDC	hCompDC = CreateCompatibleDC(hScreen);
	//取屏幕宽度和高度
	//int		nWidth	= GetSystemMetrics(SM_CXSCREEN);	
	//int		nHeight	= GetSystemMetrics(SM_CYSCREEN);
	//创建Bitmap对象
	hBmp = CreateCompatibleBitmap(hScreen, ScrWidth, ScrHeight);
	hOld = (HBITMAP)SelectObject(hCompDC, hBmp);
	BitBlt(hCompDC, 0, 0, ScrWidth, ScrHeight, hScreen, 0, 0, SRCCOPY);
	SelectObject(hCompDC, hOld);
	//释放对象
	DeleteDC(hScreen);
	DeleteDC(hCompDC);
}
//把HBITMAP型转成Mat型
BOOL HBitmapToMat(HBITMAP& _hBmp, Mat& _mat)
{
	//BITMAP操作
	BITMAP bmp;
	GetObject(_hBmp, sizeof(BITMAP), &bmp);
	int nChannels = bmp.bmBitsPixel == 1 ? 1 : bmp.bmBitsPixel / 8;
	int depth = bmp.bmBitsPixel == 1 ? IPL_DEPTH_1U : IPL_DEPTH_8U;
	//mat操作
	Mat v_mat;
	v_mat.create(cvSize(bmp.bmWidth, bmp.bmHeight), CV_MAKETYPE(CV_8U, nChannels));
	GetBitmapBits(_hBmp, bmp.bmHeight*bmp.bmWidth*nChannels, v_mat.data);
	_mat = v_mat;
	return TRUE;
}

void normalMatch(Mat &descriptors_object, Mat &descriptors_scene, std::vector<DMatch> &good_matches) {
	FlannBasedMatcher matcher;
	std::vector< DMatch > matches;
	matcher.match(descriptors_object, descriptors_scene, matches);
	double max_dist = 0; double min_dist = 100;

	//-- Quick calculation of max and min distances between keypoints
	for (int i = 0; i < descriptors_object.rows; i++)
	{
		double dist = matches[i].distance;
		if (dist < min_dist) min_dist = dist;
		if (dist > max_dist) max_dist = dist;
	}
	printf("-- Max dist : %f \n", max_dist);
	printf("-- Min dist : %f \n", min_dist);

	//-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
	//std::vector< DMatch > good_matches;
	for (int i = 0; i < descriptors_object.rows; i++)
	{
		if (matches[i].distance < 2 * min_dist) {
			good_matches.push_back(matches[i]);
		}
	}
	std::cout << "total match points: " << matches.size() << std::endl;
	std::cout << "good match points: " << good_matches.size() << std::endl;
}

void knnMatch(Mat &descriptors_object, Mat &descriptors_scene, std::vector<DMatch> &good_matches) {
	FlannBasedMatcher matcher;
	const float minRatio = 1.f / 1.5f;
	const int k = 2;
	vector<vector<DMatch>> knnMatches;
	matcher.knnMatch(descriptors_object, descriptors_scene, knnMatches, k);
	for (size_t i = 0; i < knnMatches.size(); i++) {
		const DMatch& bestMatch = knnMatches[i][0];
		const DMatch& betterMatch = knnMatches[i][1];
		float  distanceRatio = bestMatch.distance / betterMatch.distance;
		if (distanceRatio < minRatio) {
			good_matches.push_back(bestMatch);
		}
	}
	std::cout << "total match points: " << knnMatches.size() << std::endl;
	std::cout << "good match points: " << good_matches.size() << std::endl;
}