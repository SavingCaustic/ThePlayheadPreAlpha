Maybe this is a good place. Any directory could be symblinked..

And howto here:
Download Android Command line tools only
https://developer.android.com/studio#command-tools
Unpack and add directory to path

* sudo apt install openjdk-17-jdk

* add to end of .bashrc
export ANDROID_HOME=/opt/Android
export PATH=$ANDROID_HOME/cmdline-tools/bin:$PATH
export PATH=$ANDROID_HOME/platform-tools:$PATH
export PATH=$ANDROID_HOME/build-tools/30.0.3:$PATH
