# Convert Subtitled Video to Audio Clips

Extracts audio from a video file. Reads the .srt subtitle files to select snippet times of each conversation. Creates a txt file with text contained in each snippet of audio. Finally, generates snippets of audio for each line of the subtitle.

Why? This can be used to convert a movie of a target language into byte-sized snippets for language learning, or to create audio notes - by creating a custom .srt file containing your personal notes with timestamps according to a lecture, which are then merged into short clips for studying.

## Getting Started

1. First install the required [prerequisites](#Prerequisites)
2. Convert your video file to audio:

```
ffmpeg -i video.avi -vn audio.wav
ffmpeg -i video.avi -vn -f mp3 audio.mp3
```

(Run `ffmpeg --help` to see additional options)

3. Then compile and run program, passing files as inputs

(Pass `-h` or `--help` to see options)

### Prerequisites

On Linux, install [ffmpeg](https://www.ffmpeg.org/) and [espeak](http://espeak.sourceforge.net/).

For example, on Ubuntu/Debian:
```
sudo apt install ffmpeg espeak
```

### Pending Work

* Normalise volumes of both audio sources
* Investigate better text to speech and integrated into script
* Integrate conversion to audio into script
* Allow other video/subtitle formats such as .mkv to be converted
