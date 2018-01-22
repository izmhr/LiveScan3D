#include "frameFileWriterReader.h"

#include <ctime>

FrameFileWriterReader::FrameFileWriterReader()
{
	size_t BoolSize = sizeof(bool);
	size_t JointSize = sizeof(Joint);
	size_t JointTypeSize = sizeof(JointType);
	size_t CameraSpacePointSize = sizeof(CameraSpacePoint);
	size_t TrackingStateSize = sizeof(TrackingState);
	size_t Point2fSize = sizeof(Point2f);
	size_t BodySize = sizeof(Body);
}

void FrameFileWriterReader::closeFileIfOpened()
{
	if (m_pFileHandle == nullptr)
		return;

	fclose(m_pFileHandle);
	m_pFileHandle = nullptr; 
	m_bFileOpenedForReading = false;
	m_bFileOpenedForWriting = false;
}

void FrameFileWriterReader::resetTimer()
{
	recording_start_time = std::chrono::steady_clock::now();
}

int FrameFileWriterReader::getRecordingTimeMilliseconds()
{
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	return static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds >(end - recording_start_time).count());
}

void FrameFileWriterReader::openCurrentFileForReading()
{
	closeFileIfOpened();

	m_pFileHandle = fopen(m_sFilename.c_str(), "rb");

	m_bFileOpenedForReading = true;
	m_bFileOpenedForWriting = false;
}

void FrameFileWriterReader::openNewFileForWriting()
{
	closeFileIfOpened();

	char filename[1024];
	time_t t = time(0);
	struct tm * now = localtime(&t);
	sprintf(filename, "recording_%04d_%02d_%02d_%02d_%02d_%02d.bin", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
	m_sFilename = filename; 
	m_pFileHandle = fopen(filename, "wb");

	m_bFileOpenedForReading = false;
	m_bFileOpenedForWriting = true;

	resetTimer();
}

bool FrameFileWriterReader::readFrame(std::vector<Point3s> &outPoints, std::vector<RGB> &outColors, std::vector<Body> &outBodies, long long* capturedTime)
{
	if (!m_bFileOpenedForReading)
		openCurrentFileForReading();

	outPoints.clear();
	outColors.clear();
	outBodies.clear();
	FILE *f = m_pFileHandle;
	int nPoints, timestamp; 
	char tmp[1024]; 
	int nread = fscanf_s(f, "%s %d %s %lld", tmp, 1024, &nPoints, tmp, 1024, capturedTime);

	if (nread < 4)
		return false;

	if (nPoints == 0)
		return true;

	fgetc(f);		//  '\n'
	outPoints.resize(nPoints);
	outColors.resize(nPoints);

	SaveJoints saveJoints[6];

	fread((void*)outPoints.data(), sizeof(outPoints[0]), nPoints, f);
	fread((void*)outColors.data(), sizeof(outColors[0]), nPoints, f);
	fread(saveJoints, sizeof(SaveJoints), 6, f);
	fgetc(f);		// '\n'

	// array to vector
	for (int i = 0; i < 6; i++) {
		Body body;
		body.bTracked = saveJoints[i].bTracked;
		for (int j = 0; j < 25; j++) {
			body.vJoints[j] = saveJoints[i].joints[j];
			body.vJointsInColorSpace[j] = saveJoints[i].jointsInColorSpace[j];
		}
		outBodies.push_back(body);
	}

	return true;

}


void FrameFileWriterReader::writeFrame(std::vector<Point3s> points, std::vector<RGB> colors, std::vector<Body> bodies, long long captureTime)
{
	if (!m_bFileOpenedForWriting)
		openNewFileForWriting();

	FILE *f = m_pFileHandle;

	int nPoints = static_cast<int>(points.size());
	fprintf(f, "n_points= %d\nframe_timestamp= %lld\n", nPoints, captureTime);
	
	SaveJoints saveJoints[6];
	for (int i = 0; i < 6; i++) {
		saveJoints[i].bTracked = bodies[i].bTracked;
		for (int j = 0; j < 25; j++)
		{
			saveJoints[i].joints[j] = bodies[i].vJoints[j];
			saveJoints[i].jointsInColorSpace[j] = bodies[i].vJointsInColorSpace[j];
		}
	}
	if (nPoints > 0)
	{
		fwrite((void*)points.data(), sizeof(points[0]), nPoints, f);
		fwrite((void*)colors.data(), sizeof(colors[0]), nPoints, f);
		fwrite(saveJoints, sizeof(SaveJoints), 6, f);
	}
	fprintf(f, "\n");
}

FrameFileWriterReader::~FrameFileWriterReader()
{
	closeFileIfOpened();
}