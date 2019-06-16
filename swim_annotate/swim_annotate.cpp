//Tim Woinoski
//2019-05-30
// swim_annotate.cpp : This file contains the 'main' function. Program execution begins and ends there.
//This application labels swimming videos quicker

#include <iostream>
#include <fstream>//file manipulation
#include <vector>
#include <errno.h>

#include <opencv2/core/utility.hpp>//comandline parcer
#include <opencv2/core/types.hpp> //for the rect object
#include <opencv2/videoio/videoio.hpp> //displaying video
#include <opencv2/opencv.hpp> //displaying video
#include <opencv2/highgui.hpp>
#include <opencv2/core/utility.hpp>//comandline parcer
#include <opencv2/video/tracking.hpp>
#include <opencv2/core/ocl.hpp>


using namespace std;
using namespace cv;

struct swim_data
{
  Rect swimmer_box;
  int box_class = -1;
  int lane_num = -1;
};

//Global Vars
//mouse controle vars
bool selectObject = false;
Point origin;
Rect mouse_box;
int vid_hight;
int vid_width;
selectorParams.drawFromCenter = false;//do not start slection from center

Rect current_box;
int skip_size = 3; //how many frames to skip every new frame
Mat frame;

VideoCapture an_video = NULL;
int current_frame = 0;
int number_of_frames = -1;

int current_swimmer = -1;//lane number of swimmer
int current_class = 1;//current class of the annotaion
string video_file = "";

//Annotation data
swim_data** all_data = nullptr;
//End vars ************************************

string hot_keys = "\n\n\n***************************************\n*  Welcome to the swiming annotator!  *\n***************************************\n\n\n"
    "\tESC - quit the program\n"
    "\t1 - Go back to last frame\n"
    "\t2 - Go to next frame\n"
    "\t3 - Move to any arbitrary frame\n"
    "\t4 - Change annotation class\n"
    "\t5 - Change lane number\n"
    "\t6 - Stop annotating\n\n";

//define user interface
static void help() {
  cout << "\nThis program is used to annotate swimming footage\n"
    "Call:\n"
    "./ffilldemo [image_name -- Default: fruits.jpg]\n" << endl;

  cout << hot_keys;
}

//controle how the mouse affects the display
static void onMouse(int event, int x, int y, int, void*) {

  switch (event)
  {
    // update the selected bounding box
  case EVENT_MOUSEMOVE:
    if (selectorParams.isDrawing)
    {
      if (selectorParams.drawFromCenter)
      {
        selectorParams.box.width = 2 * (x - selectorParams.center.x);
        selectorParams.box.height = 2 * (y - selectorParams.center.y);
        selectorParams.box.x = std::min(
          std::max(selectorParams.center.x - selectorParams.box.width / 2.0, 0.), (double)imageSize.width);
        selectorParams.box.y = std::min(
          std::max(selectorParams.center.y - selectorParams.box.height / 2.0, 0.), (double)imageSize.height);
      }
      else
      {
        selectorParams.box.width = std::max(
          std::min(x - selectorParams.box.x, (double)imageSize.width - selectorParams.box.x), -selectorParams.box.x);
        selectorParams.box.height = std::max(
          std::min(y - selectorParams.box.y, (double)imageSize.height - selectorParams.box.y), -selectorParams.box.y);
      }
    }
    break;

    // start to select the bounding box
  case EVENT_LBUTTONDOWN:
    selectorParams.isDrawing = true;
    selectorParams.box = Rect2d(x, y, 0, 0);
    selectorParams.center = Point2f((float)x, (float)y);
    break;

    // cleaning up the selected bounding box
  case EVENT_LBUTTONUP:
    selectorParams.isDrawing = false;
    if (selectorParams.box.width < 0)
    {
      selectorParams.box.x += selectorParams.box.width;
      selectorParams.box.width *= -1;
    }
    if (selectorParams.box.height < 0)
    {
      selectorParams.box.y += selectorParams.box.height;
      selectorParams.box.height *= -1;
    }
    break;
  }
}

const char* keys =
{
    "{help h | | show help message}{@camera_number| 0 | camera number}"
};

//select the lane number of the swimmer you are annotating
//must be called first 
//Finished
void select_lane_number();

//loads the video in video file into the video object 
//sets the number_of_frames, current_frame to zero, and opens the VideoCapture object
//must be called second
//
bool load_video(string video_file);

//Create ROI
//finished
bool create_ROI_in_pool();

//returns a pointer to the swim data produced
swim_data* get_swim_data(int frame_no, int lane_no);

//displays the current frame with or without annotation
//works
void display_current_frame();

//move to next frame 
//save current annotation in all_data
//predict the box in the next frame
void predict_next_frame();

//moves to the next frame
//finished
void next_frame();

//move to the last frame
//finished
void last_frame();

//Go to the frame num specified
//finished
void go_to_frame();

//exit supper annotator
bool quit_and_save_data();

//prints the annotation options
//Is used in display current frame automaticly
//finished
bool annotation_options();

//saves the current_box rect object in the class to the all_data and the text file 
bool save_annotation();

//load completed work onto the textfile
bool update_text_file();

//changes the current class lable for the box created
void change_class();

//free the all_data memory
void free_all_swim_memory();

int main(int argc, char* argv[])
{
  CommandLineParser parser(argc, argv, keys);

  if (parser.has("help"))
  {
    help();
    return 0;
  }

  video_file = argv[1];

  an_video.open(video_file);
  if (!an_video.isOpened())
  {
    help();
    cout << "***Could not initialize video...***\n";
    cout << "Current parameter's value: \n";
    parser.printMessage();
    return -1;
  }

  load_video(video_file);

  cout << hot_keys;
  namedWindow("Annotator", 0);
  setMouseCallback("Annotator", onMouse, 0);

  
  for (;;)
  {

    display_current_frame();
    imshow("Annotator", frame);

    char c = (char)waitKey(10);
    if (c == 27)//ESC key
      break;
    switch (c)
    {
    case '1': // Go back to last frame
      last_frame();
      break;
    case '2': // Go to next frame
      next_frame();
      break;
    case '3': // Move to any arbitrary frame
      go_to_frame();
      break;
    case '4': //Change annotation class
      change_class();
      update_text_file();
      break;
    case '5': //Change lane number
      select_lane_number();
      update_text_file();
      break;
    case '6': //Stop annotating
      //free_all_swim_memory()
      break;
    default:
      ;
    }
  }

  return 0;
}


bool load_video(string video_filename)
{
  Mat frame;
  an_video.open(video_filename);

  if (an_video.isOpened()) {

    ifstream get_from_header;
    ofstream write_to_header;
    string header_filename = video_filename;
    double FPS_vid = an_video.get(CAP_PROP_FPS);
    int n = an_video.get(CAP_PROP_FRAME_HEIGHT);//hight
    int m = an_video.get(CAP_PROP_FRAME_WIDTH);//width

    //for the mouse slection
    vid_hight = n;
    vid_width = m;

    //set class member fields
    number_of_frames = int(an_video.get(CAP_PROP_FRAME_COUNT));
    video_file = video_filename;
    current_frame = 0;

    namedWindow("Annotating Window", WINDOW_AUTOSIZE);

    //Check if header already has been created
    //A data file will be mapped to its name, this data file will initially contain header with, 
    //video frame rate, video resolution, the frame skip size, and total number of frames in video. 

    //Change file name to end it .txt
    size_t pos = header_filename.find_first_of('.', 0);//no need to be carfull as the file was already opened
    header_filename.erase(pos + 1, 3);
    header_filename.append("txt");

    get_from_header.open(header_filename);
    if (get_from_header.is_open()) {
      char line[500];
      string find;
      string num;
      string::size_type sz;
      int ii = 0;//frame number
      int jj = 0;//lane number
      int kk = 0;//number type
      int num_possible_data_lines = number_of_frames / skip_size;

      get_from_header.getline(line, 500);
      if (line[0] != '#') {
        cout << "header missing initial # on frist line, file open failed!" << endl;
        return false;
      }
      get_from_header.getline(line, 500);//get FPS
      find = line;
      pos = find.find_first_of(' ', 0);
      num = find.substr(pos + 1, (find.size() - pos - 1));
      if (abs(stod(num) - FPS_vid) > .01) { //to remove any rounding errors from the text file
        cout << "FPS dont match file, open failed" << endl;
        return false;
      }
      get_from_header.getline(line, 500);//get Hight and width
      find = line;
      pos = find.find_first_of(' ', 0);
      num = find.substr(pos + 1, (find.size() - pos - 1));
      if (stoi(num, &sz) != n) {
        cout << stoi(num, &sz) << endl;
        cout << "hight dose not match file, open failed" << endl;
        return false;
      }
      else if (stoi(num.substr(sz)) != m) {
        cout << stoi(num.substr(sz)) << endl;
        cout << "width dose not match file, open failed" << endl;
        return false;
      }
      get_from_header.getline(line, 500);//get #of frames 
      find = line;
      pos = find.find_first_of(' ', 0);
      num = find.substr(pos + 1, (find.size() - pos - 1));
      if (stoi(num) != number_of_frames) {
        cout << stoi(num) << endl;
        cout << "number of frames dose not match file, open failed" << endl;
        return false;
      }

      //load data...
      get_from_header.getline(line, 500);//read in "#data"

      //init data holder
      all_data = new swim_data * [num_possible_data_lines]; //memory allocated, needs to be deleted (on line... )
      for (ii = 0; ii < num_possible_data_lines; ii++) {
        all_data[ii] = new swim_data[10];
        for (jj = 0; jj < 10; jj++) {//init to -1
          all_data[ii][jj].box_class = -1;
          all_data[ii][jj].swimmer_box.x = -1;
          all_data[ii][jj].swimmer_box.y = -1;
          all_data[ii][jj].swimmer_box.height = -1;
          all_data[ii][jj].swimmer_box.width = -1;
          all_data[ii][jj].lane_num = -1;
        }
      }

      size_t pos_lane = 0;
      size_t pos_num = 0;
      size_t pos_num_check = 0;
      size_t pos_num_end = 0;
      int num_val = 0;

      for (ii = 0; ii < num_possible_data_lines; ii++) {//get #of frames 
        //frame loop
        if (get_from_header.getline(line, 500)) {//should hold all data at max
          find = line;
          pos_lane = 0;
          //start looking threw the frame
          for (jj = 0; jj < 10; jj++) {
            //lane loop
            pos_lane = find.find_first_of('{', pos_lane);//look for the next lane
            if (pos_lane != string::npos) {//if there is a lane
              //start looking through numbers for that lane
              pos_num_check = find.find_first_of('}', pos_lane);//check if there are 6 numbers
              if (pos_num_check == string::npos) {//if there is not an end to the other braket
                cout << "Braket error in frame " << ii << endl;
                return false;
              }
              pos_num = pos_lane;//pos_num holds the position of the current number in a lane
              pos_num_end = pos_lane;
              for (kk = 0; kk < 6; kk++) {
                //number loop
                pos_num_end = find.find_first_of(",", pos_num);//find end of number
                if ((pos_num_end > pos_num_check) && (kk < 5)) {//check if there are 6 numbers
                  cout << "Error in frame " << jj << " lane index " << pos_lane << ". not enough numbers." << endl;
                  return false;
                }
                else if (pos_num_end < pos_num_check && (kk == 5)) {//if in here then more than 6 number are in the lane 
                  cout << "Error in frame, too many numbers." << jj << endl;
                  return false;
                }
                else if (pos_num_end > pos_num_check) {//get the final number
                  pos_num_end = pos_num_check;
                }

                num_val = stoi(find.substr(pos_num + 1, pos_num_end - pos_num));//get the found number
                //cout << num_val << endl;
                //this is where the lane numbers are inserted into the sturcture
                if (kk == 0) all_data[ii][jj].swimmer_box.x = num_val;
                if (kk == 1) all_data[ii][jj].swimmer_box.y = num_val;
                if (kk == 2) all_data[ii][jj].swimmer_box.height = num_val;
                if (kk == 3) all_data[ii][jj].swimmer_box.width = num_val;
                if (kk == 4) all_data[ii][jj].box_class = num_val;
                if (kk == 5) all_data[ii][jj].lane_num = num_val;

                pos_num = pos_num_end + 1;//reset the pos_num to look for the next number
              }
            }
            else {//if there are no more lanes go to the next line
              break;
            }
            pos_lane = pos_num_check;
            //end of lane loop
          }
        }
        else {//if the video data is incomplete, stop looking for data that does not exist
          break;
        }
        //end of frame loop
      }
      get_from_header.close();
    }
    else {//Create new header 
      write_to_header.open(header_filename);
      write_to_header << "#Header, frames per second, video resolution(hight width), total number of frames in video." << endl;
      write_to_header << "FPS: " << FPS_vid << endl;
      write_to_header << "RES: " << n << " " << m << endl;
      write_to_header << "TNF: " << number_of_frames << endl;
      write_to_header << "#Data x, y, h, w, c, and l always in this order, for each lane. Sorted in order." << endl;
      write_to_header.close();
    }
    return true;
  }
  else {
    cout << "Supper annotator could not open video file\n";
    return false;
  }
}


//to show box change class object flag
//primary loop control for supper_annotator class
void display_current_frame()
{
  bool display_box = false;
  swim_data* lane;

  //Get the frame
  an_video.set(CAP_PROP_POS_FRAMES, current_frame);//CV_CAP_PROP_POS_FRAMES
  an_video >> frame;

  //Update current box
  lane = get_swim_data(int(current_frame / skip_size), current_swimmer);//could return an empty lane!!
  if (lane != nullptr) {
    if (lane->lane_num == -1) {//if empty lane 
      display_box = false; //dont display box
    }
    else {
      display_box = true;//display box
      current_box = lane->swimmer_box;
    }
  }

  if (display_box) {
    rectangle(frame, current_box, Scalar(255, 0, 0), 2, 1);// will create a box in the image frame
  }

  return;//exiting annotator
}

bool create_ROI_in_pool()
{
  Mat frame;
  an_video.set(CAP_PROP_POS_FRAMES, current_frame);//CV_CAP_PROP_POS_FRAMES
  an_video >> frame;
  current_box = selectROI("Annotating Window", frame, false, false);
  if (current_box.empty()) return false;
  save_annotation();

  return true;
}

//saves the current_box rect object in the class to the all_data and the text file 
bool save_annotation()
{
  swim_data* lane_data;
  lane_data = get_swim_data(int(current_frame / skip_size), current_swimmer);//checks for out of bounds errors

  if (lane_data) {
    //replace the data
    lane_data->box_class = current_class;
    lane_data->lane_num = current_swimmer;
    lane_data->swimmer_box = current_box;
    return true;
  }
  else {//if there was out of bound input
    return false;
  }

}

//loads data from all data into the current text file
bool update_text_file()
{
  ofstream update_header;
  string header_filename = video_file;
  char line[500];
  string header_data;
  double FPS_vid = an_video.get(CAP_PROP_FPS);
  int n = an_video.get(CAP_PROP_FRAME_HEIGHT);//hight
  int m = an_video.get(CAP_PROP_FRAME_WIDTH);//width

  update_header.open("_app.txt");//temp file

  //make a new text file called _app.txt and then when created change the file names
  if (update_header.is_open()) {//if file opened start updating 
    int ii = 0;//frame number
    int jj = 0;//lane number
    int kk = 0;//lane contence
    int num_possible_data_lines = number_of_frames / skip_size;//need to round down, need due to skipping frames
    swim_data* a_lane;
    update_header << "#Header, frames per second, video resolution(hight width), total number of frames in video." << endl;
    update_header << "FPS: " << FPS_vid << endl;
    update_header << "RES: " << n << " " << m << endl;
    update_header << "TNF: " << number_of_frames << endl;
    update_header << "#Data x, y, h, w, c, and l always in this order, for each lane. Sorted in order." << endl;

    for (ii = 0; ii < num_possible_data_lines; ii++) {//look at each frame 
      for (jj = 0; jj < 10; jj++) {//look at each lane
        a_lane = get_swim_data(ii, jj);
        if ((a_lane != nullptr) && (a_lane->lane_num == jj)) { //add the lane to the text file
          update_header << "{";
          for (kk = 0; kk < 6; kk++) {
            if (kk == 0) { update_header << a_lane->swimmer_box.x << ", "; }
            if (kk == 1) { update_header << a_lane->swimmer_box.y << ", "; }
            if (kk == 2) { update_header << a_lane->swimmer_box.height << ", "; }
            if (kk == 3) { update_header << a_lane->swimmer_box.width << ", "; }
            if (kk == 4) { update_header << a_lane->box_class << ", "; }
            if (kk == 5) { update_header << a_lane->lane_num << "}"; }
          }
        }
        //if last frame dont end line else end line
        if ((ii < (num_possible_data_lines - 1)) && (jj == 9)) update_header << endl;
      }
    }
    update_header.close();

    //rename the files
    //Change file name to end it .txt
    size_t pos = header_filename.find_first_of('.', 0);
    header_filename.erase(pos, 4);
    header_filename.append(".txt");
    remove("back_up.txt");//remove any file if exists
    if (rename(header_filename.c_str(), "back_up.txt") == 0) {//create a backup file
      if (rename("_app.txt", header_filename.c_str()) == 0) {
        return true;
      }
      else {
        perror("Error renaming file");
        return false;
      }
    }
    else {
      perror("Error renaming file");
      return false;
    }
  }
  else {//if file does not open
    return false;
  }
}

//changes the current class lable for the box created
void change_class()
{
  char class_num;
  int num = -1;
  bool done;

  //select lane number
  do {
    cout << "What class are we lableing? Options are..." << endl;
    cout << "on_block (1), diving (2), swimming (3), underwater (4), turning (5), finishing (6)" << endl;
    cout << "Class: ";
    cin >> class_num;

    if (!isdigit(class_num)) {
      num = -1;
    }
    else {
      num = int(class_num) - 48;//convert to int
    }

    if ((num > 6) || (num < 1)) {
      cout << "\nAn invalid lane number was selected" << endl;
      done = false;
    }
    else {
      current_class = num;
      done = true;
    }
  } while (!done);

  return;
}

//select the lane number of the swimmer you are annotating
//return prevouse lane number
//return -1 on error
void select_lane_number() {

  char lane_num;
  int num = -1;
  bool done;

  //select lane number
  do {
    cout << "What lane number are we working on? ";
    cin >> lane_num;

    if (!isdigit(lane_num)) {
      num = -1;
    }
    else {
      num = int(lane_num) - 48;//convert to int
    }

    if ((num > 9) || (num < 0)) {
      cout << "\nAn invalid lane number was selected" << endl;
      done = false;
    }
    else {
      current_swimmer = num;
      done = true;
    }
  } while (!done);

  return;
}


//Use the camshift() algorithum to find swimmer in next frame
//Every new frame, update the referance frame
void predict_next_frame()
{
  Mat old_frame, frame;

  //get the frist frame to get the histogram of the ROI
  an_video.set(CAP_PROP_POS_FRAMES, current_frame);//CV_CAP_PROP_POS_FRAMES
  an_video >> frame;
  old_frame = frame.clone();

  //get the next frame
  current_frame += skip_size;
  if (current_frame > (number_of_frames - 1)) {
    current_frame -= skip_size;
    cout << "This is the last frame in this video" << endl;
    return;
  }

  an_video.set(CAP_PROP_POS_FRAMES, current_frame);
  an_video >> frame;


  return;
}

bool quit_and_save_data()
{
  //must destroy windows!!!
  char answer = 'n';
  bool keep_asking = true;

  do {
    cout << "Are you sure you are done? (y/n)" << endl;

    cin.ignore(1000, '\n');//remove extra stuff 

    cin >> answer;
    if (answer == 'n') {
      keep_asking = false;
      return false;
    }
    else if (answer == 'y') {
      keep_asking = false;

      //save data
      if (!update_text_file()) {
        cout << "could not save work!! look at text file for problem or data will be lost" << endl;
        return false;
      }
      return true;
    }
    else {
      keep_asking = true;
    }
  } while (keep_asking);

  return false;
}


//Annotate next frame 
//We are going to skip every other frame
void next_frame()
{
  current_frame += skip_size;

  if (current_frame > (number_of_frames - 1)) {
    current_frame -= skip_size;
    cout << "This is the last frame in this video" << endl;
    return;
  }

  return;
}

void last_frame()
{
  current_frame -= skip_size;

  if (current_frame < 0) {
    current_frame += skip_size;
    cout << "Cant got farther back, this is the frist frame in this video" << endl;
    return;
  }

  return;
}

//go to a spesified frame 
void go_to_frame()
{
  int frame_num = -1;
  string answer;
  string nums("1234567890");
  int right_frame = 0;

  cout << "Input the frame number: ";

  cin.ignore(1000, '\n');
  cin >> answer;
  if (answer.find_first_not_of(nums) != string::npos) {
    cout << "Invalid input for frame number" << endl;
    return;
  }

  frame_num = stoi(answer);

  //Move the input frame number so that it is a multiple of the frame skip size
  right_frame = frame_num % skip_size;
  frame_num -= right_frame;

  if ((frame_num < 0) || (frame_num > (number_of_frames - 1))) {
    cout << "This is not a vaid frame number for this video" << endl;
    return;
  }
  current_frame = frame_num;
  return;
}

//return a pointer to the data in all_data
swim_data* get_swim_data(int frame_no, int lane_no)
{
  int most_possible_data = number_of_frames / skip_size;
  if ((frame_no > most_possible_data - 1) || (frame_no < 0)) {// out of range frames
    cout << "Invalid frame number was requested from data" << endl;
    return nullptr;//empty_lane;
  }
  if ((lane_no > 9) || (lane_no < 0)) {//out of range lanes
    cout << "Invalid lane number was requested from data" << endl;
    return nullptr;//empty_lane;
  }

  //A reminder that frames are skipped every skip_size frames 
  swim_data* frame = all_data[frame_no];
  int ii = 0;
  for (ii = 0; ii < 10; ii++) {
    if (frame[ii].lane_num == lane_no) {//check if lane number exists
      return &frame[ii];//return the location of that lane
    }
    if (frame[ii].lane_num == -1) {//If no lane number exists then return pointer to first empty lane
      return &frame[ii];
    }
  }
  return nullptr;//empty_lane;
}









//potentialy not needed------------------------------------------

//free the all_data memory
void free_all_swim_memory()
{
  int ii = 0;
  int num_possible_data_lines = number_of_frames / skip_size;

  for (ii = 0; ii < num_possible_data_lines; ii++) {
    delete[] all_data[ii];
  }
  delete[] all_data;
}

//gets the next acction to be exicuted on the video data
bool annotation_options()
{
  //I want unbuffered input to speed up annotations however this is difficult to do
  // there is an option to maybe use the OpenCV API setMouseCallback however I just
  // want somthing to work for now
  char answer = 'z';
  int ans = -1;

  cout << "\nOptions for annotation editing.\n\n";
  cout << "Change lane number of annotations, press (1)\n";
  cout << "Predict next frame and save current frame, press (2)\n";//change to right mouse button click
  cout << "Create ROI, press (3)\n";//change to left mouse button click
  cout << "Go back to last frame, press (4)\n";
  cout << "Go to next frame, press (5)\n";
  cout << "Move to any arbitrary frame, press (6)\n";
  cout << "Change annotation class, press (7)\n";
  cout << "Stop annotating video, press (8)\n";
  cout << "\nAnnotate F:" << current_frame << " L:" << current_swimmer << " c:" << current_class << "> ";


  cin.ignore(1000, '\n');
  cin >> answer;

  if (!isdigit(answer)) {
    cout << "An unrecognised value was input\n";
    return false;
  }
  else {
    ans = int(answer) - 48;//convert to int
  }

  switch (ans) {
  case 1://Change lane number of annotations
    select_lane_number();
    break;
  case 2://Predict next frame and save current frame
    predict_next_frame();
    break;
  case 3://Create ROI
    if (create_ROI_in_pool()) {
      cout << "ROI saved!" << endl;
    }
    else {
      cout << "ROI failed to save" << endl;
    }
    break;
  case 4://Go back to last frame
    last_frame();
    break;
  case 5://Go to next frame
    next_frame();
    break;
  case 6://Move to any arbitrary frame//Change annotation class
    go_to_frame();
    break;
  case 7://Change annotation class
    change_class();
    break;
  case 8://Stop annotating video
    if (quit_and_save_data()) {
      return false;//exit the annotator
    }
    break;
  default:
    cout << "An unrecognised value was input\n";
    break;
  }
  return true;
}

