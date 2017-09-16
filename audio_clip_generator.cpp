/*
//	Author: Daniel Smit
	Date: 21 Feb 2017
//	Title: Subtitled video to audio clips
//	Description: Extracts audio from a video file. Reads subtitle files to select snippet times of each conversation. Creates a txt file with text contained in each snippet of audio.
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <regex>

using namespace std;

struct srtLine;
int initialChecks(char* inputAudio, char* inputSrt);
int checkAudioFormat();
int getTimestampStart(string line);
int getTimestampEnd(string line);
int matchesTimestampFormat(string line);
int matchesLineFormat(string line);
int matchesBlankLine(string line);

struct srtLine {
	int start;
	int end;
	vector<string> messages;
};

int initialChecks(char* inputAudio, char* inputSrt){

	/* Need to check files for compatibility */

	int res = checkAudioFormat();
	if(res == -1){
		//cout << "Error checking video format" << endl;
		return -1;
	} else if(res == 0){
		//cout << "Video format not supported" << endl;
	} else {
		//cout << "Video format supported" << endl;
	}

	return res;
}

int checkAudioFormat(){
	return 1;
}

int getTimestampStart(string line){

	string hours_str{line.at(0), line.at(1)};
	string minutes_str{line.at(3), line.at(4)};
	string seconds_str{line.at(6), line.at(7)};

	int hours = stoi(hours_str);
	int minutes = stoi(minutes_str);
	int seconds = stoi(seconds_str);

	return floor(hours*60*60 + minutes*60 + seconds);
}

int getTimestampEnd(string line){
	string hours_str{line.at(17), line.at(18)};
	string minutes_str{line.at(20), line.at(21)};
	string seconds_str{line.at(23), line.at(24)};

	int hours = stoi(hours_str);
	int minutes = stoi(minutes_str);
	int seconds = stoi(seconds_str);

	return ceil(hours*60*60 + minutes*60 + seconds);
}

int matchesTimestampFormat(string line){
	/* "00:00:16,391 --> 00:00:19,476" */
	if (regex_match (line, regex("([0-9]{2}(:)){2}[0-9]{2}(,)[0-9]{3}( --> )([0-9]{2}(:)){2}[0-9]{2}(,)[0-9]{3}") )){
		return 1;
	}
	return 0;
}

int matchesLineFormat(string line){
	return 1;
}

int matchesBlankLine(string line){
	if(line.compare("") == 0){
		/* Found blank line */
		return 1;
	}
	return 0;
}


int main(int argc, char* argv[]){

	if(argc < 3){
		//cout << "Not enough input arguments. Usage: ./audio_clip_generator input_audio.mp3 input_subtitles.srt" << endl;
		return -1;
	}

	int res = initialChecks(argv[1], argv[2]);
	if(res < 0){
		return res;
	}

	ifstream srtStream(argv[2]);	

	
	int mode = 0; /* mode: 0 if haven't started yet, or looking for sequence number; 1 if found next sequence number; 2 if found timestamp; 3 if found text subtitle; back to 0 if found blank line */
	int currentIdx = 1; /* currentIdx: Current srt index. Starts at 1 as per specification. Incremented for each new srt */
	int stringIdx = 0;
	string line;
	//srtLine * srtLines = new srtLine[1];
	vector<srtLine> srtLines;

	int startPoint;
	int endPoint;
	//string messages[5]; /* Assume max 5 lines of text per timestamp */
	vector<string> messages;

	while(getline(srtStream, line)){

		//cout << "\n\n\nWhile loop: currentIdx: " << currentIdx << endl;
		//cout << "Line: " << line << endl;

		line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
		line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

		//cout << "TP1: mode: " << mode << endl;
		
		switch(mode){

			/* Check for srt index */
			case 0:
			//cout << "TP3 line: " << line << endl;
			//cout << "CurrentIdx: " << currentIdx << endl;
			if(!matchesBlankLine(line)){
			//if(line.compare(to_string(currentIdx)) == 0){
				/* Found next srt index */
				mode = 1;
				//cout << "Found sequence number. Mode updated." << endl;
			} else {
				continue;
			}
			break;


			/* Check for timestamp */
			case 1:
			//cout << "TP4" << endl;
			//cout << "Timestamp: " << currentIdx << endl;
			if(matchesTimestampFormat(line)){
				/* Found timestamp */
				mode = 2;
				
				/* Get timestamp start and end in seconds */
				startPoint = getTimestampStart(line);
				endPoint = getTimestampEnd(line);

				//cout << "Start: " << startPoint << ", endPoint: " << endPoint << ". Mode updated." << endl;
			}
			break;


			/* Check for subtitle message - Note - there may be multiple lines */
			case 2:
			//cout << "TP5" << endl;
			if(matchesLineFormat(line) && !matchesBlankLine(line)){
				/* Fetch line message */
				messages.push_back(line);
				stringIdx++;
				mode = 3;
			} else if(matchesBlankLine(line) && stringIdx == 0){
				/* If first message is a blank line, nothing important to add, so skip this and go to next sequence number */
				currentIdx++;
				mode = 0;
				continue;
			}
			break;


			/* Check for blank line or extra message */
			case 3:
			//cout << "TP6" << endl;
			if(matchesBlankLine(line)){
				

				srtLine srtLineTemp;
				
				/* Save subtitle line to array of lines */
				srtLineTemp.start = startPoint;
				srtLineTemp.end = endPoint;

				//cout << "messages.size(): " << messages.size() << endl;

				for(int i=0; i<messages.size(); i++){
					srtLineTemp.messages.push_back(messages[i]);
					//cout << "messages[i]: " <<  messages[i] << endl;
				}
				srtLines.push_back(srtLineTemp);

				/* Reset variables for next srt line */
				mode = 0;
				currentIdx++;
				stringIdx = 0;
				messages.clear();
			} else if(matchesLineFormat(line)){
				messages.push_back(line);
				stringIdx++;
			}
		}
		//char c;
		//cin>>c;
	}

	
	cout << "srtLines.size(): " << srtLines.size() << endl;

	for(int i=0; i<srtLines.size(); i++){
		//char c;
		//cin>>c;
		cout << "start: " << srtLines[i].start << endl;
		//cin>>c;
		cout << "end: " <<  srtLines[i].end << endl;
		//cin>>c;
		cout << "message 0: " <<  srtLines[i].messages[0] << endl;
		
	}
	

	//cout << "TP7" << endl;
	
	return 0;
}

