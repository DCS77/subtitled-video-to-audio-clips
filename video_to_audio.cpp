/*
//	Author: Daniel Smit
	Date: 21 Feb 2017
//	Title: Subtitled video to audio clips
//	Description: Extracts audio from a video file. Reads subtitle files to select snippet times of each conversation. Creates a txt file with text contained in each snippet of audio.
*/

#include <iostream>

using namespace std;

int checkVideoFormat(){
	return 1;
}

int convertVideoToAudio(){
	return 1;
}

int main(int argc, char* argv[]){

	int res;
	if(res = checkVideoFormat == -1){
		cout << "Error checking video format" << endl;
		return -1;
	} else if(res == 0){
		cout << "Video format not supported" << endl;
	} else {
		cout << "Video format supported" << endl;
	}

	return 0;
}

