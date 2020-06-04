#include "food_supplier.h"

#include <grpcpp/grpcpp.h>

#include <string>
#include <iostream>
#include <map>

#include "absl/time/clock.h"
#include "prometheus/exposer.h"
#include "opencensus/exporters/stats/prometheus/prometheus_exporter.h"




int main(int argc, char **argv) {
	if (argc < 2) {
		std::cerr << "Please provide a port";
		return 1;

	}

	int  port = 0 ;

	absl::SimpleAtoi(argv[1], &port);



	/*Prepare exporters*/
	vendor_latency_view_descriptor.RegisterForExport();
	rpc_count_view_descriptor.RegisterForExport();
	rpc_error_view_descriptor.RegisterForExport();
	
	grpc::RegisterOpenCensusPlugin();

	RegisterExporters();

	auto exporter = std::make_shared<opencensus::exporters::stats::PrometheusExporter>();

	prometheus::Exposer exposer("127.0.0.1:8080");

	exposer.RegisterCollectable(exporter);

	IngredientServiceImpl ingredientService;

	grpc::ServerBuilder builder;

	/*Monitor the given port*/
	builder.AddListeningPort(absl::StrCat("[::]:", port),grpc::InsecureServerCredentials(), &port);

	/*Set the monitoring to look for the ingredientService Message*/
	builder.RegisterService(&ingredientService);

	/*Build the server*/
	std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

	std::cout << "Server listening on [::]:" << port << "\n";

	server->Wait();

}
