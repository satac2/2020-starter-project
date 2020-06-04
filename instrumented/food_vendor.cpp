#include "food_vendor.h"

#include <grpcpp/grpcpp.h>

#include <string>
#include <iostream>

#include "instrumented/exporters.h"
#include "absl/strings/str_cat.h"
#include "absl/time/clock.h"
#include "opencensus/trace/trace_config.h"
#include "opencensus/trace/with_span.h"
#include "opencensus/trace/context_util.h"
#include "opencensus/trace/sampler.h"
#include "prometheus/exposer.h"
#include "opencensus/exporters/stats/prometheus/prometheus_exporter.h"


namespace {

	using baking::VendorReply;
	using baking::VendorRequest;
	using baking::VendorService;
	using baking::PriceReply;
	using baking::PriceRequest;
	using baking::PriceService;


}


int main(int argc, char **argv) {
	if (argc < 4) {
		std::cerr << "Please provide a port and name and atleast one food";
		return 1;

	}

	int  port = 0 ;
	absl::SimpleAtoi(argv[1], &port);

	std::string vendorName = argv[2];

	std::vector<std::string> ingredient_list;

	/*Assemble the current ingredients*/
	for(int i = 3; i < argc; i++){
		ingredient_list.push_back(argv[i]);
	}


	rpc_count_view_descriptor.RegisterForExport();

	grpc::RegisterOpenCensusPlugin();

	RegisterExporters();

	VendorServiceImpl vendorService(ingredient_list);

	PriceServiceImpl priceService(ingredient_list);

	grpc::ServerBuilder builder;

	/*Set the port to listen on*/	
	builder.AddListeningPort(absl::StrCat("[::]:", port),grpc::InsecureServerCredentials(), &port);

	/*Register both the vendor and price service*/
	builder.RegisterService(&vendorService);

	builder.RegisterService(&priceService);

	std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

	std::cout << "Server listening on [::]:" << port << "\n";

	server->Wait();

}
