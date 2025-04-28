香橙派使用USB摄像头的方法

USB摄像头测试

1. 首先将USB摄像头插入到Orange Pi开发板的USB接口中

2. 然后通过`lsmod | grep uvcvideo`命令可以看到内核自动加载了下面的模块

   ![image-20250421214725833](D:\typora\picture\image-20250421214725833.png)

3. 通过`v4l2-ctl`命令可以看到USB摄像头的设备节点信息为`dev/video1`（需要注意的是这里的video1可能会改变，下一次插入它显示的可能就为video0）

   ~~~shell
   #1.首先安装v4l-utils工具
   sudo apt-get update
   sudo apt-get install -y v4l-utils
   
   #2.使用v4l-untils工具查看摄像头设备节点
   orangepi@orangepizero2:~$ v4l2-ctl --list-devices
   cedrus (platform:cedrus):
           /dev/video0
           /dev/media0
   
   #这个是我的摄像头
   Q8 HD Webcam: Q8 HD Webcam (usb-5200000.usb-1):
           /dev/video1			#通常是第一个要用的设备节点信息	
           /dev/video2
           /dev/media1	
   ~~~

4. 使用`fswebcam`测试USB摄像头

   - 安装`fswebcam`

     ~~~shell
     sudo apt-get update
     sudo apt-get install -y fswebcam
     ~~~

   - 安装完`fswebcam`后可以使用下面的命令来拍照

     - `-d`选项用于指定USB摄像头的设备节点
     - `--no-banner`用于去除照片的水印
     - `-r`选项用于指定照片的分辨率
     - `-S`选项用于设置与跳过前面的帧数
     - `./image.jpg`用于设置生成照片的名字和路径

     ~~~shell
     sudo fswebcam -d /dev/video1 --no-banner -r 1280x720 -S 5 ./image.jpg
     ~~~

   - 在香橙派上拍照完成后，可以使用`scp`指令将照片传到Ubuntu上去查看

     ~~~shell
     scp image.jpg usrname@192.168.0.31:/home/username
     #根据实际情况修改用户名、IP地址和路径
     ~~~

   - 在Ubuntu中可以直接查看刚刚在香橙派上拍的照片

5. 使用`mjpg-streamer`测试USB摄像头

   - 下载`mjpg-streamer`

     ~~~shell
     git clone https://gitee.com/leeboby/mjpg-streamer
     #这里选用国内的gitee，国外的GitHub对网络要求较高
     ~~~

   - 安装依赖的软件包

     ~~~shell
     #Ubuntu系统
     sudo apt-get install -y cmake libjpeg8-dev
     ~~~

   - 编译安装`mjpg-streamer`

     ~~~shell
     cd mjpg-streamer/mjpg-streamer-experimental/
     make -j4		#-j4表示使用所有板载内核来处理任务
     sudo make install
     ~~~

   - 然后输入下面的命令启动`mjpg-streamer`

     ~~~shell
     export LD_LIBRARY_PATH=.	#将当前路径添加到环境变量中
     sudo \
     ./mjpg_streamer -i "./input_uvc.so -d /dev/video1 -u -f 30" \
     -o "./output_http.so -w ./www"
     
     #这里的video1要根据上边执行v4l2-ctl --list-devices来确定
     ~~~

   - 然后在浏览器输入`开发板IP地址+端口号`就能查看摄像头输出的画面了

     <img src="https://gitee.com/persist011104/imgs/raw/master/imgs/202406122258335.png" alt="image-20240609162724616" style="zoom:50%;" />

USB摄像头使用

上边已经介绍了怎么测试板子是否能够正确的连接USB摄像头，接下来介绍一下后续怎么使用它。

启动脚本

在`/mjpg-streamer/mjpg-streamer-experimental`文件夹中默认就有一个启动的脚本，查看脚本内容发现有一句话和刚刚输入的测试命令很像

![image-20250422000911024](D:\typora\picture\image-20250422000911024.png)

直接将这条命令修改成如下内容：

~~~shell
./mjpg_streamer -i "./input_uvc.so -d /dev/video0 -u -f 30" -o "./output_http.so -w ./www"
#这里的/dev/video1不是固定的，修改完内容后报错退出
~~~

然后直接在当前目录运行`./start.sh`看看服务能否正常运行，同上在浏览器输入IP地址进行查看。

<img src="https://gitee.com/persist011104/imgs/raw/master/imgs/202406122258334.png" alt="image-20240609162638988" style="zoom:80%;" />

<img src="https://gitee.com/persist011104/imgs/raw/master/imgs/202406122258335.png" alt="image-20240609162724616" style="zoom:50%;" />

udev规则

简单介绍以下`udev`规则，`udev`规则就是当一个设备插入到板子中，内核时最先知晓的，这时候`udev`就给这个设备创建一个单独的文件，当这个设备拔走时，`udev`又会将这个文件自动删除。这样极大地方便了用户查找对应的设备文件，所以现在给摄像头编写`udev`规则，保证摄像头对应的设备文件时唯一的。这是维基百科关于udev的概述：[udev - 维基百科，自由的百科全书](https://zh.wikipedia.org/wiki/Udev)

1. 查看usb ID：`lsusb`

   ![image-20250422002525406](D:\typora\picture\image-20250422002525406.png)

   通过拔插摄像头确定摄像头的设备ID是：`1bcf:2281`

2. 查看usb设备的信息

   ~~~shell
   #首先使用v4l2-ctl指令查看usb摄像头的设备节点
   v4l2-ctl --list-devices
   Q8 HD Webcam: Q8 HD Webcam (usb-fc880000.usb-1):
           /dev/video1
           /dev/video2
           /dev/media0
   #然后使用指令查看usb设备信息，这里的video1不是固定的，是要根据上边的设备节点进行调整的
   udevadm info -a -p /sys/class/video4linux/video1
   
   looking at device '/devices/platform/soc/5200000.usb/usb2/2-1/2-1:1.0/video4linux/video1':
       KERNEL=="video1"
       SUBSYSTEM=="video4linux"
       DRIVER==""
       ATTR{dev_debug}=="0"
       ATTR{index}=="0"
       ATTR{name}=="Q8 HD Webcam: Q8 HD Webcam"
       ATTR{power/control}=="auto"
       ATTR{power/runtime_active_time}=="0"
       ATTR{power/runtime_status}=="unsupported"
       ATTR{power/runtime_suspended_time}=="0"
   
     looking at parent device '/devices/platform/soc/5200000.usb/usb2/2-1/2-1:1.0':
       KERNELS=="2-1:1.0"
       SUBSYSTEMS=="usb"
       DRIVERS=="uvcvideo"
       ATTRS{authorized}=="1"
       ATTRS{bAlternateSetting}==" 0"
       ATTRS{bInterfaceClass}=="0e"
       ATTRS{bInterfaceNumber}=="00"
       ATTRS{bInterfaceProtocol}=="00"
       ATTRS{bInterfaceSubClass}=="01"
       ATTRS{bNumEndpoints}=="01"
       ATTRS{iad_bFirstInterface}=="00"
       ATTRS{iad_bFunctionClass}=="0e"
       ATTRS{iad_bFunctionProtocol}=="00"
       ATTRS{iad_bFunctionSubClass}=="03"
       ATTRS{iad_bInterfaceCount}=="02"
       ATTRS{interface}=="Q8 HD Webcam"
       ATTRS{supports_autosuspend}=="1"
   
     looking at parent device '/devices/platform/soc/5200000.usb/usb2/2-1':
       KERNELS=="2-1"
       SUBSYSTEMS=="usb"
       DRIVERS=="usb"
       ATTRS{authorized}=="1"
       ATTRS{avoid_reset_quirk}=="0"
       ATTRS{bConfigurationValue}=="1"
       ATTRS{bDeviceClass}=="ef"
       ATTRS{bDeviceProtocol}=="01"
       ATTRS{bDeviceSubClass}=="02"
       ATTRS{bMaxPacketSize0}=="64"
       ATTRS{bMaxPower}=="500mA"
       ATTRS{bNumConfigurations}=="1"
       ATTRS{bNumInterfaces}==" 4"
       ATTRS{bcdDevice}=="0429"
       ATTRS{bmAttributes}=="80"
       ATTRS{busnum}=="2"
       ATTRS{configuration}==""
       ATTRS{devnum}=="2"
       ATTRS{devpath}=="1"
       ATTRS{devspec}=="(null)"
       ATTRS{idProduct}=="2281"
       ATTRS{idVendor}=="1bcf"
       ATTRS{ltm_capable}=="no"
       ATTRS{manufacturer}=="Q8 HD Webcam"
       ATTRS{maxchild}=="0"
       ATTRS{power/active_duration}=="4473204"
       ATTRS{power/autosuspend}=="2"
       ATTRS{power/autosuspend_delay_ms}=="2000"
       ATTRS{power/connected_duration}=="4478404"
       ATTRS{power/control}=="auto"
       ATTRS{power/level}=="auto"
       ATTRS{power/persist}=="1"
       ATTRS{power/runtime_active_time}=="4472866"
       ATTRS{power/runtime_status}=="active"
       ATTRS{power/runtime_suspended_time}=="5128"
       ATTRS{product}=="Q8 HD Webcam"
       ATTRS{quirks}=="0x0"
       ATTRS{removable}=="unknown"
       ATTRS{rx_lanes}=="1"
       ATTRS{speed}=="480"
       ATTRS{tx_lanes}=="1"
       ATTRS{urbnum}=="1115253"
       ATTRS{version}==" 2.00"
   
   
   #这里需要注意的是它会输出很多的信息，而且通过观察不难发现它输出的形式是逐级向父目录靠近的，这里我们只需要获取到它的几个关键值，使用grep指令过滤出关键信息，由于知道它是逐级向父目录靠近的，所有第一个拿到的就是摄像头对应的idProduct和idVendor
   orangepi@orangepizero2:~$ udevadm info -a -p /sys/class/video4linux/video1 | grep "ATTRS{idProduct}"
       ATTRS{idProduct}=="2281"
       ATTRS{idProduct}=="0002"
   orangepi@orangepizero2:~$ udevadm info -a -p /sys/class/video4linux/video1 | grep "ATTRS{idVendor}"
       ATTRS{idVendor}=="1bcf"
       ATTRS{idVendor}=="1d6b"
   
   #拿到几个关键的信息
   KERNEL=="video1"
   SUBSYSTEM=="video4linux"
   DRIVERS=="uvcvideo"
   ATTRS{idProduct}=="2281"
   ATTRS{idVendor}=="1bcf"
   #这几个地方都对应的摄像头的唯一标识
   ~~~

3. 构建udev规则

   基于上述信息，编写唯一的规则来匹配摄像头设备。

   ~~~shell
   cd /etc/udev/rules.d
   ls		#查看当前目录下的udev规则，需要注意的是必须以.rules为后缀，否则不会生效
   orangepi@orangepizero2:/etc/udev/rules.d$ ls
   10-wifi-disable-powermanagement.rules  50-usb-realtek-net.rules 
   sudo vi orangepi_camera.rules
   # 匹配摄像头设备
   KERNEL=="video*", SUBSYSTEM=="video4linux", \
     ATTRS{idVendor}=="1bcf", ATTRS{idProduct}=="2281", \
     SYMLINK+="orangepi_camara"
     
    
   #KERNEL=="video*":
   #匹配所有名为 videoX 的设备（例如 video0, video1 等）。
   #SUBSYSTEM=="video4linux":
   #确保设备属于 video4linux 子系统。
   
   #ATTRS{idVendor}=="1bcf", ATTRS{idProduct}=="2281":
   #根据 USB 设备的 idVendor 和 idProduct 属性进一步确认设备的唯一性。
   #SYMLINK+="orangepi_camara":
   #创建一个符号链接 /dev/orangepi_camara，方便用户访问该设备。
   
   ~~~

4. 测试 udev 规则

   - 重新加载udev规则

     ~~~shell
     sudo udevadm control --reload-rules
     sudo udevadm trigger
     ~~~

   - 验证符号链接

     检查 `/dev/` 目录下是否生成了 `orangepi_camara` 符号链接：

     ~~~shell
     ls -l /dev/orangepi_camara
     
     #使用udev创建的一个符号连接
     
     ~~~

   - 修改`start.sh`文件

     将里边的`/dev/video1`为`/dev/orangepi_camera`，这时候无论它的设备节点怎么变，只要运行这个脚本就能够启动`mjpg-streamer`服务

开机`mjpg-streamer`服务自启

​	要想实现开机自动启动`mjpg-streamer`服务需要先写一个脚本来运行当前这个目录下的`start.sh`，开启自启的文件存放在`/etc/xdg/autostart`文件夹下。进入到这个目录随后编写`mjpg-streamer`开机启动的脚本。

~~~shell
cd /etc/xdg/autostart/
ls
at-spi-dbus-bus.desktop  pasystray.desktop                            spice-vdagent.desktop                  xdg-user-dirs.desktop
blueman.desktop          polkit-gnome-authentication-agent-1.desktop  ubuntu-advantage-notification.desktop  xfce4-notifyd.desktop
im-launch.desktop        print-applet.desktop                         update-notifier.desktop                xfsettingsd.desktop
nm-applet.desktop        pulseaudio.desktop                           user-dirs-update-gtk.desktop           xscreensaver.desktop
orca-autostart.desktop   snap-userd-autostart.desktop                 xapp-sn-watcher.desktop
orangepi@orangepizero2:/etc/xdg/autostart$ vi print-applet.desktop

sudo cp print-applet.desktop mjpg-streamer.desktop
sudo vi mjpg-streamer.desktop

#添加以下内容
[Desktop Entry]
Name=mjpg-streamer
Comment=Start mjpg-streamer on boot
Exec=/home/orangepi/git/My_Project/mjpg-streamer/mjpg-streamer-experimental/start.sh
Path=/home/orangepi/git/My_Project/mjpg-streamer/mjpg-streamer-experimental
Type=Application
X-GNOME-Autostart-enabled=true

#重启系统
sudo reboot

#查看mjpg-streamer服务是否启动
ps -aux | grep mjpg |grep -v grep

~~~



