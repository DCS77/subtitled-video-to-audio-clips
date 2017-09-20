Title: subtitled-video-to-audio-clips
Author: DCS77
Date: 15/09/2017

######################################

Objective: Extract audio from a video file. Read subtitle files to select snippet times of each conversation. Create txt file with text contained in each snippet of audio. Generate snippets of audio for each line of the subtitle.

######################################

Requirements:

[1] - ffmpeg - https://www.ffmpeg.org/ - sudo apt install ffmpeg
[2] - espeak - http://espeak.sourceforge.net/ - sudo apt install espeak

######################################

How to use:

[1] - Convert video file to audio:

ffmpeg -i video.avi -vn audio.wav
ffmpeg -i video.avi -vn -f mp3 audio.mp3

(Run "ffmpeg --help" to see additional options)







Video and srt file are separate
Video is converted to audio
Srt file is scanned 


