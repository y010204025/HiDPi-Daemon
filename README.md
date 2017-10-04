HiDPi-Daemon
---

What is this
---

Build to solve "topleft issues" on Hanckintosh after turning on the HiDPi and after first wake.

Like this:

![](https://github.com/edward-p/HiDPi-Daemon/raw/picture/IMG_20171004_102631.jpg)

How does it work
---

When the daemon start, it creats a wake/sleep events handler.
refer [Registering and unregistering for sleep and wake notifications](https://developer.apple.com/library/content/qa/qa1340/_index.html)

When wake detected, the handler will reset the resolution(by callinng `/Applications/RDM.app/Contents/MacOS/SetResX`) for you automatically then the daemon will unload it self.(Just because you don't need it for second wake)

__To make sure it works:__ Install `RDM.app` as `/Application/RDM.app`

Run at start by creating/Library/LaunchDaemons/com.edward-p.hidpi.daemon.plist" using `-o` option.
You can change by changing the following:

``` C
#define LAUNCHD_PATH "/Library/LaunchDaemons/com.edward-p.hidpi.daemon.plist"
#define STOP_DAEMON_COMMAND "launchctl unload /Library/LaunchDaemons/com.edward-p.hidpi.daemon.plist"
```


Usage
---

	 Usage:	hidpi-daemon <[Options] [Values]> <[Option]>

	 Options:

	    -d	Default Resolution: 1920x1080 etc.
	    -r	HiDPi(Retina) Resolution: 1600x900 etc.
	    -o	Export launchd.plist to file.
	    -h	Print this help.

	 examples:

	    hidpi-daemon -d 1920x1080 -r 1600x900
	 Note:
	    Assume that you have installed it as /usr/local/bin/hidpi-deamon
	    Assume that you have installed RDM.app in /Application
	    sudo hidpi-daemon -d 1920x1080 -r 1600x900 -o
	        Then launchd.plist will appeared into /Library/LaunchDaemons
	        as "com.edward-p.hidpi.daemon.plist"
