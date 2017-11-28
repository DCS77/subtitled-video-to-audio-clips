/*
//	Author: Daniel Smit
//	Date: 21 Feb 2017
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
#include <fstream>

using namespace std;

struct srtLine;
int initialChecks(char* inputAudio, char* inputSrt);
int checkAudioFormat();
bool checkInputs(bool verbose, bool foundAudioFile, bool foundSubtitleFile, bool foundOutputFolder, bool foundAudioOffset, bool foundExtraSeconds);
int getTimestampStartSec(string line);
int getTimestampEndSec(string line);
string getTimestampStart(string line, float audioOffset, float extraSeconds);
string getTimestampEnd(string line, float audioOffset, float extraSeconds);
int matchesTimestampFormat(string line);
int matchesLineFormat(string line);
int matchesBlankLine(string line);
vector<srtLine> getSrtLines(char* inputSrt, float audioOffset, float extraSeconds);
void clipAudio(char* inputAudio, vector<srtLine> srtLines, char* outputFolder, int secondsOffset, int extraSeconds);
void generateTextToSpeech(vector<srtLine> srtLines, char* outputFolder);
void mergeTTSandAudio(vector<srtLine> srtLines, char* outputFolder);

struct srtLine {
	string start;
	string end;
	vector<string> messages;
};

int initialChecks(char* inputAudio, char* inputSrt){

	/* Checking files for compatibility */
	int res = checkAudioFormat();
	if(res == -1){
		cout << "Error checking video format" << endl;
		return -1;
	} else if(res == 0){
		cout << "Video format not supported" << endl;
	} else {
		cout << "Video format supported" << endl;
	}

	return res;
}

int checkAudioFormat(){
	return 1;
}

bool checkInputs(bool verbose, bool foundAudioFile, bool foundSubtitleFile, bool foundOutputFolder, bool foundAudioOffset, bool foundExtraSeconds){
	if(verbose){
		if(foundAudioFile){
			cout << "Input audio found." << endl;
		}
		if(foundSubtitleFile){
			cout << "Input subtitle found." << endl;
		}
		if(foundOutputFolder){
			cout << "Output folder specified." << endl;
		}
		if(foundAudioOffset){
			cout << "Audio offset specified." << endl;
		}
		if(foundExtraSeconds){
			cout << "Extra seconds specified." << endl;
		}
	}
	
	if(!foundAudioFile){
		cout << "Error: Input audio not specified correctly. Exiting." << endl;
		return 0;
	}
	if(!foundSubtitleFile){
		cout << "Error: Input srt subtitle file not specified correctly. Exiting." << endl;
		return 0;
	}
	if(!foundOutputFolder){
		cout << "Error: Output folder not specified correctly. Exiting." << endl;
		return 0;
	}

	return 1;
}

int getTimestampStartSec(string line){

	string hours_str{line.at(0), line.at(1)};
	string minutes_str{line.at(3), line.at(4)};
	string seconds_str{line.at(6), line.at(7)};

	int hours = stoi(hours_str);
	int minutes = stoi(minutes_str);
	int seconds = stoi(seconds_str);

	return floor(hours*60*60 + minutes*60 + seconds);
}

int getTimestampEndSec(string line){
	string hours_str{line.at(17), line.at(18)};
	string minutes_str{line.at(20), line.at(21)};
	string seconds_str{line.at(23), line.at(24)};

	int hours = stoi(hours_str);
	int minutes = stoi(minutes_str);
	int seconds = stoi(seconds_str);

	return ceil(hours*60*60 + minutes*60 + seconds);
}

string getTimestampStart(string line, float audioOffset, float extraSeconds){

	string hours_str{line.at(0), line.at(1)};
	string minutes_str{line.at(3), line.at(4)};
	string seconds_str{line.at(6), line.at(7)};
	string mili_str{line.at(9), line.at(10), line.at(11)};

	int hours = stoi(hours_str);
	int minutes = stoi(minutes_str);
	int seconds = stoi(seconds_str);
	int mili = stoi(mili_str);

	if(audioOffset - extraSeconds != 0){

		int totalMili = hours*3600000 + minutes*60000 + seconds*1000 + mili;
		totalMili += (audioOffset - extraSeconds)*1000;

		if(totalMili < 0){
			totalMili = 0;
		}

		hours = floor(totalMili / 3600000);
		minutes = floor((totalMili - hours*3600000) / 60000);
		seconds = floor((totalMili - hours*3600000 - minutes*60000) / 1000);
		mili = floor((totalMili - hours*3600000 - minutes*60000 - seconds*1000));
	}

	char newTimestamp[30];
	sprintf(newTimestamp, "%02d:%02d:%02d.%03d", hours, minutes, seconds, mili);
	string returnTimestamp(newTimestamp);

	return returnTimestamp;
}

string getTimestampEnd(string line, float audioOffset, float extraSeconds){

	string hours_str{line.at(17), line.at(18)};
	string minutes_str{line.at(20), line.at(21)};
	string seconds_str{line.at(23), line.at(24)};
	string mili_str{line.at(26), line.at(27), line.at(28)};

	int hours = stoi(hours_str);
	int minutes = stoi(minutes_str);
	int seconds = stoi(seconds_str);
	int mili = stoi(mili_str);


	if(audioOffset + extraSeconds != 0){

		int totalMili = hours*3600000 + minutes*60000 + seconds*1000 + mili;
		totalMili += (audioOffset + extraSeconds)*1000;

		if(totalMili < 0){
			totalMili = 0;
		}

		hours = floor(totalMili / 3600000);
		minutes = floor((totalMili - hours*3600000) / 60000);
		seconds = floor((totalMili - hours*3600000 - minutes*60000) / 1000);
		mili = floor((totalMili - hours*3600000 - minutes*60000 - seconds*1000));

	}

	char newTimestamp[30];
	sprintf(newTimestamp, "%02d:%02d:%02d.%03d", hours, minutes, seconds, mili);
	string returnTimestamp(newTimestamp);

	return returnTimestamp;
}

int matchesTimestampFormat(string line){
	/* Expected srt format: "HH:MM:SS,mmm --> HH:MM:SS,mmm" */
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

vector<srtLine> getSrtLines(char* inputSrt, float audioOffset, float extraSeconds){
	ifstream srtStream(inputSrt);
	
	int mode = 0; /* mode: 0 if haven't started yet, or looking for sequence number; 1 if found next sequence number; 2 if found timestamp; 3 if found text subtitle; back to 0 if found blank line */
	int currentIdx = 1; /* currentIdx: Current srt index. Starts at 1 as per specification. Incremented for each new srt */
	int stringIdx = 0;
	string line;
	vector<srtLine> srtLines;

	string startPoint;
	string endPoint;
	vector<string> messages;

	while(getline(srtStream, line)){

		line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
		line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
		
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
				startPoint = getTimestampStart(line, audioOffset, extraSeconds);
				endPoint = getTimestampEnd(line, audioOffset, extraSeconds);
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
	}

	return srtLines;
}

void displaySrtLines(vector<srtLine> srtLines){
	cout << "srtLines.size(): " << srtLines.size() << endl;

	for(int i=0; i<srtLines.size(); i++){
		cout << "start: " << srtLines[i].start << endl;
		cout << "end: " <<  srtLines[i].end << endl;
		cout << "message 0: " <<  srtLines[i].messages[0] << endl;
	}
}

void clipAudio(char* inputAudio, vector<srtLine> srtLines, char* outputFolder, int secondsOffset, int extraSeconds){
	
	string inputAudioStr(inputAudio);
	string outputFolderStr(outputFolder);

	string command = "mkdir -p " + outputFolderStr;
	system(command.c_str());

	char commandChr[300];

	
	for(int i=0; i<srtLines.size(); i++){
		sprintf(commandChr, "ffmpeg -y -i %s -ss %s -to %s %s/output-%d.wav -loglevel \"error\"", inputAudioStr.c_str(), srtLines[i].start.c_str(), srtLines[i].end.c_str(), outputFolderStr.c_str(), i);
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

		sprintf(commandChr, "rm %s %s", ttsFile, audioFile);

		system(commandChr);
	}
}


int main(int argc, char* argv[]){

	bool foundAudioFile = false;
	bool foundSubtitleFile = false;
	bool foundOutputFolder = false;
	bool foundAudioOffset = false;
	bool foundExtraSeconds = false;
	bool verbose = false;

	int audioFileIdx;
	int subtitleFileIdx;
	int outputFolderIdx;
	float audioOffset = 0;
	float extraSeconds = 0;

	for(int i=1; i<argc; i++){

		cout << "For loop checking input args: " << i << endl;

		if(i < argc - 1){
			/* Check for audio file */
			if(strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0){
				fstream fs;
				fs.open(argv[i+1]);
				if(fs.is_open()){
					foundAudioFile = true;
					audioFileIdx = i+1;
				} else {
					cout << "Error occurred for argument " << i << ":" << argv[i+1] << endl;
					cout << "Audio file could not be opened. Exiting." << endl;
					return 0;
				}
			}

			/* Check for subtitle file */
			if(strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--sub") == 0){
				fstream fs;
				fs.open(argv[i+1]);
				if(fs.is_open()){
					foundSubtitleFile = true;
					subtitleFileIdx = i+1;
				} else {
					cout << "Error occurred for argument " << i << ":" << argv[i+1] << endl;
					cout << "Subtitle file could not be opened. Exiting." << endl;
					return 0;
				}
			}

			/* Check for output folder argument */
			if(strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--out") == 0){
				foundOutputFolder = true;
				outputFolderIdx = i+1;
			}

			/* Check for audio offset */
			if(strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--offset") == 0){
				foundAudioOffset = true;
				audioOffset = atof(argv[i+1]);
			}

			/* Check for extra seconds required */
			if(strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--extra") == 0){
				foundExtraSeconds = true;
				extraSeconds = atof(argv[i+1]);
			}
		}

		/* Check for verbose */
		if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0){
			verbose = true;
		}

		/* Check for help */
		if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0){
			cout << "Usage: ./audio_clip_generator -i input_audio.wav -s input_subtitles.srt -o output_folder [options]" << endl;
			cout << "Required arguments:" << endl;
			cout << "  -i\tInput audio file (e.g. audio.wav)" << endl;
			cout << "    \t\tAlias: --input" << endl;
			cout << "  -s\tInput .srt subtitle file (e.g. subtitles.srt)" << endl;
			cout << "    \t\tAlias: --sub" << endl;
			cout << "  -o\tOutput folder (e.g. AudioOutputFolder)" << endl;
			cout << "    \t\tAlias: --out" << endl;
			cout << "Options:" << endl;
			cout << "  -a\tAudio offset (seconds) - e.g. if subtitles occur 1 second before audio, use -a 1" << endl;
			cout << "    \t\tAlias: --offset" << endl;
			cout << "  -e\tExtra seconds before and after each audio clip (seconds) - e.g. to capture audio 2 seconds before and after the times specified in subtitle file, use -e 2" << endl;
			cout << "    \t\tAlias: --extra" << endl;
			cout << "  -v\tVerbose mode" << endl;
			cout << "    \t\tAlias: --verbose" << endl;
			cout << endl;
		}
	}

	cout << "Checking inputs" << endl;
	if( !checkInputs(verbose, foundAudioFile, foundSubtitleFile, foundOutputFolder, foundAudioOffset, foundExtraSeconds) ){
		return 0;
	}

	cout << "getting srt lines" << endl;
	vector<srtLine> srtLines = getSrtLines(argv[subtitleFileIdx], audioOffset, extraSeconds);
	
	cout << "Displaying srt lines" << endl;
	displaySrtLines(srtLines);

	cout << "Clipping audio" << endl;
	clipAudio(argv[audioFileIdx], srtLines, argv[outputFolderIdx], audioOffset, extraSeconds);

	cout << "Generating text to speech" << endl;
	generateTextToSpeech(srtLines, argv[outputFolderIdx]);
	
	cout << "Merging text to speech and clipped audio" << endl;
	mergeTTSandAudio(srtLines, argv[outputFolderIdx]);

	//mergeAllOutput(srtLines, argv[outputFolderIdx]);

	return 0;
}

