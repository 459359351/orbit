// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/flags/flag.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <libfuzzer/libfuzzer_macro.h>

#include <cstdio>
#include <filesystem>

#include "App.h"
#include "Capture.h"
#include "CaptureSerializer.h"
#include "SamplingProfiler.h"
#include "TimeGraph.h"
#include "capture_data.pb.h"

// Hack: This is declared in a header we include here
// and the definition needs to take place somewhere.
ABSL_FLAG(bool, enable_stale_features, false,
          "Enable obsolete features that are not working or are not "
          "implemented in the client's UI");
ABSL_FLAG(bool, devmode, false, "Enable developer mode in the client's UI");
ABSL_FLAG(uint16_t, sampling_rate, 1000, "Frequency of callstack sampling in samples per second");
ABSL_FLAG(bool, frame_pointer_unwinding, false, "Use frame pointers for unwinding");

DEFINE_PROTO_FUZZER(const orbit_client_protos::CaptureSerializerFuzzerInfo& info) {
  std::string buffer{};
  {
    google::protobuf::io::StringOutputStream stream{&buffer};
    google::protobuf::io::CodedOutputStream coded_stream{&stream};
    orbit_client_protos::CaptureHeader capture_header{};
    capture_header.set_version("1.52");

    CaptureSerializer::WriteMessage(&capture_header, &coded_stream);
    CaptureSerializer::WriteMessage(&info.capture_info(), &coded_stream);
    for (const auto& timer : info.timers()) {
      CaptureSerializer::WriteMessage(&timer, &coded_stream);
    }
  }

  OrbitApp::Init({}, nullptr);
  TimeGraph time_graph{};
  GCurrentTimeGraph = &time_graph;
  auto string_manager = std::make_shared<StringManager>();
  time_graph.SetStringManager(string_manager);
  GOrbitApp->ClearCapture();

  CaptureSerializer serializer{};
  serializer.time_graph_ = GCurrentTimeGraph;

  // NOLINTNEXTLINE
  std::istringstream input_stream{std::move(buffer)};
  (void)serializer.Load(input_stream);

  GOrbitApp->GetThreadPool()->ShutdownAndWait();
  GOrbitApp.reset();
}
