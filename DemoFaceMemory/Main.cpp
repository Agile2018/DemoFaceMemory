#include <opencv2/core.hpp>
#include <opencv2/core/opengl.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/opencv.hpp>
#include "iface.h"
#include <iostream>
#include "windows.h"
#include <iostream> 
#include <fstream> 
#define MEGABYTE (1024 * 1024)
#define FILE_LOG	"log.txt"
#define FILE_IMAGE	"image.png"

using namespace std;
using namespace cv;

bool flagFrame = false;

void InitLib() {
	int errorCode;

	errorCode = IFACE_Init();


	if (errorCode == IFACE_OK) {
		cout << "License OK" << endl;
		
	}
	else if (errorCode == IFACE_ERR_LICENSE_INTEGRATION_GENERIC) {
		int hwIdLen = 1024;
		char hwId[1024];

		IFACE_GetHardwareId(hwId, &hwIdLen);
		char msg[256];
		sprintf_s(msg, "Your license is invalid or not present, \nplease contact support for license with this HwId %s\n", hwId);
		cout << msg << endl;
	}
	
}

void SetParams() {
	
	int errorCode;
	errorCode = IFACE_SetParam(IFACE_GLOBAL_PARAMETERS,
		IFACE_PARAMETER_GLOBAL_THREAD_NUM, IFACE_GLOBAL_THREAD_NUM_DEFAULT);
	if (errorCode == IFACE_OK) {
		cout << "Params OK" << endl;
	}
	else {
		cout << "Errors Params" << endl;
	}
}

void DetectFace(unsigned char* rawImage, int width, int height) {
	int detectedFaces = 1;
	int minEyeDistance = 25;
	int maxEyeDistance = 200;
	int errorCode;
	int result = 0;
	void* faceTemp;
	void* faceHandler;
	int templateSize;

	errorCode = IFACE_CreateFaceHandler(&faceHandler);
	if (errorCode == IFACE_OK) {
		errorCode = IFACE_CreateFace(&(faceTemp));
		if (errorCode == IFACE_OK) {
			errorCode = IFACE_DetectFaces(rawImage, width, height,
				minEyeDistance, maxEyeDistance, faceHandler, &detectedFaces, &faceTemp);
			if (errorCode == IFACE_OK) {
				if (detectedFaces != 0)
				{
					errorCode = IFACE_CreateTemplate(faceTemp, faceHandler, 0, &templateSize, NULL);
					if (errorCode == IFACE_OK) {
						char* templateData = new char[templateSize];
						errorCode = IFACE_CreateTemplate(faceTemp, faceHandler, 0,
							&templateSize, templateData);
						if (errorCode == IFACE_OK) {
							int majorVersion0, minorVersion0, majorVersion1, minorVersion1, quality0, quality1;
							errorCode = IFACE_GetTemplateInfo(faceHandler, templateData, &majorVersion0, &minorVersion0, &quality0);
							std::cout << "First template version: " << majorVersion0 << "." << minorVersion0 << ", quality: " << quality0
								<< endl;
						}
						else {
							cout << "Create Template: " << errorCode << endl;
						}
						delete[] templateData;
					}

				}
			}
			else {
				cout << "Error Detect Face: " << errorCode << endl;
			}
		}
		else {
			cout << "Error Create Face: " << errorCode << endl;
		}
	}
	else {
		cout << "Error Create Handler: " << errorCode << endl;
	}
	
	errorCode = IFACE_ReleaseEntity(faceHandler);
	if (errorCode != IFACE_OK) {
		cout << "Error Release: " << errorCode << endl;
	}

	errorCode = IFACE_ReleaseEntity(faceTemp);
	if (errorCode != IFACE_OK) {
		cout << "Error Release: " << errorCode << endl;
	}
}

long CheckMemoryCurrentlyAvailable() {

	MEMORYSTATUS MemoryStatus;
	ZeroMemory(&MemoryStatus, sizeof(MEMORYSTATUS));
	MemoryStatus.dwLength = sizeof(MEMORYSTATUS);

	GlobalMemoryStatus(&MemoryStatus);
	if (MemoryStatus.dwTotalPhys == -1)
	{
		MEMORYSTATUSEX MemoryStatusEx;
		GlobalMemoryStatusEx(&MemoryStatusEx);
		return (long)(MemoryStatusEx.ullTotalPhys / (1024 * 1024));
	}
	return (long)((MemoryStatus.dwTotalPhys - MemoryStatus.dwAvailPhys) / MEGABYTE);
}

void writeData(long memory) {
	ofstream file_obj;
	file_obj.open(FILE_LOG, ios::app);
	file_obj << memory << endl;
	file_obj.close();
}

void ProcessImage(Mat image) {
	int lenght, errorCode, width, height;
	imwrite(FILE_IMAGE, image);
	errorCode = IFACE_LoadImage(FILE_IMAGE, &width, &height, &lenght, NULL);
	if (errorCode == IFACE_OK)
	{
		unsigned char* rawImage = new unsigned char[lenght];
		errorCode = IFACE_LoadImage(FILE_IMAGE, &width,
			&height, &lenght, rawImage);
		if (errorCode == IFACE_OK)
		{
			DetectFace(rawImage, width, height);

		}

		delete rawImage;
	}
	else {
		cout << "Error load image" << endl;
	}
	writeData(CheckMemoryCurrentlyAvailable());
	flagFrame = false;
}



int main() {
	SetParams();
	InitLib();
	
	VideoCapture capture;
	capture.open("video1.mp4");
	if (!capture.isOpened())
		return -1;
	
	namedWindow("video", 1);
	for (;;)
	{
		Mat frame;
		capture >> frame;
		if (!flagFrame)
		{
			flagFrame = true;

			std::thread squ(&ProcessImage, frame);
			squ.detach();
		}
		imshow("video", frame);
		if (waitKey(30) >= 0) break;
	}
	
	return 0;
}