plugins {
    id("com.android.application")
}

android {
    namespace = "com.blockeater.blockeater"
    compileSdk = 34

    defaultConfig {
        applicationId = "com.blockeater.blockeater"
        minSdk = 24
        targetSdk = 34
        versionCode = 1
        versionName = "1.0"

        ndk {
            abiFilters += listOf("arm64-v8a")
        }

        // For C++ projects, use c++_shared STL
        // See: https://github.com/raysan5/raylib/wiki/Working-for-Android
        externalNativeBuild {
            cmake {
                arguments += listOf("-DANDROID_STL=c++_shared")
            }
        }
    }

    buildTypes {
        debug {
            // Use consistent debug signing configuration
            // This ensures the same signature across builds for easy updates
            signingConfig = signingConfigs.getByName("debug")
        }
        release {
            isMinifyEnabled = false
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
        }
    }

    // Configure debug signing with consistent parameters
    signingConfigs {
        create("debug") {
            // Use standard Android debug keystore
            // The debug keystore is automatically created by Android Studio/Tools
            // If using GitHub Actions, it will be created at: ~/.android/debug.keystore
            storeFile = file(System.getProperty("user.home") + "/.android/debug.keystore")
            storePassword = "android"
            keyAlias = "androiddebugkey"
            keyPassword = "android"
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }

    ndkVersion = "25.2.9519653"

    // Include font files in APK assets
    sourceSets {
        getByName("main") {
            assets.srcDirs("src/main/cpp/fonts")
        }
    }
}

dependencies {
    implementation("androidx.appcompat:appcompat:1.6.1")
}
