#ifndef FOOD_FIND_H_
#define FOOD_FIND_H_

#include <grpcpp/opencensus.h>

#include "opencensus/stats/aggregation.h"
#include "opencensus/stats/bucket_boundaries.h"
#include "opencensus/stats/view_descriptor.h"
#include "opencensus/stats/stats.h"
#include "absl/strings/str_cat.h"

opencensus::stats::Aggregation MillisDistributionAggregation() {
  return opencensus::stats::Aggregation::Distribution(
      opencensus::stats::BucketBoundaries::Explicit({0, 0.1, 1, 10, 100, 1000}));
}

ABSL_CONST_INIT const absl::string_view food_finder_client_roundtrip_latency 
    = "food_finder/client/roundtrip_latency";

auto key1 = opencensus::tags::TagKey::Register("key1");

const std::string supplier_measure_name = "food_finder/SuppliersPerQuery";

const opencensus::stats::MeasureDouble supplier_measure = 
    opencensus::stats::MeasureDouble::Register(supplier_measure_name,
                                               "Suppliers per query",
                                               "suppliers");

const auto supplier_view_descriptor = opencensus::stats::ViewDescriptor()
  .set_name("food_finder/SuppliersPerQuery")
  .set_measure(supplier_measure_name)
  .set_aggregation(opencensus::stats::Aggregation::Distribution(
        opencensus::stats::BucketBoundaries::Explicit({0,1,2,3,4,5,6,7,8,9,10,15})))
  .add_column(key1)
  .set_description("Cumulative distribution of suppliers per Query");


const std::string ingredient_latency_measure_name = "food_finder/Roundtrip_ingredient_latency";

const opencensus::stats::MeasureDouble ingredient_latency_measure = 
     opencensus::stats::MeasureDouble::Register(ingredient_latency_measure_name , 
                                                "Latency to get vendor list", 
                                                "ms");

const auto ingredient_latency_view_descriptor = opencensus::stats::ViewDescriptor()
  .set_name("food_finder/Roundtrip_ingredient_latency")
  .set_measure(ingredient_latency_measure_name)
  .set_aggregation(MillisDistributionAggregation())
  .add_column(key1)
  .set_description("Cumulative distribution of ingredient request latency");


const std::string price_latency_measure_name = "food_finder/Roundtrip_price_latency";

const opencensus::stats::MeasureDouble price_latency_measure = 
  opencensus::stats::MeasureDouble::Register(price_latency_measure_name, 
                                             "Latency to get ingredient price",
                                              "ms");

const auto price_latency_view_descriptor = opencensus::stats::ViewDescriptor()
  .set_name("food_finder/Roundtrip_price_latency")
  .set_measure(price_latency_measure_name)
  .set_aggregation(MillisDistributionAggregation())
  .add_column(key1)
  .set_description("Cumulative distribution of price request latency");


const std::string rpc_count_measure_name = "food_finder/rpc_count";

const opencensus::stats::MeasureInt64 rpc_count_measure = 
    opencensus::stats::MeasureInt64::Register(rpc_count_measure_name,
                                              "rpc count",
                                              "count");

const auto rpc_count_view_descriptor = opencensus::stats::ViewDescriptor()
  .set_name("food_finder/Rpc_count")
  .set_measure(rpc_count_measure_name)
  .set_aggregation(opencensus::stats::Aggregation::Count())
  .add_column(key1)
  .set_description("Cumulative distribution of rpc counts");


const std::string rpc_error_measure_name = "food_finder/rpc_errors";

const opencensus::stats::MeasureInt64 rpc_error_measure =
    opencensus::stats::MeasureInt64::Register(rpc_error_measure_name,"rpc errors", "errors");

const auto rpc_error_view_descriptor = opencensus::stats::ViewDescriptor()
  .set_name("food_finder/Rpc_errors")
  .set_measure(rpc_error_measure_name)
  .set_aggregation(opencensus::stats::Aggregation::Count())
  .add_column(key1)
  .set_description("Cumulative distribution of rpc errors");

#endif
