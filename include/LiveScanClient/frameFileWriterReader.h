#pragma once

#include <stdio.h>
#include <string>
#include <vector>
#include <chrono>
#include "utils.h"
#include "KinectCapture.h"

typedef struct _SaveJoints
{
	bool bTracked;
	Joint SpineBase;
	Joint SpineMid;
	Joint SpineShoulder;
	Joint HipLeft;
	Joint HipRight;
} SaveJoints;

class FrameFileWriterReader
{
public:
    FrameFileWriterReader();
	void openNewFileForWriting();
	void openCurrentFileForReading();
	
	// leave filename blank if you want the filename to be generated from the date
	void setCurrentFilename(std::string filename = ""); 

	void writeFrame(std::vector<Point3s> points, std::vector<RGB> colors, std::vector<Body> bodies, long long captureTime);
	bool readFrame(std::vector<Point3s> &outPoints, std::vector<RGB> &outColors, SaveJoints *saveJoints, long long* capturedTime);

	bool openedForWriting() { return m_bFileOpenedForWriting; }
	bool openedForReading() { return m_bFileOpenedForReading; }


	void closeFileIfOpened();

    ~FrameFileWriterReader();

private:
	void resetTimer();
	int getRecordingTimeMilliseconds();

	FILE *m_pFileHandle = nullptr;
	bool m_bFileOpenedForWriting = false; 
	bool m_bFileOpenedForReading = false;

	std::string m_sFilename = "";

	std::chrono::steady_clock::time_point recording_start_time;
};