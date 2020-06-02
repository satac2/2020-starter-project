#include <grpcpp/grpcpp.h>
#include <grpcpp/opencensus.h>

#include <string>
#include <iostream>

#include "instrumented/baking.grpc.pb.h"
#include "instrumented/baking.pb.h"
#include "absl/strings/str_cat.h"

namespace {

	using baking::VendorReply;
	using baking::VendorRequest;
	using baking::VendorService;
	using baking::PriceReply;
	using baking::PriceRequest;
	using baking::PriceService;

	/*Class to handle the VendorService message*/
	class VendorServiceImpl final : public VendorService::Service {
		public:

		std::vector<std::string> _ingredient_list;
		
		VendorServiceImpl(std::vector<std::string> ingredient_list){
			_ingredient_list = ingredient_list;
		}


		grpc::Status SendVendorRequest (grpc::ServerContext *context,
				const VendorRequest * request, VendorReply *reply ) override {
			int in_stock = 0;
			std::cout << "vendor response" << std::endl;
			
			for(auto i = _ingredient_list.begin(); i != _ingredient_list.end(); i++){
				std::cout << *i << std::endl;
				if(*i == request->ingredient_name()){
					in_stock = 1;
				}
			}
			
			reply->set_stock(in_stock);



		return grpc::Status::OK;
		}
	};

	/*Class to handle the priceService*/
	class PriceServiceImpl final : public  PriceService::Service {
			
			public:
			std::vector<std::string> _ingredient_list;
			
			PriceServiceImpl(std::vector<std::string> ingredient_list){
				_ingredient_list = ingredient_list;
			}

			grpc::Status SendPriceRequest (grpc::ServerContext *context,
				const PriceRequest * request, PriceReply *reply ) override {
				std::cout << "price response test" << std::endl;
				reply->set_price(1);
				reply->set_quantity(1);
				
				return grpc::Status::OK;
			}
		};


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