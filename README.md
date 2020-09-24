本程序还在持续开发中，临时提供二进制测试包。分为Loongson版和AMD64版，放在了我的个人网站上。</br>
[点击打开下载页面（含使用说明）](http://www.loongson.xyz/Articles/Content/18)</br>
因为打包时没有检查调用的库文件哪些是系统中已有的，哪些是后来新增的，把所有用到的库文件全都放进了压缩包，文件就比较大。</br>
[开发日记](http://www.loongson.xyz/Articles/Content/19)</br>

# GueeRecorder （Guee 录屏机） 前言
　　这是一个为了学习Linux下的应用程序开发，而编写的一个实验性的对屏幕和摄像头录像，然后保存为视频文件或直播到网络的软件。</br>
　　这是我第一次在Linux上开发的软件，在此之前我从未实践过Linux下的软件开发，甚至于在购买基于国产CPU龙芯3A4000的电脑之前，我对Linux系统的累计的使用时间大概也只有几天。</br>
　　2020春节期间，在快递把龙芯3A4000送到之后，尝试编译了一款名为“SimpleScreenRecorder”的软件，但它的功能太简单，并且由于源码中对x86有大量SIMD汇编优化，但龙芯却只能使用兼容的C++代码，在龙芯CPU上的性能表现也就有些不堪。我数年前开发过Windows平台下的录屏软件，不过那是公司的产品，各种功能实现与Windows及x86也关联得较深，无法直接移植，于是我就萌生了在直接龙芯平台上重新开发一款录屏软件的想法。只是我也知道最大的性能瓶颈是视频编码库没有优化，可我能力有限，能够控制的只是把图像送入视频编码库之前的代码。但我还是想要尝试一下，性能优化有困难，但是在功能方面做得比它完善一些，应该不难。虽然我知道同类软件中有个大名鼎鼎的OBS，我预计要完成的功能只是OBS的子集，但作为第一次在Linux上开发的程序，只要能把预计的功能完成，并且在龙芯平台上的运行效率能稍高于SimpleScreenRecorder，那就是胜利。</br>
　　 然后一边学一边做大约花了半个月时间写了一些基础的代码，只是由于众所周知的原因，不久后孩子开始上网课，公司也要求远程工作，于是每天疲于奔命，就中断了开发。后来虽然稍有了一些空闲时间，但每天琐事仍然很多，自造轮子这种不太紧要的事情也就一直没有重新拾起。在连续的繁忙之后，每天颓废得连上网水帖看新闻都没有精力，开发中断之后，龙芯的电脑我都有半年没有开过机。</br>
　　本来很怀疑这个软件又会像我以往的众多半成品一样，一旦开发过程被打断，就会永远躺在硬盘里成为一个归档文件。但这毕竟是我第一次在龙芯电脑上开发软件，还是不愿就这么半途而废。即使只是为了熟悉Linux下应用软件开发，我也应该把它完成。近期工作生活都逐渐像疫情前那样按步就班有了条理，因此我决定继续开发这个软件。由于这个软件总体上算是学习过程中的实验品，无论功能、界面还是代码等都感觉有点凌乱，因此打算在把它完成之后，再重写所有的代码，继续提高软件的性能和完善软件功能，还需重新设计软件界面。大概，未来计划中的2.0版才会是令我自己基本满意的作品。</br>

# 功能说明
　　可以把屏幕、摄像头，图片等各种画面作为图层，合成在一起之后再编码为视频文件。软件有一个预览区域，在预览区域可以对各个图层平移和缩放。当选中某个图像时，会在预览区域中显示图层列表，可以通过功能按钮调整图层顺序及其它参数。可设置视频编码的参数，视频可保存为mp4或flv文件。同时具有较为灵活的录音功能，可以分别录制电脑播放的声音和麦克风输入，也可以随时对录音进行开关。</br>
### 屏幕录像：
　　录制全屏，支持单屏和多屏幕；</br>
　　录制窗口，支持选择整个窗口或窗口客户区（不含窗口标题栏和边框）；</br>
　　录制区域，选择区域和选择窗口的操作与QQ截图类似。</br>
　　可以任意添加屏幕多个区域，并可以随时添加和删除。
### 摄像头：
　　支持添加USB摄像头，可选择分辨率和帧率。</br>
　　每个摄像头只允许添加一次，但可以添加多个摄像头。
### 图片文件：
　　支持向画面中添加多个 bmp、png、jpg 等常见格式的文件。</br>
### 文字/涂鸦：
　　计划中，尚未开发。</br>
### 视频编码：
　　当前仅支持x264软件编码器。</br>
　　可自由设置为各种分辨率，不受截屏大小和摄像头分辨率等限制。</br>
　　可设置视频帧率，截屏或摄像头性能较低时进行视频编码。</br>
　　可设置视频编码的各种常规参数：码率（含码率控制方式）、预设质量等级、关键帧间隔等。</br>
　　可保存为 mp4 或 flv 格式的文件。
### 视频直播：
　　计划中，尚未开发。可以向支持rtmp推流的直播站点进行直播。</br>
### 声音录制：
　　可以同时录制电脑播放的声音和麦克风声音，也可以只录制其中之一，在录像过程中可以随时打开和关闭录音。</br>

# 开发环境
### 硬件环境：
　　CPU：龙芯 Loongson 3A4000@1.8GHz</br>
　　内存：紫光 DDR4 2666 8G</br>
　　显卡：AMD R5 230</br>
　　硬盘：WD SN750 1T Nvme</br>
### 软件环境：
　　OS：统信 UOS 专业版、龙梦 Fedora28</br>
　　开发工具：Qt 5.x</br>
#### 开发环境配置：
　　因为我对Linux还不熟悉，特别是被VisualStudio养成了懒人，对GCC这类编译器以及编译各种开源库都缺少经验，于是只要是软件源中存在的库，我就直接安装使用。即使库的版本老一点，即使编译参数不是最优……只要能凑合用就行。</br>
　　统信 UOS 专业版：</br>
　　　　sudo apt-get install g++ gdb</br>
　　　　sudo apt-get install libgl1-mesa-dev</br>
　　　　sudo apt-get install libqt5x11extras5-dev</br>
　　　　sudo apt-get install libxinerama-dev</br>
　　　　sudo apt-get install libxfixes-dev</br>
　　　　sudo apt-get install libx264-dev</br>
　　　　sudo apt-get install libfaac-dev</br>

　　　　sudo apt-get install qt5-default</br>
　　　　sudo apt-get install qt5creator</br>
</br>
　　龙梦 Fedora28：</br>
　　　　sudo yum inatall gcc-c++</br>
　　　　sudo yum inatall gdb</br>
　　　　sudo yum install mesa-libGL-devel</br>
　　　　sudo yum install mesa-libGLU-devel</br>
　　　　sudo yum install qt5-qtx11extras-devel</br>
　　　　sudo dnf install libXinerama-devel</br>
　　　　sudo dnf install libXfixes-devel</br>
　　　　sudo dnf install x264-devel</br>
　　　　//aac编码库需要自行下载和编译源码，请参考 [其他人写的文章](https://www.cnblogs.com/wayns/p/facc-install.html)</br>

　　　　sudo dnf inatall qt5-devel</br>
　　　　sudo dnf inatall qt-creator</br>
</br>

