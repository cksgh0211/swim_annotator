#include <string>
#include <iostream>

#include "annotate_engine.h"

using namespace std;

annotate_engine::annotate_engine() {
  //make up a defalt root directory for inizalization 
  file_name = "nothing_yet";
  current_request = waiting_for_request;
}


annotate_engine::annotate_engine(string video_file) {
  //make up a defalt root directory for inizalization 
  file_name = video_file;
  current_request = waiting_for_request;
}


annotate_engine::~annotate_engine() {

}

bool annotate_engine::add_file_name(string file_name) {
  //make up a defalt root directory for inizalization 
  file_name = file_name;
}

//Waits for the users next request 
void annotate_engine::service_next_request() {

  switch (current_request) {
  case start_work:
    //start image annotate class
    if (!run_supper_annotator()) {
      cout << "Could not run annotator with this file" << endl;
    }
    break;
  case finish_up:
    //finish the work done by the annotate class
    break;
  case exit_opt:
    current_request = exit_opt;
    break;
  default:
    break;
  }
}

//Checks if app requested to shut down
bool annotate_engine::is_app_finished() {
  if (current_request == exit_opt) return true;
  return false;
}

//Save all application things
void annotate_engine::kill_app() {
  cout << "Killing app!" << endl;
}


//prints the user options
bool annotate_engine::print_general_lab_options() {
  string answer;
  int ans = 0;

  cout << "\nSelect from the following options:" << endl << endl;
  cout << "Start annotating video, press (1)" << endl;
  cout << "Mark video as finished for storage, press (2)" << endl;
  cout << "Quit labelling and exit, press (3)" << endl;
  cout << "\nlab>> ";

  if (!(cin >> answer)) {
    return false;
  }

  ans = stoi(answer, nullptr, 10);

  switch (ans) {
  case 1: current_request = start_work;
    break;
  case 2: current_request = finish_up;
    break;
  case 3: current_request = exit_opt;
    break;
  default: current_request = err_val;
    cout << "An unrecognised value was input\n";
    return false;
  }

  return true;
}

//run the supper annotator to anotate the video
bool annotate_engine::run_supper_annotator() {

  int lane_num = -1;
  cout << "Loading file for annotation!" << endl;

  if (work.load_video(file_name)) {

    do {
      cout << "What lane number are we working on? ";
      cin >> lane_num;

    } while (!work.select_lane_number(lane_num));

    work.create_ROI_in_pool();
    return true;
  }
  return false;
}
