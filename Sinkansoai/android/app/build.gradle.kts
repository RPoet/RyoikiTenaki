plugins {
    id("com.android.application")
}

android {
    namespace = "com.ryoikitenaki.sinkansoai"
    compileSdk = 34

    defaultConfig {
        applicationId = "com.ryoikitenaki.sinkansoai"
        minSdk = 26  // Vulkan 1.1 support
        targetSdk = 34
        versionCode = 1
        versionName = "1.0.0"

        ndk {
            // Target ARM64 devices (most modern Android phones)
            abiFilters += listOf("arm64-v8a")
        }

        externalNativeBuild {
            cmake {
                cppFlags += "-std=c++17"
                arguments += listOf(
                    "-DANDROID_STL=c++_shared",
                    "-DANDROID_TOOLCHAIN=clang"
                )
            }
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
        debug {
            isDebuggable = true
            isJniDebuggable = true
        }
    }

    externalNativeBuild {
        cmake {
            path = file("../../CMakeLists.txt")
            version = "3.22.1"
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }

    buildFeatures {
        prefab = true
    }
}

dependencies {
    // No Java/Kotlin dependencies needed for pure native app
}
