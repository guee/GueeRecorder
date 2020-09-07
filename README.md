# GueeRecorder 前言
　　这是一个为了学习Linux下的应用程序开发，而编写的一个实验性的对屏幕和摄像头录像，然后保存为视频文件或直播到网络的软件。</br>
　　这是我第一次在Linux上开发的软件，在此之前我从未实践过Linux下的软件开发，甚至于在购买基于国产CPU龙芯3A4000的电脑之前，我对Linux系统的累计的使用时间也没超过24小时。</br>
　　2020春节期间，在快递把龙芯3A4000送到之后，尝试编译了一款名为“SimpleScreenRecorder”的软件，但它的功能太简单，并且由于源码中有对x86的SIMD优化，但龙芯却只能使用兼容的C++代码，在龙芯CPU上的性能表现也有些不堪。我数年前开发过Windows平台下的录屏软件，不过那是公司的产品，各种功能实现与Windows及x86也关联得较深，无法直接移植，于是我就萌生了在直接龙芯平台上重新开发一款录屏软件的想法。可是我也暂时没有手写LoongISA/MIPS汇编的能力，何况在进行分析之后，发现最大的性能瓶颈还是在于视频编码库的优化不足，因此在性能方面要想做到更好比较困难，但是在功能方面做得比它完善一些，应该没有问题。虽然我知道同类软件中有个大名鼎鼎的OBS，我预计要完成的功能只是OBS的子集，但作为第一次在Linux上开发的程序，只要能把预计的功能完成，并且在龙芯平台上的运行效率能稍高于SimpleScreenRecorder，那就是胜利。</br>
　　 然后一边学一边做大约花了半个月时间写了一些基础的代码，只是由于众所周知的原因，不久后孩子开始上网课，公司也要求远程工作，于是每天疲于奔命，就中断了开发。后来虽然稍有了一些空闲时间，但每天琐事仍然很多，自造轮子这种不太紧要的事情也就一直没有重新拾起。在连续的繁忙之后，每天颓废得连上网水帖看新闻都没有精力，开发中断之后，龙芯的电脑我都有半年没有开过机。</br>
　　本来很怀疑这个软件又会像我以往的众多半成品一样，一旦开发过程被打断，就会永远躺在硬盘里成为一个归档文件。但这毕竟是我第一次在龙芯电脑上开发软件，还是不愿就这么半途而废。即使只是为了熟悉Linux下应用软件开发，我也应该把它完成。近期工作生活都逐渐像疫情前那样按步就班有了条理，因此我决定继续开发这个软件，并且开源发布，期待有人能监督和催促我完成，免得我又犯了懒病。由于这个软件总体上算是学习过程中的实验品，无论功能、界面还是代码等都感觉有点混乱，因此打算在把它完成之后，再重写所有的代码，继续提高软件的性能和完善软件功能，还需重新设计软件界面，未来计划中的2.0版才会是令我自己基本满意的作品。</br>

# 功能说明
　　可以录制屏幕、摄像头，还可加入图片和文本，并把这些内容合成在同一个画面上再编码为视频文件。软件有一个预览区域，在预览区域可以拖动各个图像的大小和位置。当选中某个图像时，会在预览区域中显示图层列表，可以通过功能按钮调整图层顺序及其它参数，或选中其它图层。可设置视频编码的参数，视频可保存为mp4或flv文件，也可直接向rtmp服务器推送直播视频流</br>
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
　　开发中。可以同时录制电脑播放的声音和麦克风声音，也可以只录制其中之一，在录像过程中可以随时打开和关闭录音。</br>

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
　　　　sudo apt-get install libx264-dev</br>
　　　　sudo apt-get install qt5-default</br>
　　　　sudo apt-get install qt5creator</br>
</br>
　　龙梦 Fedora28：</br>
　　　　sudo dnf inatall gcc-g++</br>
　　　　sudo dnf inatall gdb</br>
　　　　sudo dnf install libgl1-mesa-dev</br>
　　　　sudo dnf install libqt5x11extras5-dev</br>
　　　　sudo dnf install libxinerama-dev</br>
　　　　sudo dnf install x264-devel</br>
　　　　sudo dnf inatall qt5-devel</br>
　　　　sudo dnf inatall qt-creator</br>
</br>

# 开发日记（日期逆序）：
2020-09-07
　　决定把编码画面时间戳误差较大的问题放到以后再改善，先把录音的功能加上。今天用SimpleScreenRecorder测试了ALSA、PulseAudio、JACK几种接口的录音，发现UOS上默认情况下只有ALSA能录到声音，Qt中应该也是用的ALSA的录音方案。于是翻看Qt的例子代码，打算就直接用Qt的库做录音功能了。但是录音如果采样率与音频编码库及视频文件格式的要求不匹配，还是要自己写一段音频重采样的代码。</br>

2020-09-06</br>
　　今天继续修改昨天没改完的问题，目前剩下的最严重的问题是画面运动不平顺。由于程序设计为屏幕截图只是画面来源之一，可以随时添加删除，因此画面渲染线程和屏幕截图线程都有独立的帧率控制，以渲染线程的时间作为视频帧的时间戳，而渲染线程只有在图像来源的内容有变化后，且在帧间时间间隔到达后，才会绘制一帧画面送到编码器，这样帧时间就与实际的截图时间产生了不平均的差异，造成从视觉感受上看，视频会有轻微的卡顿。可是由于画面内容可以随时增删，帧时间就不能以屏幕截图时间为准备，可能需要对内部的流程做些调整。</br>
　　今天还和SimpleScreenRecorder简单对比了一下性能。虽然我的程序对图像的处理流程上要复杂得多，但在龙芯平台上我的程序效率仍然略高一点，但在x86(i5-4460@3.2GHzx4) UOS上SimpleScreenRecorder的CPU占用率要比我的程序低十个百分点。因为两者使用相同的x264库文件，编码参数也尽量一致，性能差距就只发生在视频编码之前。证实了我以前的判断，因为它对龙芯CPU没有任何优化，所以即使简单的颜色空间转换，也比我更复杂的处理流程消耗了更多的CPU时间。我以前测试过用SSE计算1080@30fps的RGB to YUV 转换，i5级别的CPU消耗在1%~2%之间，未优化的C++直接计算则需要10倍甚至更多的CPU时间。另外还有x264编码库的性能更是瓶颈，代码优化和未优化达到几倍的性能差距很常见，取决于优化的程度，有的程序达到几十倍的性能差距也不罕见。我在龙芯开发者社区看到Loongnix系统中有龙芯优化过的x264库文件，可惜作为Linux新手的我没能在Loongnix成功配置Qt的开发环境，暂时试验不了龙芯优化过的x264编码库有多少性能提升。</br>

2020-09-05</br>
　　之前对 QWaitCondition 的理解有误，造成编码线程偶尔出现一直 wait 的情况，用惯了 Windows 的 SetEvent 造成了认知障。</br>
　　设置界面初步搭建完成，然后暴露出许多bug，比如修改分辨率造成重新计算图像绘制坐标时出现混乱，以及预览区域绘制错误。然后发现编码生成的视频文件也有许多问题，时间戳计算有误，画面运动不平滑。还有之前设计的用OpenGL把RGB转换到YUV420也发现结果不正确，是因为奇数分辨率没有处理好，以及行字节没有对齐的原因。

2020-09-04</br>
　　把OpenGL渲染合成的画面绘制另一个FBO，使用着色器完成了RGB到YUV(I420)的转换，从FBO中取出数据就可以直接送往x264编码器了。</br>
　　以前在Windows下写过一个从TS/MP4文件提取H.264流，并重新封装为其它文件格式的小工具，现在把写文件相关的代码拿到这个项目中，进行了一些修改，已经整合完成。屏幕、摄像头画面合成->编码为H.264流->保存为FLV/MP4文件的整个流程已经调通，但暂时还是没有音频。</br>
　　接下来需要完成的是视频编码设置界面，当前连分辨率和帧率都是直接写在代码中，没有设置界面这个软件就没法正常使用。</br>

2020-08-31</br>
　　把本项目上传到GitHub，并补上日志记录开始过程。</br>
　　对摄像头数据的处理，是把YUYV数据放入OpenGL纹理，在着色器中转换为RGB再用于渲染，不使用CPU计算。</br>
　　由于对Linux录音完全不了解，打算先把画面编码为h.264之后再增加录音功能。另外打算把合成的画面渲染到FBO时做一些处理，最好是从FBO取出就是I420的数据，可以直接送到x264编码器编码视频。</br>

2020-08-29</br>
　　开发环境换回到真机，还是在龙芯3A4000上开发，只是操作系统从Fedora28换成了UOS，不是Fedora28不好，只是UOS更漂亮，顔值即正义。</br>
　　验证了Qt支持的数种获取摄像头数据的方法，决定还是取得YUYV原始数据，自己再处理为RGB，我认为这样会有更高的效率。</br>

2020-08-27</br>
　　实验增加录制摄像头功能。并把较简单的对图像文件的支持加上。</br>
　　实验录音功能，由于对Linux了解太少，暂时没有头绪，Qt自带的录音类感觉效率有点低。</br>
　　实践证明在x86+Win10+VMWare+UOS的环境中，摄像头功能和录音功能都跑不通，决定不再尝试在虚拟机中开发。</br>

2020-08-25</br>
　　注册统信UOS开发者身份成功，下载统信UOS继续折腾操作系统，最后在硬盘上安装了3个操作系统，并配置了各自的开发环境。</br>
　　在x86笔记本Win10下，尝试在VMWare中安装UOS，然后把项目放入虚拟机，把现有功能调通。</br>

2020-08-23</br>
　　折腾操作系统，做乱七八糟的实验。反反复复轮流重新安装Fedora28和Loongnix。</br>

2020-08-22</br>
　　中断了半年的开发继续开始，决定晚上不上网刷帖灌水，尽量抽出时间完成这个软件。</br>
　　由于上次中断开发时，正在对一些功能进行调试，于是程序重新运行起来什么都不对。梳理了一遍代码，看懂了自己半年前写的东西，重新把已有的功能跑通。</br>

————————————————————————————</br>
开发中断6个月+</br>
————————————————————————————</br>

2020-02-15 左右</br>
　　公司复工，远程上班，加上孩子网课和作业，精力耗尽，开发中断。</br>
　　此时完成了在预览界面中编辑图层，以及对其它一些功能的完善。</br>

2020-02-10 左右</br>
　　开始陪伴孩子上网课，自由时间锐减。</br>
　　基本完成主界面搭建、选择截图区域的界面、屏幕截图、帧率控制、图像合成和预览、基本的图层管理等。</br>
　　这段时间的开发，Qt的跨平台特性作用很大。另外就是学习了x11的一些API，完成了屏幕载取相关功能。</br>
　　虽然使用的截图API与SimpleScreenRecorder这款软件使用的相同，但我对截图数据的缓存和使用方式和它不同，应该更有效率一些。</br>
　　x11截屏性能不高，可我对Linux还不熟，暂时没有找到其它的截屏相关的技术资料，打算以后读一读OBS的代码，看看它有没有什么不同。</br>

2020-02-01 左右</br>
　　项目开始。初始开发环境：龙芯3A4000 + 龙梦Fedora28 + Qt5.x。</br>
　　我把龙芯3A4000电脑原配的AMD R5 230显卡更换成了AMD RX580，龙芯官方的loongnix系统暂时不能驱动它。相对来说龙梦Fedora28对个人用户更友好，虽然对各种办公设备的驱动可能不如loongnix完善，但对个人电脑常见的硬件能够支持得更好一些。
