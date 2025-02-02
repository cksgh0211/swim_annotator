#include "stroke_annotate.h"

stroke_annotate::stroke_annotate() {

}

void stroke_annotate::graph_example()
{

  char end = '0';
  char responce = '0';
  int cntr_end = 0;
  double time_slice = double(1) / double(30);
  double vid_time = 22.5;

  //stroke_annotate stroke_work;
  graph_drawing tester("test window", vid_time, time_slice);

  tester.start_graph_drawer();

  while (end == '0') {
    responce = waitKey(25);
    if (responce == 't') {
      //tester.find_not_swimming();
      tester.input_new_frame(true, true, fly);
    }
    else if (responce == 'b') {
      tester.undo_work(3);
    }
    else if (responce == 'p') {
      toggel_pause();
    }
    else if (responce == 27) {
      end++;//break
    }

    if (tester.input_new_frame(false, true, fly)) {
      end++;
    }
    tester.draw_graph();
    cntr_end++;
  }

  tester.kill_graph_drawer();
}


void stroke_annotate::file_example()
{
  string test_file_name = "2_str.txt";

  SA_file_mannager tester(test_file_name);

  if (!tester.read_file()) {
    cout << "An error occured, could not read stroke annotation file in stroke_annoate.cpp" << endl;
  }

  if (!tester.save_file()) {
    cout << "An error occured, could not save stroke annotation file in stroke_annoate.cpp" << endl;
  }

}


//loads and opens all objects required for stroke annotation 
//Display all windows
bool stroke_annotate::start_stroke_counting(string intput_video_file)
{

  video_file = intput_video_file;
  video_window_name = video_file + " video player";
  double FPS = 1;
  int frame_count = 0;
  int hight  = 0;
  int width = 0;
  Mat frame;

  string vf_text = video_file;
  char resp = '0';


  //Video stuff
  cap.open(video_file);
  if (!cap.isOpened()) {
    cout << "Could not open video file for stroke counting" << endl;
    return false;
  }
  namedWindow(video_window_name, WINDOW_NORMAL);
  moveWindow(video_window_name, 700, 25);
  resizeWindow(video_window_name, Size(500, 500));
  FPS = cap.get(CAP_PROP_FPS); frame_count = cap.get(CAP_PROP_FRAME_COUNT);
  hight = cap.get(CAP_PROP_FRAME_HEIGHT); width = cap.get(CAP_PROP_FRAME_WIDTH);
  if (!cap.read(frame)) {
    cout << "Could not peek" << endl;
  }
  if (frame.empty()) {
    cout << "Error, Could not read first frame of video" << endl;
    return false;
  }
  imshow(video_window_name, frame);
  waitKey(1);

  //File stuff
  vf_text.replace(vf_text.end() - 4, vf_text.end(), "_str.txt");
  //Get video paramiters
  man_file.input_info(vf_text, FPS, frame_count, hight, width);
  //look for files related to video file name
  if (!man_file.read_file()) {
    cout << "An error occured, when reading a stroke annotation file" << endl;
    destroyWindow(video_window_name);
    return false;
  }

  //Grapher stuff
  grapher.change_window_name(intput_video_file + " Stroke Visualization");
  grapher.input_data(man_file.return_data());
  grapher.input_varibales(double(frame_count)/FPS, double(1)/FPS);
  grapher.start_graph_drawer();

  print_vid_dialog();

  return true;
}


//Show options for vid once
//wait for user to choose one
//Also display in video options
void stroke_annotate::print_vid_dialog()
{
  char resp = '0';

  while (1) {

    cout << "\nAnnotation options:" << endl;
    cout << "To start annotating (edit mode), press 1" << endl;
    cout << "To view any annotations and/or video (view mode), press 2" << endl;
    cout << "To quit, press 3" << endl << ">>";

    cap.set(CAP_PROP_POS_FRAMES, 0);

    cin >> resp;
    if (cin.fail()) {
      cin.clear();
      cin.ignore(numeric_limits<streamsize>::max(), '\n');
      continue;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (resp == '1') {
      //start annotating 
      if (grapher.remove_all_data()) annotate_video(true);
    }
    else if (resp == '2') {
      //view annotations
      annotate_video(false);
    }
    else if (resp == '3') {
      //quit
      quit_stroke_annotator();
      break;
    }
  }
  return;
}


//Start annotating video (edit mode)
//View any annotations and/or watch video (view mode)
//Fist ask if swimming is swimming or not, set swimmer_is_swimming accordingly
void stroke_annotate::annotate_video(bool is_edit_mode)
{
  Mat frame;
  char resp = '0';
  int cntr_end = 0, cntr_start = 0;
  double frame_rate = cap.get(CAP_PROP_FPS);
  vector<stroke_data> temp;
  stroke_data element;

  while (waitKey(1) > 0);//clear this windows iostream buffer

  //Find out what swimmer is doing
  if (is_edit_mode) {
    if (get_swimmer_stait()) {
      get_swimmer_stroke();
    }
  }

  print_video_options(is_edit_mode);

  cout << "Video paused, press any key to start..." << endl;
  while (waitKey(0) < 0);

  //Main loop for annotation
  while (1) {

    resp = waitKey(int(frame_rate * video_speed));//video speed controle

    if (resp == 'p') {
      //cout << "video paused" << endl;
      toggel_pause();
    }
    else if (resp == 'b') {
      skip_back(num_skip_back, is_edit_mode);
    }
    else if (resp == 'd') {//speed down
      change_speed(false);
    }
    else if (resp == 'u') {//speed up
      change_speed(true);
    }
    else if(resp == 'm') {//mark swimming
      if (is_edit_mode) {
        toggel_swimming();
      }
    }
    else if (resp == 27) {
      break;
    }

    //Add data to grapher
    if (is_edit_mode) {
      if (resp == 't') {
        grapher.input_new_frame(true, swimmer_is_swimming, swimmer_stroke);
      }
      else {
        grapher.input_new_frame(false, swimmer_is_swimming, swimmer_stroke);
      }
    }

    if (is_edit_mode) {
      grapher.draw_graph();
    }
    else {
      grapher.draw_graph(double(cap.get(CAP_PROP_POS_MSEC)) / double(1000));
    }
    
    //display next window frames;
    cap >> frame;
    if (frame.empty()) {
      cout << "Finished video" << endl;
      return;
    }
    imshow(video_window_name, frame);
  }

  return;
}


//prints options when editing or viewing stroke annotations
void stroke_annotate::print_video_options(bool is_edit_mode)
{
  cout << "In video options:" << endl << endl;
  
  //t option (mark stroke occuring)
  if(is_edit_mode) cout << "To mark a stroke, press t" << endl;

  //p option (pause)
  cout << "To toggle pause, press p" << endl;

  //b option (move back n strokes)
  if (!is_edit_mode) {
    cout << "To go back " << num_skip_back << " strokes without changing annotations, press b" << endl;
  }
  else {
    cout << "To go back " << num_skip_back << " strokes to change annotations, press b" << endl;
  }

  //m option (mark when swimmer is swimming and when they are not)
  if (is_edit_mode) cout << "To toggal swimmer swimming, press m" << endl;

  //d option (slow Down video playback)
  cout << "To slow down video, press d" << endl;
  
  //u option (speed Up video playback)
  cout << "To speed up video, press u" << endl;
  
  //quit option 
  cout << "To quit, press esc" << endl << endl;
}


//asks user to input if swimmer is swimming or not
bool stroke_annotate::get_swimmer_stait()
{
  char resp = '0';

  while (1) {
    cout << "What is the swimmer doing right now? (swimming (s) or not (n))" << endl;

    cin >> resp;
    if (cin.fail()) {
      cin.clear();
      continue;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (resp == 's') {
      swimmer_is_swimming = true;
      return true;
    }
    else if (resp == 'n') {
      swimmer_is_swimming = false;
      return false;
    }
  }
}


//ask the user to get the stroke of the swimmer
void stroke_annotate::get_swimmer_stroke()
{
  while (1) {
    cout << "What stroke is the swimmer swimming? (fly, back, brest, freestyle or mixed)" << endl;

    cin >> swimmer_stroke;
    if (cin.fail()) {
      cin.clear();
    }
    else {
      break;
    }
  }
}


//double or half the speed every time (to a limit)
void stroke_annotate::change_speed(bool speed_up)
{
  double conv = log2(video_speed) + 2;

  if (speed_up) {
    if(conv > max_speed) {
      video_speed /= 2;
    }
    else {
      cout << "Cant speed up video any more!" << endl;
    }
  }
  else {
    if (conv < min_speed) {
      video_speed *= 2;
    }
    else {
      cout << "Cant slow down video any more!" << endl;
    }
  }

  if (video_speed == double(1)) {
    cout << "normal speed" << endl;
  }

  return;
}


//flips the swimmer_is_swimming flag
//ask for the stroke swimmer is swimming
void stroke_annotate::toggel_swimming()
{
  if (swimmer_is_swimming) {
    cout << "Swimmer stoped swimming!" << endl;
    swimmer_is_swimming = !swimmer_is_swimming;
  }
  else {
    cout << "Swimmer started swimming!" << endl;
    get_swimmer_stroke();
    while (waitKey(1) > 0);
    cout << "Press any key to start..." << endl;
    while (waitKey(0) < 0);
    swimmer_is_swimming = !swimmer_is_swimming;
  }
}


//Changes cntr_end and cntr_star aproprately
//Modifies the data in the grapher object
void stroke_annotate::skip_back(int n_strokes, bool in_edit_mode)
{
  Mat frame;
  int num_back = 0, crnt_pos = 0;

  if (in_edit_mode) {
    if (grapher.undo_work(n_strokes)) {
      get_swimmer_stait();
    }
    cap.set(CAP_PROP_POS_FRAMES, grapher.get_current_frame_num());
    cap >> frame;
    imshow(video_window_name, frame);
    while (waitKey(1) > 0);
    cout << "Video paused, press any key to start" << endl;
    while (waitKey(0) < 0);
  }
  else {
    crnt_pos = cap.get(CAP_PROP_POS_FRAMES) - 1;
    num_back = grapher.look_back(n_strokes,crnt_pos);
    cap.set(CAP_PROP_POS_FRAMES, crnt_pos - num_back);
    grapher.draw_graph(double(cap.get(CAP_PROP_POS_MSEC)) / double(1000));
  }

  return;
}


//Specify stroke being prefomed in video (If not already spcifed ask to change)
//Save all work in file
//kill all windows
void stroke_annotate::quit_stroke_annotator()
{ 

  destroyWindow(video_window_name);
  man_file.add_data(grapher.retreive_work());
  grapher.kill_graph_drawer();
  man_file.save_file();
  
}

