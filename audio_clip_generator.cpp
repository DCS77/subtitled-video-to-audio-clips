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
#include <cstdlib>

using namespace std;

struct srtLine;
int initialChecks(char* inputAudio, char* inputSrt);
int checkAudioFormat();
int getTimestampStart(string line);
int getTimestampEnd(string line);
int matchesTimestampFormat(string line);
int matchesLineFormat(string line);
int matchesBlankLine(string line);
vector<srtLine> getSrtLines(char* inputSrt);
void clipAudio(char* inputAudio, vector<srtLine> srtLines, char* outputFolder, int secondsOffset, int extraSeconds);
void generateTextToSpeech(vector<srtLine> srtLines, char* outputFolder);
void mergeTTSandAudio(vector<srtLine> srtLines, char* outputFolder);

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

vector<srtLine> getSrtLines(char* inputSrt){
	ifstream srtStream(inputSrt);
	
	int mode = 0; /* mode: 0 if haven't started yet, or looking for sequence number; 1 if found next sequence number; 2 if found timestamp; 3 if found text subtitle; back to 0 if found blank line */
	int currentIdx = 1; /* currentIdx: Current srt index. Starts at 1 as per specification. Incremented for each new srt */
	int stringIdx = 0;
	string line;
	vector<srtLine> srtLines;

	int startPoint;
	int endPoint;
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
			if(!matchesBlankLine(line)){
			//if(line.compare(to_string(currentIdx)) == 0){
				/* Found next srt index */
				mode = 1;
			} else {
				continue;
			}
			break;


			/* Check for timestamp */
			case 1:
			if(matchesTimestampFormat(line)){
				/* Found timestamp */
				mode = 2;
				
				/* Get timestamp start and end in seconds */
				startPoint = getTimestampStart(line);
				endPoint = getTimestampEnd(line);
			}
			break;


			/* Check for subtitle message - Note - there may be multiple lines */
			case 2:
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
			if(matchesBlankLine(line)){

				srtLine srtLineTemp;
				
				/* Save subtitle line to array of lines */
				srtLineTemp.start = startPoint;
				srtLineTemp.end = endPoint;

				for(int i=0; i<messages.size(); i++){
					srtLineTemp.messages.push_back(messages[i]);
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

	return srtLines;
}

void displaySrtLines(vector<srtLine> srtLines){
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
}

void clipAudio(char* inputAudio, vector<srtLine> srtLines, char* outputFolder, int secondsOffset, int extraSeconds){
	
	string inputAudioStr(inputAudio);
	string outputFolderStr(outputFolder);

	string command = "mkdir -p " + outputFolderStr;
	system(command.c_str());

	char commandChr[300];

	int startClip;
	int endClip;
	
	for(int i=0; i<srtLines.size(); i++){
		startClip = srtLines[i].start + secondsOffset - extraSeconds;
		endClip = srtLines[i].end + secondsOffset + extraSeconds;

		if(startClip < 0){
			startClip = 0;
		}

		sprintf(commandChr, "ffmpeg -y -i %s -ss %d -to %d %s/output-%d.wav -loglevel \"error\"", inputAudioStr.c_str(), startClip, endClip, outputFolderStr.c_str(), i);
		//cout << "Running command: " << commandChr << endl;
		system(commandChr);
	}
}

void generateTextToSpeech(vector<srtLine> srtLines, char* outputFolder){

	string outputFolderStr(outputFolder);
	string command = "mkdir -p " + outputFolderStr;
	system(command.c_str());
	string textMessage;

	char commandChr[300];

	for(int i=0; i<srtLines.size(); i++){
		textMessage = "";
		for(int j=0; j<srtLines[i].messages.size(); j++){
			textMessage = textMessage + srtLines[i].messages[j];
		}
		//sprintf(commandChr, "espeak \"%s\" --stdout > %s/tts-output-%d.wav", textMessage.c_str(), outputFolderStr.c_str(), i);
		sprintf(commandChr, "espeak \"%s\" --stdout > %s/tts-output-%d.wav", textMessage.c_str(), outputFolder, i);
		//cout << "Running command: " << commandChr << endl;
		system(commandChr);
	}
}

void mergeTTSandAudio(vector<srtLine> srtLines, char* outputFolder){

	char ttsFile[300];
	char audioFile[300];
	char outputFile[300];
	string sil_audio = "silent_audio_2s.wav";

	char commandChr[300];

	for(int i=0; i<srtLines.size(); i++){
	
		sprintf(ttsFile, "%s/tts-output-%d.wav", outputFolder, i);
		sprintf(audioFile, "%s/output-%d.wav", outputFolder, i);
		sprintf(outputFile, "%s/tts-srt-out-%d.wav", outputFolder, i);

		/* Merge subtitle text-to-speech and extracted audio together */ 
		/* Overwrite existing output files with same name without prompt */
		sprintf(commandChr, "ffmpeg -y -i %s -i %s -i %s -i %s -filter_complex '[0:0][1:0][2:0][3:0]concat=n=4:v=0:a=1[out]' -map '[out]' %s -loglevel \"error\"", ttsFile, sil_audio.c_str(), audioFile, sil_audio.c_str(), outputFile);

		//cout << "Running command: ffmpeg concat: " << i << endl;
		system(commandChr);
	}
}


int main(int argc, char* argv[]){

	if(argc < 4){
		cout << "Not enough input arguments. Usage: ./audio_clip_generator input_audio.wav input_subtitles.srt output_folder" << endl;
		cout << "You can also specify the audio offset (integer) and extra seconds (integer) before and after each audio clip. Usage: ./audio_clip_generator input_audio.wav input_subtitles.srt output_folder audio_offset extra_seconds" << endl;
		return -1;
	}

	int secondsOffset = 0;

	if(argc > 4){
		secondsOffset = atoi(argv[4]);
	}

	int extraSeconds = 0;

	if(argc > 5){
		extraSeconds = atoi(argv[5]);
	}

	int res = initialChecks(argv[1], argv[2]);
	if(res < 0){
		return res;
	}

	vector<srtLine> srtLines = getSrtLines(argv[2]);
	
	displaySrtLines(srtLines);

	clipAudio(argv[1], srtLines, argv[3], secondsOffset, extraSeconds);

	generateTextToSpeech(srtLines, argv[3]);
	
	mergeTTSandAudio(srtLines, argv[3]);

	return 0;
}

