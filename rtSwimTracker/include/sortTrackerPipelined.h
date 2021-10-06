#ifndef SORTTRACKERPIPELINED_H
#define SORTTRACKERPIPELINED_H

#include <iostream>
#include <set>
#include <vector>

#include "KalmanTracker.h"
#include "SORTtrackingBox.h"

#include <opencv2/core.hpp>

//Implementation used from 
//https://github.com/mcximing/sort-cpp


class sortTrackerPiplelined
{
public:

	sortTrackerPiplelined();
	std::vector<TrackingBox> singleFrameSORT(std::vector<TrackingBox> swimmerDetections);

private:
	void inputDetectionData(const std::vector<TrackingBox>& detFrameData)
	{ 
		m_frameData = detFrameData; 
		m_numDetections = static_cast<int>(detFrameData.size());
	}

	void initializeTrackersUsing(const std::vector<TrackingBox>& detFrameData);
	void processFrame();
	std::vector<cv::Rect_<float>>& createTrajecotoryPredictions(std::vector<cv::Rect_<float>>& initializedValue);
	std::vector<std::vector<double>>& constructIOUCostMat(const std::vector<cv::Rect_<float>>& trajectoryPredictions, std::vector<std::vector<double>>& iouCostMatrix);
	std::vector<cv::Point>& matchDetectionsToTrajectories(const std::vector<std::vector<double>>& iouCostMatrix, std::vector<cv::Point>& pairs);
	void updateTrackers(const std::vector<cv::Point>& pairs);
	void createNewTrackersWithLeftoverDetections();
	void collectResultsWhileKillingTrackers();
	void fillUnmatchedDetections(const std::vector<int>& assignments);
	void fillUnmatchedTrajectories(const std::vector<int>& assignments);
	double GetIOU(cv::Rect_<float> bb_test, cv::Rect_<float> bb_gt);

	std::vector<KalmanTracker> m_vectorOfTrackers;
	std::vector<TrackingBox> m_frameData;
	std::vector<TrackingBox> m_frameTrackingResults;
	std::set<int> m_unmatchedDetections;
	std::set<int> m_unmatchedTrajectories;
	int m_numTrajectories;
	int m_numDetections;
	int m_numberFramesProcessed;

	const double m_iou = 0.05; //Orignaly this value was 0.30
	const int m_maxUpdateAllowance = 1;
	const int m_minHitsInFrames = 3;
};

#endif // !SORTTRACKERPIPELINED_H

