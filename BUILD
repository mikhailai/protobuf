licenses(["notice"])

exports_files(["LICENSE"])

################################################################################
# Protobuf Plugin for compiling javanano
################################################################################

COPTS = [
    "-DHAVE_PTHREAD",
    "-Wall",
    "-Wwrite-strings",
    "-Woverloaded-virtual",
    "-Wno-sign-compare",
    "-Wno-unused-function",
]

LINK_OPTS = select({
    "//conditions:default": ["-lpthread"],
})

config_setting(
    name = "ios_armv7",
    values = {
        "ios_cpu": "armv7",
    },
)

config_setting(
    name = "ios_armv7s",
    values = {
        "ios_cpu": "armv7s",
    },
)

config_setting(
    name = "ios_arm64",
    values = {
        "ios_cpu": "arm64",
    },
)

IOS_ARM_COPTS = COPTS + [
    "-DOS_IOS",
    "-miphoneos-version-min=7.0",
    "-arch armv7",
    "-arch armv7s",
    "-arch arm64",
    "-D__thread=",
    "-isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS9.2.sdk/",
]

cc_library(
    name = "protoc_javanano_lib",
    srcs = [
        "src/google/protobuf/compiler/javanano/javanano_enum.cc",
        "src/google/protobuf/compiler/javanano/javanano_helpers.cc",
        "src/google/protobuf/compiler/javanano/javanano_enum_field.cc",
        "src/google/protobuf/compiler/javanano/javanano_map_field.cc",
        "src/google/protobuf/compiler/javanano/javanano_extension.cc",
        "src/google/protobuf/compiler/javanano/javanano_message.cc",
        "src/google/protobuf/compiler/javanano/javanano_field.cc",
        "src/google/protobuf/compiler/javanano/javanano_message_field.cc",
        "src/google/protobuf/compiler/javanano/javanano_file.cc",
        "src/google/protobuf/compiler/javanano/javanano_primitive_field.cc",
        "src/google/protobuf/compiler/javanano/javanano_generator.cc",
    ],
    hdrs = glob(["src/google/protobuf/compiler/javanano/*.h"]),
    copts = select({
        ":ios_armv7": IOS_ARM_COPTS,
        ":ios_armv7s": IOS_ARM_COPTS,
        ":ios_arm64": IOS_ARM_COPTS,
        "//conditions:default": COPTS,
    }),
    includes = ["src/", "third_party/"],
    linkopts = LINK_OPTS,
    visibility = ["//visibility:public"],
    deps = ["@com_google_protobuf//:protoc_lib"],
)

cc_binary(
    name = "protoc-gen-javanano",
    srcs = ["src/google/protobuf/compiler/javanano/main.cc"],
    linkopts = LINK_OPTS,
    visibility = ["//visibility:public"],
    deps = [":protoc_javanano_lib"],
)
