#include <grpcpp/grpcpp.h>
#include <grpcpp/opencensus.h>

#include <iostream>
#include <string>

#include "exporters.h"
#include "instrumented/baking.grpc.pb.h"
#include "instrumented/baking.pb.h"
#include "absl/strings/str_cat.h"
#include "absl/time/clock.h"
#include "opencensus/trace/trace_config.h"
#include "opencensus/trace/with_span.h"
#include "opencensus/trace/context_util.h"
#include "opencensus/trace/sampler.h"
#include "opencensus/stats/aggregation.h"
#include "opencensus/stats/bucket_boundaries.h"
#include "opencensus/stats/view_descriptor.h"
#include "opencensus/tags/context_util.h"
#include "opencensus/tags/tag_key.h"
#include "opencensus/tags/tag_map.h"
#include "opencensus/tags/with_tag_map.h"
#include "opencensus/stats/stats.h"



namespace {

	using baking::IngredientService;
	using baking::IngredientRequest;
	using baking::VendorListReply;
	using baking::PriceService;
	using baking::PriceRequest;
	using baking::PriceReply;

	opencensus::tags::TagKey MyKey() {
		static const auto key = opencensus::tags::TagKey::Register("my_key");
		return key;
	}

	opencensus::stats::Aggregation MillisDistributionAggregation() {
		return opencensus::stats::Aggregation::Distribution(
				opencensus::stats::BucketBoundaries::Explicit(
					{0, 0.1, 1, 10, 100, 1000}));
	}

	ABSL_CONST_INIT const absl::string_view kRpcClientRoundtripLatencyMeasureName =
		"grpc.io/client/roundtrip_latency";

	::opencensus::tags::TagKey ClientMethodTagKey() {
		static const auto method_tag_key =
			::opencensus::tags::TagKey::Register("grpc_client_method");
		return method_tag_key;
	}

	::opencensus::tags::TagKey ClientStatusTagKey() {
		static const auto status_tag_key =
			::opencensus::tags::TagKey::Register("grpc_client_status");
		return status_tag_key;
	}

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

	RegisterExporters();

	/*Open a channel to the food_supplier service*/
	std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(argv[1], grpc::InsecureChannelCredentials());	
	std::unique_ptr<IngredientService::Stub> stub = IngredientService::NewStub(channel);
	


	IngredientRequest request;
	VendorListReply reply;


	request.set_ingredient_name(food);


	static opencensus::trace::AlwaysSampler sampler;
	
	/*Set up the parent span to be used on both the vendor and the price messages*/
	auto grocery_span = opencensus::trace::Span::StartSpan(
			"grocery", nullptr, {&sampler});

	auto span = opencensus::trace::Span::StartSpan(
			"VendorTest", &grocery_span, {&sampler});


	/*Set up custom metric*/
	const auto key1 = opencensus::tags::TagKey::Register("key1");

	const std::string supplier_measure_name = "example/suppliers/SuppliersPerQuery";
	
	const opencensus::stats::MeasureDouble supplier_measure =
		opencensus::stats::MeasureDouble::Register(supplier_measure_name,"Suppliers per query", "suppliers");

	const auto view_descriptor =
		opencensus::stats::ViewDescriptor()
		.set_name("example/suppliers/SuppliersPerQuery")
		.set_measure(supplier_measure_name)
		.set_aggregation(opencensus::stats::Aggregation::Distribution(
					opencensus::stats::BucketBoundaries::Explicit({0,1,2,3,4,5,6,7,8,9,10,15})))
		.add_column(key1)
		.set_description(
				"Cumulative distribution of example.com/Foo/FooUsage broken down "
				"by 'key1' and 'key2'.");

	view_descriptor.RegisterForExport();

	{
		
		opencensus::trace::WithSpan ws(span);
		
		grpc::ClientContext ctx;	

		opencensus::trace::GetCurrentSpan().AddAnnotation("Sending request.");

		/*Send request to the food_supplier server*/
		grpc::Status status = stub->SendIngredientRequest(&ctx, request, &reply);

		std::cout << "Got status: " << status.error_code() << ": \""
			<< status.error_message() << "\"\n";
		std::cout << "Got reply: \"" << reply.ShortDebugString() << "\"\n";

		opencensus::trace::GetCurrentSpan().AddAnnotation(
				"Got reply.", {{"status", absl::StrCat(status.error_code(), ": ",
						status.error_message())}});
		if (!status.ok()) {
			opencensus::trace::GetCurrentSpan().SetStatus(
					opencensus::trace::StatusCode::UNKNOWN, status.error_message());
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
			std::shared_ptr<grpc::Channel> channel_p = grpc::CreateChannel(absl::StrCat( "127.0.0.1:" , port_map[reply.vendor_name(i)]), grpc::InsecureChannelCredentials());	
			opencensus::trace::WithSpan ws_p(span_p);

			std::unique_ptr<PriceService::Stub> stub_p = PriceService::NewStub(channel_p);

			PriceRequest request_p;

			PriceReply reply_p;

			request_p.set_ingredient_name(food);

			grpc::ClientContext ctx_p;

			opencensus::trace::GetCurrentSpan().AddAnnotation("Sending request.");
			/*Send the price request message*/
			grpc::Status status_p = stub_p->SendPriceRequest(&ctx_p, request_p, &reply_p);

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
