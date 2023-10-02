## 大概框架
```angular2html

分类：

    //初始化类    ----> IniterI //初始化ifmt
    Initer-----|
                ----> IniterO //初始化ofmt

    //解码器类
    Decoder

    //编码器类
    Encoder

    //帧创建类
    FrameCreater

    //不可复制类
    Noncopyable

    //写文件类
    Writer

    //线程类
    Thread

    //定时器类
    Timer

    //工具类
    Utils

    //转码类
    Transcoder


```
### Initer
```angular2html
    基类为 Initer 包含两个子类继承 IniterI  IniterO

    分别通过输入的"文件路径" 初始化 AVFormatContext

    定义了get_fmt_ctx方法获取AVFormatContext上下文


```

### Decoder
```angular2html
    类为Decoder 包含解码器初始化方法、 解码操作方法

    可以根据设置的帧率进行补帧丢帧操作

    私有变量包括AVCodecContext audio和video

    私有成员包括两个解码队列 audio和video 解码的帧存储在其中



```

### Encoder
```angular2html
    类为Encoder 包含编码器初始化方法、编码操作方法


    私有变量包括 编码器上下文AVCodecContext 音频采样信息 
    一帧音视频播放时间等


```



### Writer
```angular2html

    Writer类主要包括写文件头 写packet(文件内容) 写文件尾 
    
    成员函数用了static修饰 可以直接调用


```




### Utils
```angular2html
    工具类主要包括了对数据帧的处理方法

    包括数据帧图像合并 数据帧分辨率改变 AVFrame和Mat格式相互转换等方法

    成员函数用了static修饰可以直接调用

```


### Noncopyable/Thread
```angular2html
    Noncopyable主要是保证对象不可复制

    Thread类主要是封装了c++11的std::thread 
    继承了Noncopyable保证线程对象不可复制


    构造函数使用了可变参数模板和forward 实现了传入内容的灵活性
    template<typename Function, typename... Args>
    可以是不同参数不同返回值函数 也可以是对象的成员函数

```


### Timer
```angular2html
    暂时只有getCurrentTime()获取当前时间 用了static修饰 可以直接调用
```



### Transcoder
```angular2html
    转码类 可以说是转码操作的入口 支持转MP4、rtmp推流等方法
    
    初始化设置需要分辨率大小 帧率 采样率
    先调用初始化：IniterI--->decoder+encoder--->IniterO
    写文件头：Writer::write_header
    线程调用方法：Thread(decode()...)解码 + Thread(encode())编码(writer_packet)
    写文件尾: Writer::write_tail
```


## 解码丢帧补帧问题
```angular2html
    根据原视频帧率和设置帧率进行补帧丢帧
    补帧: if(set > src){
            ratio = (set - src) * 1.0 / src
            count = 0.0

            while(!over){
                while(count >= 1.0){
                    //补帧
                    count -= 1.0
                }       
                else{
                    //正常解码
                    count += ratio
                }
            }
       }


    丢帧: if(set <= src){
            ratio = (src - set) * 1.0 / set
            count = 1.0
            
            while(!over){
                while(count >= 1.0){
                    //放弃这个帧
                    count -= 1.0
                    continue
                }

                else{
                    正常解码
                    count += ratio
                }
            }
        }

```



## 计时部分
```angular2html

1.编码第1帧的时间--------编码第30帧的时间

2.1帧从提取到编码用了多长时间

3.前一帧编码结束 到新的一帧开始用了多长时间


```


## 音频帧转换问题
```angular2html

decoder---->deocded_queue(audio)---->frame----->audio_fifo

----->trans---->new frame---->encode

```

