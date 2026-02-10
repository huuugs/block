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
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
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
}

dependencies {
    implementation("androidx.appcompat:appcompat:1.6.1")
}
