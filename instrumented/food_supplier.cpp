#include <grpcpp/grpcpp.h>
#include <grpcpp/opencensus.h>

#include <string>
#include <iostream>
#include <map>


#include "instrumented/exporters.h"
#include "instrumented/baking.grpc.pb.h"
#include "instrumented/baking.pb.h"
#include "absl/strings/str_cat.h"
#include "absl/time/clock.h"
#include "opencensus/trace/trace_config.h"
#include "opencensus/trace/with_span.h"
#include "opencensus/trace/context_util.h"
#include "opencensus/trace/sampler.h"
#include "prometheus/exposer.h"
#include "opencensus/exporters/stats/prometheus/prometheus_exporter.h"

namespace {

	using baking::IngredientService;
	using baking::IngredientRequest;
	using baking::VendorListReply;
	using baking::VendorService;
	using baking::VendorRequest;
	using baking::VendorReply;

	

	/*Sends a request to a vendor and returns the quantity of the requested ingredient in stock*/

	int check_vendor(std::string port, std::string requested_ingredient, grpc::ServerContext *context, const opencensus::trace::Span* parentSpan){

		int in_stock = 0;

		std::cout << "Checking: "<< requested_ingredient <<"  port:" <<port << std::endl;
		
		/*Open the channel to the vendor*/	
		std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel( absl::StrCat("127.0.0.1:",port ), grpc::InsecureChannelCredentials());      

		std::unique_ptr<VendorService::Stub> stub = VendorService::NewStub(channel);

		VendorRequest request;

		VendorReply reply;

		request.set_ingredient_name(requested_ingredient);
		
		/*Convert the context from the server to maintain context through multiple client/server pairs*/
		auto ctx_test = ((grpc_impl::ClientContext::FromServerContext(*context)));
		
		opencensus::trace::Span span = opencensus::trace::Span::StartSpan("sending_vendor_request", parentSpan);

		span.AddAttribute("my_attribute", "orange");		

		span.AddAnnotation("Sending vendor request");
		
		/*Send the request*/
		grpc::Status status = stub->SendVendorRequest(ctx_test.release(), request, &reply);

		span.End();	

		std::cout << "Got status: " << status.error_code() << ": \""
			<< status.error_message() << "\"\n";
		std::cout << "Got reply: \"" << reply.ShortDebugString() << "\"\n";

		std::cout << "Ingredient: "<< requested_ingredient  << " " << reply.stock() << std::endl;
		


		return reply.stock(); 
	}


	// A helper function that performs some work in its own Span.
	void PerformWork(opencensus::trace::Span *parent) {
		auto span = opencensus::trace::Span::StartSpan("internal_work", parent);
		span.AddAttribute("my_attribute", "blue");
		span.AddAnnotation("Performing work.");
		absl::SleepFor(absl::Milliseconds(20));  // Working hard here.
		span.End();
	}


	/*The response to an incoming IngredientRequest message*/
	class IngredientServiceImpl final : public IngredientService::Service {
		grpc::Status SendIngredientRequest (grpc::ServerContext *context,
				const IngredientRequest * request, VendorListReply *reply ) override {

			opencensus::trace::Span span = grpc::GetSpanFromServerContext(context);
			span.AddAttribute("my_attribute", "red");		

			PerformWork(&span);
			span.AddAnnotation("Sleeping.");

			std::string vendor_list[3] = {"foodMart","grainStore","twinkieStore"};
			std::map<std::string,std::string> port_map = {{vendor_list[0],"9002"},{vendor_list[1],"9003"},{vendor_list[2],"9004"}};
			
			/*For each of the vendors that have the ingredient in stock, add them to the reply to be sent to the food_finder*/
			for(int i = 0; i < 3; i++){

				if(check_vendor(port_map[vendor_list[i]], request->ingredient_name(), context, &span) > 0){
					std::cout << "adding vendor:" << vendor_list[i]<<std::endl;	
					reply->add_vendor_name(vendor_list[i]);
				}

			}


			return grpc::Status::OK;
		}
	};


}


int main(int argc, char **argv) {
	if (argc < 2) {
		std::cerr << "Please provide a port";
		return 1;

	}

	int  port = 0 ;

	absl::SimpleAtoi(argv[1], &port);

	/*Prepare exporters*/
	grpc::RegisterOpenCensusPlugin();

	grpc::RegisterOpenCensusViewsForExport();

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
