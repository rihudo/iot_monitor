# How to transfer file
Divide the file to several segments.  
Then package each segment sperately to a message.  
The message format:
1. START message: "BABYCARE_START" + segments number(2byte) + FILE SIZE(4byte) + FILE NAME(remaining length of message)
2. DATA message: "BABYCARE_DATA" + segment index(2byte) + file data(remaining length of message)
3. END message: "BABYCARE_END"
