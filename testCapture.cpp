#include <Windows.h>
#include <stdint.h>
#include <stdio.h>
#include "cv.h"
#include "highgui.h"
using namespace cv;
void Screen();
BOOL HBitmapToMat(HBITMAP& _hBmp,Mat& _mat);
HBITMAP	hBmp;
HBITMAP	hOld;
 
void main()
{
	while(1)
	{
		Mat src;
		Mat dst;
		//屏幕截图
		Screen();
		//类型转换
		HBitmapToMat(hBmp,src);
		//调整大小
		resize(src,dst,cvSize(640,400),0,0);
		imshow("dst",dst);
		DeleteObject(hBmp);
		waitKey(1);//这里调节帧数  现在200ms是5帧
	}
}
 
//抓取当前屏幕函数
void Screen(){
	HWND hWnd=FindWindow(NULL,"image");
	HDC hScreen;
    int32_t ScrWidth = 0, ScrHeight = 0;
    RECT rect = { 0 };
    if (hWnd == NULL)
    {
		hScreen	= CreateDC("DISPLAY", NULL, NULL, NULL);
        ScrWidth = GetDeviceCaps(hScreen, HORZRES);
        ScrHeight = GetDeviceCaps(hScreen, VERTRES);
    }
    else
    {
		hScreen	= GetDC(hWnd);
        GetWindowRect(hWnd, &rect);
        ScrWidth = rect.right - rect.left;
        ScrHeight = rect.bottom - rect.top;
    }
	//创建画板
	//HDC hScreen	= CreateDC("DISPLAY", NULL, NULL, NULL);
	HDC	hCompDC	= CreateCompatibleDC(hScreen);
	//取屏幕宽度和高度
	//int		nWidth	= GetSystemMetrics(SM_CXSCREEN);	
	//int		nHeight	= GetSystemMetrics(SM_CYSCREEN);
	//创建Bitmap对象
	hBmp	= CreateCompatibleBitmap(hScreen, ScrWidth, ScrHeight);
	hOld	= (HBITMAP)SelectObject(hCompDC, hBmp);
	BitBlt(hCompDC, 0, 0, ScrWidth, ScrHeight, hScreen, 0, 0, SRCCOPY);	
	SelectObject(hCompDC, hOld);
	//释放对象
	DeleteDC(hScreen);	
	DeleteDC(hCompDC);
}
//把HBITMAP型转成Mat型
BOOL HBitmapToMat(HBITMAP& _hBmp,Mat& _mat)
{
	//BITMAP操作
	BITMAP bmp;    
	GetObject(_hBmp,sizeof(BITMAP),&bmp);    
	int nChannels = bmp.bmBitsPixel == 1 ? 1 : bmp.bmBitsPixel/8 ;   
	int depth = bmp.bmBitsPixel == 1 ? IPL_DEPTH_1U : IPL_DEPTH_8U;    
	//mat操作
	Mat v_mat;
	v_mat.create(cvSize(bmp.bmWidth,bmp.bmHeight), CV_MAKETYPE(CV_8U,nChannels));
	GetBitmapBits(_hBmp,bmp.bmHeight*bmp.bmWidth*nChannels,v_mat.data);  
	_mat=v_mat;
	return TRUE;   
}