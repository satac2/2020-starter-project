#ifndef FOOD_SUPP_H_
#define FOOD_SUPP_H_

#include <grpcpp/opencensus.h>

#include "absl/strings/str_cat.h"
#include "opencensus/stats/aggregation.h"
#include "opencensus/stats/bucket_boundaries.h"
#include "opencensus/stats/view_descriptor.h"
#include "opencensus/stats/stats.h"

opencensus::stats::Aggregation MillisDistributionAggregation() {
  return opencensus::stats::Aggregation::Distribution(
      opencensus::stats::BucketBoundaries::Explicit({0, 0.1, 1, 10, 100, 1000}));
}


const auto key1 = opencensus::tags::TagKey::Register("key1");

const std::string vendor_latency_measure_name = 
    "food_supplier/Roundtrip_vendor_latency";

const opencensus::stats::MeasureDouble vendor_latency_measure =
    opencensus::stats::MeasureDouble::Register(vendor_latency_measure_name,
                                           "Latency to get stock information", 
                                           "ms");

const auto vendor_latency_view_descriptor =
opencensus::stats::ViewDescriptor()
  .set_name("food_supplier/Roundtrip_vendor_latency")
  .set_measure(vendor_latency_measure_name)
  .set_aggregation(MillisDistributionAggregation())
  .add_column(key1)
  .set_description("Cumulative distribution of vendor request latency");




const std::string rpc_count_measure_name = "food_supplier/rpc_count";

const opencensus::stats::MeasureInt64 rpc_count_measure =
    opencensus::stats::MeasureInt64::Register(rpc_count_measure_name,"rpc count", "count");

const auto rpc_count_view_descriptor = opencensus::stats::ViewDescriptor()
  .set_name("food_supplier/Rpc_count")
  .set_measure(rpc_count_measure_name)
  .set_aggregation(opencensus::stats::Aggregation::Count())
  .add_column(key1)
  .set_description("Cumulative distribution of rpc counts");




const std::string rpc_error_measure_name = "food_supplier/rpc_errors";

const opencensus::stats::MeasureInt64 rpc_error_measure = 
    opencensus::stats::MeasureInt64::Register(rpc_error_measure_name,
                                              "rpc errors", 
                                              "errors");

const auto rpc_error_view_descriptor = opencensus::stats::ViewDescriptor()
  .set_name("food_supplier/Rpc_errors")
  .set_measure(rpc_error_measure_name)
  .set_aggregation(opencensus::stats::Aggregation::Count())
  .add_column(key1)
  .set_description("Cumulative distribution of rpc errors");


#endif
