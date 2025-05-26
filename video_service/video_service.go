package main

import (
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"time"

	"github.com/gin-gonic/gin"
)

func main() {
	ts_path_map := make(map[string]string)
	args := os.Args
	video_path := "./video"
	if len(args) >= 2 {
		fmt.Println("Use video path:", args[1])
		video_path = args[1]
	}

	r := gin.Default()
	r.LoadHTMLGlob("templates/*")
	r.Static("/videos", video_path)

	// 动态文件目录接口
	r.GET("/", func(c *gin.Context) {
		files, _ := ioutil.ReadDir(video_path) // 读取视频目录
		videoList := make([]string, 0)
		for _, f := range files {
			if filepath.Ext(f.Name()) == ".mp4" { // 过滤 MP4 文件[1](@ref)
				file_name := f.Name()[:strings.Index(f.Name(), ".")]
				timestamp, _ := strconv.ParseInt(file_name, 10, 64)
				t := time.Unix(timestamp, 0)
				time_str := t.Format("2006-01-02 15:04:05")
				ts_path_map[time_str] = f.Name()
				videoList = append(videoList, time_str)
			}
		}
		c.HTML(200, "index.html", gin.H{"videos": videoList}) // 模板渲染[2,3](@ref)
	})

	// 视频播放接口
	r.GET("/play/:filename", func(c *gin.Context) {
		file_path, ok := ts_path_map[c.Param("filename")]
		if ok {
			c.HTML(200, "play_video.html", gin.H{"video_name": file_path, "video_time": c.Param("filename")})
		} else {
			c.String(404, "No this video")
		}
	})

	r.GET("/favicon.ico", func(c *gin.Context) {
		c.File("./templates/favicon.png")
	})

	r.Run(":8080")
}
