#ifndef FOOD_FIND_H_
#define FOOD_FIND_H_

#include <grpcpp/opencensus.h>

#include "instrumented/baking.grpc.pb.h"
#include "instrumented/baking.pb.h"
#include "opencensus/stats/aggregation.h"
#include "opencensus/stats/bucket_boundaries.h"
#include "opencensus/stats/view_descriptor.h"
#include "opencensus/stats/stats.h"
#include "absl/strings/str_cat.h"

using baking::VendorReply;
using baking::VendorRequest;
using baking::VendorService;
using baking::PriceReply;
using baking::PriceRequest;
using baking::PriceService;


const auto key1 = opencensus::tags::TagKey::Register("key1");
const std::string rpc_count_measure_name = "food_vendor/rpc_count";
const opencensus::stats::MeasureInt64 rpc_count_measure =
opencensus::stats::MeasureInt64::Register(rpc_count_measure_name,"rpc count", "count");
  const auto rpc_count_view_descriptor =
opencensus::stats::ViewDescriptor()
  .set_name("food_vendor/Rpc_count")
  .set_measure(rpc_count_measure_name)
  .set_aggregation(opencensus::stats::Aggregation::Count())
.add_column(key1)
  .set_description("Cumulative distribution of rpc counts");

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
        opencensus::stats::Record({{ rpc_count_measure, 1}});
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
      opencensus::stats::Record({{ rpc_count_measure, 1}});
      return grpc::Status::OK;
    }
};


#endif
