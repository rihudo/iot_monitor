# Iot monitor
## What modules are there
### monitor_client
It corresponds to folder "monitor_client".  
It uses the camera to record video after startup, and uploads a video file to mqtt server when there are enough frames in the file.  
You can type "monitor_client" in terminal for usage
### mqtt server
I use mosquitto, which is a is an open source (EPL/EDL licensed) message broker, to build the mqtt server.
### monitor_receiver
It corresponds to folder "monitor_receiver".  
It receives video files that forwarded by mqtt server from monitor_client. Then save these files to folder "./files".  
You can type "monitor_receiver" in terminal for usage
### video_service
It corresponds to folder "video_service".  
It's a web service used to provide viewing of video files recorded by monitor_client.  
Implemented by golang with gin, so you need to install gin manually.  
Usage: video_service [path of videos, default:./video]

## How to transfer file
Divide the file to several segments.  
Then package each segment sperately to a message.  
The message format:
1. START message: "BABYCARE_START" + segments number(2byte) + FILE SIZE(4byte) + FILE NAME(remaining length of message)
2. DATA message: "BABYCARE_DATA" + segment index(2byte) + file data(remaining length of message)
3. END message: "BABYCARE_END"
