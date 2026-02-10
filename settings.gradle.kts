pluginManagement {
    repositories {
        maven { url = uri("https://maven.aliyun.com/repository/gradle-plugin") }
        maven { url = uri("https://maven.aliyun.com/repository/public") }
        mavenCentral()
        google()
        gradlePluginPortal()
    }
}

rootProject.name = "BlockEater"
include(":app")
