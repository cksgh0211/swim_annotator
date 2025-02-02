#include "KalmanTracker.h"
#include <iostream>

using namespace std;

int KalmanTracker::kf_count = 0;

// initialize Kalman filter
void KalmanTracker::init_kf(StateType stateMat)
{
	float t = float(1) / float(30);//frame rate
	const int stateNum = 7;
	const int measureNum = 4;
	kf = KalmanFilter(stateNum, measureNum, 0);

	measurement = Mat::zeros(measureNum, 1, CV_32F);

	//kf.transitionMatrix = *(Mat_<float>(stateNum, stateNum) <<
	kf.transitionMatrix = (Mat_<float>(stateNum, stateNum) <<
		1, 0, 0, 0, t, 0, 0,
		0, 1, 0, 0, 0, t, 0,
		0, 0, 1, 0, 0, 0, t,
		0, 0, 0, 1, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0,
		0, 0, 0, 0, 0, 1, 0,
		0, 0, 0, 0, 0, 0, 1);

	setIdentity(kf.measurementMatrix);
	setIdentity(kf.processNoiseCov, Scalar::all(1e-2));//movment of swimmers
	setIdentity(kf.measurementNoiseCov, Scalar::all(1e-1));//error of detection
	setIdentity(kf.errorCovPost, Scalar::all(1));

	// initialize state vector with bounding box in [cx,cy,s,r] style
	kf.statePost.at<float>(0, 0) = stateMat.x + stateMat.width / 2;
	kf.statePost.at<float>(1, 0) = stateMat.y + stateMat.height / 2;
	kf.statePost.at<float>(2, 0) = stateMat.area();
	kf.statePost.at<float>(3, 0) = stateMat.width / stateMat.height;
}

// initialize Kalman filter with known process matrix
void KalmanTracker::init_kf(StateType stateMat, Mat_<float> process_mat, Mat_<float> obser_mat)
{

	float t = float(1) / float(30);//frame rate
	const int stateNum = 7;
	const int measureNum = 4;
	kf = KalmanFilter(stateNum, measureNum, 0);

	measurement = Mat::zeros(measureNum, 1, CV_32F);

	//kf.transitionMatrix = *(Mat_<float>(stateNum, stateNum) <<
	kf.transitionMatrix = (Mat_<float>(stateNum, stateNum) <<
		1, 0, 0, 0, t, 0, 0,
		0, 1, 0, 0, 0, t, 0,
		0, 0, 1, 0, 0, 0, t,
		0, 0, 0, 1, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0,
		0, 0, 0, 0, 0, 1, 0,
		0, 0, 0, 0, 0, 0, 1);

	setIdentity(kf.measurementMatrix);
	if ((process_mat.rows == stateNum) && (process_mat.cols == stateNum)) {
		kf.processNoiseCov = process_mat;
	}
	else {
		cout << "Using defalt process mat because invald size was passed into init fuction" << endl;
		setIdentity(kf.processNoiseCov, Scalar::all(1e-2));//movment of swimmers
	}
	
	if ((obser_mat.rows == measureNum) && (obser_mat.cols == measureNum)) {
		cout << obser_mat << endl;
		kf.measurementNoiseCov = obser_mat;
	}
	else {
		cout << "Using defalt observation mat because invald size was passed into init fuction" << endl;
		setIdentity(kf.measurementNoiseCov, Scalar::all(1e-1));//error of detection
	}

	setIdentity(kf.errorCovPost, Scalar::all(1));

	// initialize state vector with bounding box in [cx,cy,s,r] style
	kf.statePost.at<float>(0, 0) = stateMat.x + stateMat.width / 2;
	kf.statePost.at<float>(1, 0) = stateMat.y + stateMat.height / 2;
	kf.statePost.at<float>(2, 0) = stateMat.area();
	kf.statePost.at<float>(3, 0) = stateMat.width / stateMat.height;
}

// Predict the estimated bounding box.
StateType KalmanTracker::predict()
{
	// predict
	Mat p = kf.predict();
	m_age += 1;

	if (m_time_since_update > 0)
		m_hit_streak = 0;
	m_time_since_update += 1;

	StateType predictBox = get_rect_xysr(p.at<float>(0, 0), p.at<float>(1, 0), p.at<float>(2, 0), p.at<float>(3, 0));

	m_history.push_back(predictBox);
	return m_history.back();
}


// Update the state vector with observed bounding box.
void KalmanTracker::update(StateType stateMat)
{
	m_time_since_update = 0;
	m_history.clear();
	m_hits += 1;
	m_hit_streak += 1;

	// measurement
	measurement.at<float>(0, 0) = stateMat.x + stateMat.width / 2;
	measurement.at<float>(1, 0) = stateMat.y + stateMat.height / 2;
	measurement.at<float>(2, 0) = stateMat.area();
	measurement.at<float>(3, 0) = stateMat.width / stateMat.height;

	// update
	kf.correct(measurement);
}


// Return the current state vector
StateType KalmanTracker::get_state()
{
	Mat s = kf.statePost;
	return get_rect_xysr(s.at<float>(0, 0), s.at<float>(1, 0), s.at<float>(2, 0), s.at<float>(3, 0));
}


// Convert bounding box from [cx,cy,s,r] to [x,y,w,h] style.
StateType KalmanTracker::get_rect_xysr(float cx, float cy, float s, float r)
{
	float w = sqrt(s * r);
	float h = s / w;
	float x = (cx - w / 2);
	float y = (cy - h / 2);

	if (x < 0 && cx > 0)
		x = 0;
	if (y < 0 && cy > 0)
		y = 0;

	return StateType(x, y, w, h);
}



