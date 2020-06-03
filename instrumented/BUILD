# Copyright 2018, OpenCensus Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#load("@io_opencensus_cpp//opencensus:copts.bzl", "DEFAULT_COPTS", "TEST_COPTS")

#load("@io_opencensus_cpp//bazel:cc_grpc_library.bzl", "cc_grpc_library")
load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")



licenses(["notice"])  # Apache License 2.0

package(default_visibility = ["//visibility:private"])




proto_library(
		name = "baking_proto",
		srcs = ["baking.proto"],
		)

cc_proto_library(
		name = "baking_cc_proto",
		deps = [":baking_proto"],
		)

cc_grpc_library(
		name = "baking_cc_grpc",
		srcs = [":baking_proto"],
		grpc_only = True,
		deps = [":baking_cc_proto"],
		)

cc_library(
		name = "exporters",
		srcs = ["exporters.cc"],
		hdrs = ["exporters.h"],
		deps = [
				"@io_opencensus_cpp//opencensus/exporters/stats/stackdriver:stackdriver_exporter",
				"@io_opencensus_cpp//opencensus/exporters/stats/stdout:stdout_exporter",
				"@io_opencensus_cpp//opencensus/exporters/trace/ocagent:ocagent_exporter",
				"@io_opencensus_cpp//opencensus/exporters/trace/stackdriver:stackdriver_exporter",
				"@io_opencensus_cpp//opencensus/exporters/trace/stdout:stdout_exporter",
				"@com_google_absl//absl/strings",
				],
		)


cc_binary(
		name = "food_finder",
		srcs = ["food_finder.cpp"],
		deps = [
				":exporters",
				":baking_cc_grpc",
				":baking_cc_proto",
				"@com_github_grpc_grpc//:grpc++",
				"@com_github_grpc_grpc//:grpc_opencensus_plugin",

				"@io_opencensus_cpp//opencensus/tags",
				"@io_opencensus_cpp//opencensus/tags:context_util",
				"@io_opencensus_cpp//opencensus/tags:with_tag_map",
				"@io_opencensus_cpp//opencensus/trace",
				"@io_opencensus_cpp//opencensus/trace:context_util",
				"@io_opencensus_cpp//opencensus/trace:with_span",
				"@com_google_absl//absl/strings",
				"@com_google_absl//absl/time",
				],
		)

cc_binary(
		name = "food_supplier",
		srcs = ["food_supplier.cpp"],
		deps = [
				":exporters",
				":baking_cc_grpc",
				":baking_cc_proto",
				"@com_github_grpc_grpc//:grpc++",
				"@com_github_grpc_grpc//:grpc_opencensus_plugin",
				"@io_opencensus_cpp//opencensus/exporters/stats/prometheus:prometheus_exporter",	

				"@io_opencensus_cpp//opencensus/tags",
				"@io_opencensus_cpp//opencensus/tags:context_util",
				"@io_opencensus_cpp//opencensus/tags:with_tag_map",


				"@com_github_jupp0r_prometheus_cpp//pull",
				"@io_opencensus_cpp//opencensus/trace",
				"@io_opencensus_cpp//opencensus/trace:context_util",
				"@io_opencensus_cpp//opencensus/trace:with_span",
				"@com_google_absl//absl/strings",
				"@com_google_absl//absl/time",
				],
		)

cc_binary(
		name = "food_vendor",
		srcs = ["food_vendor.cpp"],
		deps = [
				":exporters",
				":baking_cc_grpc",
				":baking_cc_proto",
				
				"@com_github_grpc_grpc//:grpc++",
				"@com_github_grpc_grpc//:grpc_opencensus_plugin",
				"@io_opencensus_cpp//opencensus/exporters/stats/prometheus:prometheus_exporter",	

				"@io_opencensus_cpp//opencensus/tags",
				"@io_opencensus_cpp//opencensus/tags:context_util",
				"@io_opencensus_cpp//opencensus/tags:with_tag_map",


				"@com_github_jupp0r_prometheus_cpp//pull",
				"@io_opencensus_cpp//opencensus/trace",
				"@io_opencensus_cpp//opencensus/trace:context_util",
				"@io_opencensus_cpp//opencensus/trace:with_span",
				"@com_google_absl//absl/strings",
				"@com_google_absl//absl/time",
				
				],
		)

