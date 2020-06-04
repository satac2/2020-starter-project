#include "food_finder.h"

#include <grpcpp/grpcpp.h>

#include <iostream>
#include <string>

#include "exporters.h"
#include "instrumented/baking.grpc.pb.h"
#include "instrumented/baking.pb.h"
#include "absl/time/clock.h"
#include "opencensus/trace/trace_config.h"
#include "opencensus/trace/with_span.h"
#include "opencensus/trace/context_util.h"
#include "opencensus/trace/sampler.h"
#include "opencensus/tags/context_util.h"
#include "opencensus/tags/tag_key.h"
#include "opencensus/tags/tag_map.h"
#include "opencensus/tags/with_tag_map.h"



namespace {

  using baking::IngredientService;
  using baking::IngredientRequest;
  using baking::VendorListReply;
  using baking::PriceService;
  using baking::PriceRequest;
  using baking::PriceReply;

}



int main(int argc, char **argv) {
  if (argc < 3) {
    std::cerr << "Please provide a port and a food";
    return 1;

  }

  int port = 0;

  std::string food = argv[2];

  absl::SimpleAtoi(argv[1], &port);


  /*Setup opencensus and exporters*/
  grpc::RegisterOpenCensusPlugin();

  //grpc::RegisterOpenCensusViewsForExport();
  RegisterExporters();

  /*Open a channel to the food_supplier service*/
  std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(argv[1], grpc::InsecureChannelCredentials());	
  std::unique_ptr<IngredientService::Stub> stub = IngredientService::NewStub(channel);


  static opencensus::trace::AlwaysSampler sampler;

  /*Set up the parent span to be used on both the vendor and the price messages*/
  auto grocery_span = opencensus::trace::Span::StartSpan(
      "grocery", nullptr, {&sampler});

  auto span = opencensus::trace::Span::StartSpan(
      "VendorTest", &grocery_span, {&sampler});


  /*Set up custom metrics*/
  supplier_view_descriptor.RegisterForExport();

  ingredient_latency_view_descriptor.RegisterForExport();

  price_latency_view_descriptor.RegisterForExport();

  rpc_count_view_descriptor.RegisterForExport();

  rpc_error_view_descriptor.RegisterForExport();


  IngredientRequest request;
  VendorListReply reply;

  {

    opencensus::trace::WithSpan ws(span);

    grpc::ClientContext ctx;	

    opencensus::trace::GetCurrentSpan().AddAnnotation("Sending request.");

    absl::Time start = absl::Now();


    request.set_ingredient_name(food);
    /*Send request to the food_supplier server*/
    grpc::Status status = stub->SendIngredientRequest(&ctx, request, &reply);

    absl::Time end = absl::Now();
    double latency_ms = absl::ToDoubleMilliseconds(end - start);

    opencensus::stats::Record({{ ingredient_latency_measure, latency_ms}});

    opencensus::stats::Record({ {rpc_count_measure, 1} });


    std::cout << "Got status: " << status.error_code() << ": \""
        << status.error_message() << "\"\n";
    std::cout << "Got reply: \"" << reply.ShortDebugString() << "\"\n";

    opencensus::trace::GetCurrentSpan().AddAnnotation(
        "Got reply.", {{"status", absl::StrCat(status.error_code(), ": ",
            status.error_message())}});
    
    if (!status.ok()) {
      opencensus::trace::GetCurrentSpan().SetStatus(
          opencensus::trace::StatusCode::UNKNOWN, 
          status.error_message());

      opencensus::stats::Record({{rpc_error_measure, 1}});
    }

  }
  span.End();



  auto span_price_parent = opencensus::trace::Span::StartSpan(
      "PriceParent", &grocery_span, {&sampler});

  /*Statically stored for now and possibly ever*/
  std::string vendor_list[3] = {"foodMart","grainStore","twinkieStore"};
  std::map<std::string,std::string> port_map = {{vendor_list[0],"9002"},{vendor_list[1],"9003"},{vendor_list[2],"9004"}};

  /*Record custom metric data*/
  opencensus::stats::Record({{supplier_measure, (float)reply.vendor_name_size()}}, {{key1, "v1"}});

  /*For each vendor that has the ingredient, get the price*/	
  for(int i = 0; i < reply.vendor_name_size(); i++){

    auto span_p = opencensus::trace::Span::StartSpan(
        "PriceTest", &span_price_parent , {&sampler});
    {

      /*Open a channel to a vendor*/
      std::shared_ptr<grpc::Channel> channel_p = grpc::CreateChannel(absl::StrCat( "127.0.0.1:" , 
            port_map[reply.vendor_name(i)]), grpc::InsecureChannelCredentials());	
      opencensus::trace::WithSpan ws_p(span_p);

      std::unique_ptr<PriceService::Stub> stub_p = PriceService::NewStub(channel_p);

      PriceRequest request_p;

      PriceReply reply_p;

      request_p.set_ingredient_name(food);

      grpc::ClientContext ctx_p;

      opencensus::trace::GetCurrentSpan().AddAnnotation("Sending request.");

      absl::Time start_p = absl::Now();

      /*Send the price request message*/
      grpc::Status status_p = stub_p->SendPriceRequest(&ctx_p, request_p, &reply_p);

      absl::Time end_p = absl::Now();
      double latency_ms = absl::ToDoubleMilliseconds(end_p - start_p);

      opencensus::stats::Record({{ price_latency_measure, latency_ms}});

      opencensus::stats::Record({ {rpc_count_measure, 1} });



      if (!status_p.ok()) {
        opencensus::trace::GetCurrentSpan().SetStatus(
            opencensus::trace::StatusCode::UNKNOWN, status_p.error_message());

        opencensus::stats::Record({ {rpc_error_measure, 1} });
      }
      span_p.End(); //Does this have an effect because the scope is ending anyway?
    }

  }

  span_price_parent.End();



  opencensus::trace::TraceConfig::SetCurrentTraceParams(
      {128, 128, 128, 128, opencensus::trace::ProbabilitySampler(0.0)});

  //If we end too quick the data will not export
  std::cout << "Client sleeping, ^C to exit.\n" << std::endl;
  while (true){
    absl::SleepFor(absl::Seconds(10));
  }

}
