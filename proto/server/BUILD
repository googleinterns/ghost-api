load("@rules_proto//proto:defs.bzl", "proto_library")
load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_proto_library")
load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")

cc_library(
  name = "server-lib",
  srcs = ["server.cc"],
  hdrs = ["server.h"],
  deps = [
      "@com_github_grpc_grpc//:grpc++",
  ],

)

cc_binary(
  name = "run-server",
  srcs = ["main.cc"],
  defines = ["BAZEL_BUILD"],
  deps = [":server-lib"],
)
