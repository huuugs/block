// Top-level build file
plugins {
    id("com.android.application") version "8.2.2" apply false
}

allprojects {
    repositories {
        maven { url = uri("https://maven.aliyun.com/repository/public") }
        mavenCentral()
        google()
    }
}

tasks.register("clean", Delete::class) {
    delete(rootProject.buildDir)
}
