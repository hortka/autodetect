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
		//��Ļ��ͼ
		Screen();
		//����ת��
		HBitmapToMat(hBmp,src);
		//������С
		resize(src,dst,cvSize(640,400),0,0);
		imshow("dst",dst);
		DeleteObject(hBmp);
		waitKey(1);//�������֡��  ����200ms��5֡
	}
}
 
//ץȡ��ǰ��Ļ����
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
	//��������
	//HDC hScreen	= CreateDC("DISPLAY", NULL, NULL, NULL);
	HDC	hCompDC	= CreateCompatibleDC(hScreen);
	//ȡ��Ļ��Ⱥ͸߶�
	//int		nWidth	= GetSystemMetrics(SM_CXSCREEN);	
	//int		nHeight	= GetSystemMetrics(SM_CYSCREEN);
	//����Bitmap����
	hBmp	= CreateCompatibleBitmap(hScreen, ScrWidth, ScrHeight);
	hOld	= (HBITMAP)SelectObject(hCompDC, hBmp);
	BitBlt(hCompDC, 0, 0, ScrWidth, ScrHeight, hScreen, 0, 0, SRCCOPY);	
	SelectObject(hCompDC, hOld);
	//�ͷŶ���
	DeleteDC(hScreen);	
	DeleteDC(hCompDC);
}
//��HBITMAP��ת��Mat��
BOOL HBitmapToMat(HBITMAP& _hBmp,Mat& _mat)
{
	//BITMAP����
	BITMAP bmp;    
	GetObject(_hBmp,sizeof(BITMAP),&bmp);    
	int nChannels = bmp.bmBitsPixel == 1 ? 1 : bmp.bmBitsPixel/8 ;   
	int depth = bmp.bmBitsPixel == 1 ? IPL_DEPTH_1U : IPL_DEPTH_8U;    
	//mat����
	Mat v_mat;
	v_mat.create(cvSize(bmp.bmWidth,bmp.bmHeight), CV_MAKETYPE(CV_8U,nChannels));
	GetBitmapBits(_hBmp,bmp.bmHeight*bmp.bmWidth*nChannels,v_mat.data);  
	_mat=v_mat;
	return TRUE;   
}