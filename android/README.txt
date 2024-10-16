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

mkdir ~/Android/cmdline-tools/latest
mv ~/Android/cmdline-tools/cmdline-tools/* ~/Android/cmdline-tools/latest

sdkmanager "platforms;android-33"


gradle:

wget https://services.gradle.org/distributions/gradle-8.3-bin.zip
unzip gradle-8.3-bin.zip
sudo mv gradle-8.3 /opt/gradle


android {
    compileSdkVersion 33
    defaultConfig {
        applicationId "com.example.myapp"
        minSdkVersion 26 // or 27
        targetSdkVersion 33
        versionCode 1
        versionName "1.0"
    }
    ...
}

