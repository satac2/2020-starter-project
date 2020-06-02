#!/bin/bash
./process_kill.sh

bazel build :all

env STACKDRIVER_PROJECT_ID=samatac-2020-starter-project ../bazel-bin/instrumented/food_supplier 9001 &

env STACKDRIVER_PROJECT_ID=samatac-2020-starter-project ../bazel-bin/instrumented/food_vendor 9002 foodMart apple &
env STACKDRIVER_PROJECT_ID=samatac-2020-starter-project ../bazel-bin/instrumented/food_vendor 9003 grainShoppe apple &
env STACKDRIVER_PROJECT_ID=samatac-2020-starter-project ../bazel-bin/instrumented/food_vendor 9004 twinkieStore apple &
